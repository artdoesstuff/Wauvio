#pragma once

#include "midi_music.hpp"
#include "../core/midi_parser.hpp"
#include "../core/tempo_map.hpp"

#include <array>
#include <map>
#include <string>
#include <utility>

namespace wauvio {
namespace track {

struct LoadOptions {
    midi::InstrumentResolver resolver;
    bool honor_sustain_pedal = true;
    bool honor_portamento = true;
    double pitch_bend_range_semitones = 2.0;
    double default_glide_time = 0.08;
};

namespace detail {

struct ChannelState {
    int program = 0;
    bool sustain_on = false;
    bool portamento_on = false;
    double pitch_bend_semitones = 0.0;
    double expression = 1.0;
    double volume = 1.0;
    int last_note_on_pitch = -1;
};

struct HeldNote {
    uint64_t start_tick = 0;
    uint8_t velocity = 64;
    double expression_at_onset = 1.0;
    double bend_at_onset = 0.0;
    int glide_from_midi = -1;
    int program_at_onset = 0;
    bool pending_pedal_release = false;
};

inline audio::Dynamics velocity_to_dynamics_u8(uint8_t v) {
    return audio::velocity_to_dynamics(static_cast<double>(v) / 127.0);
}

}

inline MidiMusic load_midi(const std::string& path, const LoadOptions& opts = LoadOptions()) {
    midi::ParsedMidiFile parsed = midi::parse_midi_file(path);

    midi::TempoMap tempo_map;
    std::vector<midi::TimeSignatureEvent> time_sigs;
    std::vector<midi::KeySignatureEvent> key_sigs;
    uint64_t max_tick = 0;

    for (auto& tr : parsed.tracks) {
        for (auto& ev : tr.events) {
            if (ev.abs_tick > max_tick) max_tick = ev.abs_tick;
            if (ev.type != midi::MidiEventType::Meta) continue;

            if (ev.meta_type == 0x51 && ev.meta_data.size() == 3) {
                uint32_t mpq = (static_cast<uint32_t>(ev.meta_data[0]) << 16) |
                               (static_cast<uint32_t>(ev.meta_data[1]) << 8) |
                               static_cast<uint32_t>(ev.meta_data[2]);
                if (mpq > 0) tempo_map.add_tempo_event(ev.abs_tick, mpq);
            } else if (ev.meta_type == 0x58 && ev.meta_data.size() >= 2) {
                midi::TimeSignatureEvent tse;
                tse.tick = ev.abs_tick;
                tse.numerator = ev.meta_data[0];
                tse.denominator = 1 << ev.meta_data[1];
                time_sigs.push_back(tse);
            } else if (ev.meta_type == 0x59 && ev.meta_data.size() >= 2) {
                midi::KeySignatureEvent kse;
                kse.tick = ev.abs_tick;
                kse.sharps_or_flats = static_cast<int8_t>(ev.meta_data[0]);
                kse.is_minor = ev.meta_data[1] != 0;
                key_sigs.push_back(kse);
            }
        }
    }
    tempo_map.finalize(parsed.division);

    MidiMusic music;
    music.source_path = path;
    music.initial_bpm = tempo_map.bpm_at_start();
    music.duration_seconds = tempo_map.tick_to_seconds(max_tick);
    music.ticks_per_quarter = parsed.division;
    music.time_signatures = std::move(time_sigs);
    music.key_signatures = std::move(key_sigs);

    for (size_t ti = 0; ti < parsed.tracks.size(); ++ti) {
        const midi::ParsedTrack& raw_track = parsed.tracks[ti];
        std::array<detail::ChannelState, 16> chan_state{};
        std::array<bool, 16> channel_saw_program_change{};
        std::array<std::map<int, detail::HeldNote>, 16> held_notes{};
        std::map<std::pair<int, int>, std::vector<ResolvedNote>> notes_by_channel_program;

        auto finalize_note = [&](int channel, int note, uint64_t end_tick, const detail::HeldNote& held) {
            double t0 = tempo_map.tick_to_seconds(held.start_tick);
            double t1 = tempo_map.tick_to_seconds(end_tick);
            double dur = t1 - t0;
            if (dur <= 0.0) return;
            ResolvedNote rn;
            rn.t = t0;
            rn.dur = dur;
            rn.midi_note = note;
            rn.dyn = detail::velocity_to_dynamics_u8(held.velocity);
            rn.expression = held.expression_at_onset;
            rn.pitch_bend_semitones = held.bend_at_onset;
            rn.glide_from_midi = held.glide_from_midi;
            notes_by_channel_program[{channel, held.program_at_onset}].push_back(rn);
        };

        for (const midi::MidiEvent& ev : raw_track.events) {
            if (ev.channel >= 16) continue;
            detail::ChannelState& cs = chan_state[ev.channel];
            auto& held = held_notes[ev.channel];

            switch (ev.type) {
                case midi::MidiEventType::ProgramChange:
                    cs.program = ev.data1;
                    channel_saw_program_change[static_cast<size_t>(ev.channel)] = true;
                    break;

                case midi::MidiEventType::ControlChange: {
                    if (ev.data1 == 64) {
                        bool was_on = cs.sustain_on;
                        cs.sustain_on = ev.data2 >= 64;
                        if (opts.honor_sustain_pedal && was_on && !cs.sustain_on) {
                            for (auto it = held.begin(); it != held.end();) {
                                if (it->second.pending_pedal_release) {
                                    finalize_note(ev.channel, it->first, ev.abs_tick, it->second);
                                    it = held.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                        }
                    } else if (ev.data1 == 65) {
                        cs.portamento_on = ev.data2 >= 64;
                    } else if (ev.data1 == 11) {
                        cs.expression = static_cast<double>(ev.data2) / 127.0;
                    } else if (ev.data1 == 7) {
                        cs.volume = static_cast<double>(ev.data2) / 127.0;
                    }
                    break;
                }

                case midi::MidiEventType::PitchBend: {
                    int raw14 = (static_cast<int>(ev.data2) << 7) | static_cast<int>(ev.data1);
                    double normalized = (raw14 - 8192) / 8192.0;
                    cs.pitch_bend_semitones = normalized * opts.pitch_bend_range_semitones;
                    break;
                }

                case midi::MidiEventType::NoteOn: {
                    auto existing = held.find(ev.data1);
                    if (existing != held.end()) {
                        finalize_note(ev.channel, ev.data1, ev.abs_tick, existing->second);
                        held.erase(existing);
                    }
                    detail::HeldNote hn;
                    hn.start_tick = ev.abs_tick;
                    hn.velocity = ev.data2;
                    hn.expression_at_onset = cs.expression * cs.volume;
                    hn.bend_at_onset = cs.pitch_bend_semitones;
                    hn.program_at_onset = cs.program;
                    hn.glide_from_midi = (opts.honor_portamento && cs.portamento_on &&
                                          cs.last_note_on_pitch >= 0 && cs.last_note_on_pitch != ev.data1)
                                             ? cs.last_note_on_pitch : -1;
                    held[ev.data1] = hn;
                    cs.last_note_on_pitch = ev.data1;
                    break;
                }

                case midi::MidiEventType::NoteOff: {
                    auto it = held.find(ev.data1);
                    if (it == held.end()) break;
                    if (opts.honor_sustain_pedal && cs.sustain_on) {
                        it->second.pending_pedal_release = true;
                    } else {
                        finalize_note(ev.channel, ev.data1, ev.abs_tick, it->second);
                        held.erase(it);
                    }
                    break;
                }

                default:
                    break;
            }
        }

        for (int ch = 0; ch < 16; ++ch) {
            for (auto& [note, hn] : held_notes[static_cast<size_t>(ch)])
                finalize_note(ch, note, max_tick, hn);
        }

        for (auto& [key, notes] : notes_by_channel_program) {
            const int channel = key.first;
            const int program = key.second;
            if (notes.empty()) continue;
            std::stable_sort(notes.begin(), notes.end(),
                              [](const ResolvedNote& a, const ResolvedNote& b) { return a.t < b.t; });

            MidiPart part;
            part.source_track_index = static_cast<int>(ti);
            part.channel = channel;
            part.gm_program = program;
            part.name = !raw_track.name.empty() ? raw_track.name
                                                 : ("Track" + std::to_string(ti) + "_Ch" + std::to_string(channel));

            if (midi::InstrumentResolver::is_percussion_channel(channel)) {
                part.is_percussion = true;
                auto kit = std::make_shared<audio::DrumKit>(part.name);
                for (auto& rn : notes)
                    if (!kit->has(rn.midi_note))
                        kit->map(rn.midi_note, opts.resolver.resolve_percussion(channel, rn.midi_note));
                part.drum_kit = kit;
            } else {
                int effective_program = program;
                if (program == 0 && !channel_saw_program_change[static_cast<size_t>(channel)]) {
                    int guessed = -1;
                    if (midi::guess_program_from_name(raw_track.name, guessed)) effective_program = guessed;
                }
                part.instrument = opts.resolver.resolve_melodic(channel, effective_program);
            }

            part.notes = std::move(notes);
            music.parts.push_back(std::move(part));
        }
    }

    return music;
}

}
}
