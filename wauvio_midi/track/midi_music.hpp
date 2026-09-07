#pragma once

#include "../resolution/instrument_resolver.hpp"
#include "../core/midi_types.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace wauvio {
namespace track {

struct ResolvedNote {
    double t = 0.0;
    double dur = 0.0;
    int midi_note = 60;
    audio::Dynamics dyn = audio::Dynamics::mf;
    double expression = 1.0;
    double pitch_bend_semitones = 0.0;
    int glide_from_midi = -1;
};

class MidiPart {
public:
    std::string name;
    int source_track_index = -1;
    int channel = 0;
    int gm_program = 0;
    bool is_percussion = false;

    midi::InstrumentPtr instrument;
    std::shared_ptr<audio::DrumKit> drum_kit;

    std::vector<ResolvedNote> notes;
};

class MidiMusic {
public:
    std::vector<MidiPart> parts;
    double duration_seconds = 0.0;
    double initial_bpm = 120.0;
    uint16_t ticks_per_quarter = 480;
    std::string source_path;

    std::vector<midi::TimeSignatureEvent> time_signatures;
    std::vector<midi::KeySignatureEvent> key_signatures;

    size_t part_count() const noexcept { return parts.size(); }
    const MidiPart& part(size_t index) const { return parts.at(index); }
    MidiPart& part(size_t index) { return parts.at(index); }

    template <typename T>
    void set_instrument(size_t part_index) {
        if (part_index >= parts.size()) return;
        parts[part_index].instrument = std::make_shared<T>();
        parts[part_index].drum_kit.reset();
        parts[part_index].is_percussion = false;
    }

    void set_instrument(size_t part_index, midi::InstrumentPtr instr) {
        if (part_index >= parts.size()) return;
        parts[part_index].instrument = std::move(instr);
        parts[part_index].drum_kit.reset();
        parts[part_index].is_percussion = false;
    }

    void set_drum_kit(size_t part_index, std::shared_ptr<audio::DrumKit> kit) {
        if (part_index >= parts.size()) return;
        parts[part_index].drum_kit = std::move(kit);
        parts[part_index].instrument.reset();
        parts[part_index].is_percussion = true;
    }

    StereoBuffer render(int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        size_t total = static_cast<size_t>((duration_seconds + 2.0) * sample_rate) + 1;
        StereoBuffer mix(total, 0.0f);

        for (const auto& p : parts) {
            for (const auto& n : p.notes) {
                if (n.dur <= 0.0) continue;

                StereoBuffer note_audio;
                if (p.is_percussion) {
                    if (!p.drum_kit) continue;
                    audio::PlayedNote pn = p.drum_kit->play(n.midi_note, n.dyn,
                                                             std::min(0.5, std::max(0.08, n.dur)),
                                                             sample_rate);
                    note_audio = std::move(pn.audio);
                } else {
                    if (!p.instrument) continue;
                    audio::Note note(n.midi_note, n.dur, n.dyn);
                    note.expression = n.expression;
                    note.pitch_bend_semitones = n.pitch_bend_semitones;
                    note.glide_from_midi = n.glide_from_midi;
                    audio::PlayedNote pn = p.instrument->play(note, sample_rate);
                    note_audio = std::move(pn.audio);
                }

                const size_t off = static_cast<size_t>(n.t * sample_rate);
                for (size_t i = 0; i < note_audio.size() && off + i < mix.size(); ++i) {
                    mix.L[off + i] += note_audio.L[i];
                    mix.R[off + i] += note_audio.R[i];
                }
            }
        }

        normalize(mix, 0.92f);
        clamp_buffer(mix);
        return mix;
    }
};

}
}
