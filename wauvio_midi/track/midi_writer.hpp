#pragma once

#include "midi_music.hpp"
#include "../core/tempo_map.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>

namespace wauvio {
namespace track {

namespace writer_detail {

inline void write_vlq(std::vector<uint8_t>& out, uint32_t value) {
    uint32_t buffer = value & 0x7F;
    while ((value >>= 7)) {
        buffer <<= 8;
        buffer |= ((value & 0x7F) | 0x80);
    }
    while (true) {
        out.push_back(static_cast<uint8_t>(buffer & 0xFF));
        if (buffer & 0x80) buffer >>= 8;
        else break;
    }
}

inline void write_u32be(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}
inline void write_u16be(std::vector<uint8_t>& out, uint16_t v) {
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

struct RawEvent {
    uint64_t tick;
    std::vector<uint8_t> bytes;
};

class MidiTrackBuilder {
public:
    void add(uint64_t tick, std::vector<uint8_t> bytes) { events_.push_back({tick, std::move(bytes)}); }

    void add_track_name(uint64_t tick, const std::string& name) {
        std::vector<uint8_t> b{0xFF, 0x03};
        write_vlq(b, static_cast<uint32_t>(name.size()));
        b.insert(b.end(), name.begin(), name.end());
        add(tick, std::move(b));
    }

    void add_tempo(uint64_t tick, uint32_t micros_per_quarter) {
        std::vector<uint8_t> b{0xFF, 0x51, 0x03,
            static_cast<uint8_t>((micros_per_quarter >> 16) & 0xFF),
            static_cast<uint8_t>((micros_per_quarter >> 8) & 0xFF),
            static_cast<uint8_t>(micros_per_quarter & 0xFF)};
        add(tick, std::move(b));
    }

    void add_program_change(uint64_t tick, int channel, int program) {
        add(tick, {static_cast<uint8_t>(0xC0 | (channel & 0x0F)), static_cast<uint8_t>(program & 0x7F)});
    }

    void add_control_change(uint64_t tick, int channel, int controller, int value) {
        add(tick, {static_cast<uint8_t>(0xB0 | (channel & 0x0F)),
                   static_cast<uint8_t>(controller & 0x7F), static_cast<uint8_t>(value & 0x7F)});
    }

    void add_note_on(uint64_t tick, int channel, int note, int velocity) {
        add(tick, {static_cast<uint8_t>(0x90 | (channel & 0x0F)),
                   static_cast<uint8_t>(note & 0x7F), static_cast<uint8_t>(std::max(1, velocity) & 0x7F)});
    }

    void add_note_off(uint64_t tick, int channel, int note) {
        add(tick, {static_cast<uint8_t>(0x80 | (channel & 0x0F)), static_cast<uint8_t>(note & 0x7F), 0});
    }

    std::vector<uint8_t> serialize() const {
        std::vector<RawEvent> sorted = events_;
        std::stable_sort(sorted.begin(), sorted.end(),
                          [](const RawEvent& a, const RawEvent& b) { return a.tick < b.tick; });

        std::vector<uint8_t> body;
        uint64_t last_tick = 0;
        for (auto& e : sorted) {
            uint32_t delta = static_cast<uint32_t>(e.tick - last_tick);
            last_tick = e.tick;
            write_vlq(body, delta);
            body.insert(body.end(), e.bytes.begin(), e.bytes.end());
        }
        write_vlq(body, 0);
        body.insert(body.end(), {0xFF, 0x2F, 0x00});

        std::vector<uint8_t> chunk;
        chunk.insert(chunk.end(), {'M','T','r','k'});
        write_u32be(chunk, static_cast<uint32_t>(body.size()));
        chunk.insert(chunk.end(), body.begin(), body.end());
        return chunk;
    }

private:
    std::vector<RawEvent> events_;
};

inline void write_file(const std::string& path, uint16_t format, uint16_t ntrks, uint16_t division,
                        const std::vector<std::vector<uint8_t>>& track_chunks)
{
    std::vector<uint8_t> out;
    out.insert(out.end(), {'M','T','h','d'});
    write_u32be(out, 6);
    write_u16be(out, format);
    write_u16be(out, ntrks);
    write_u16be(out, division);
    for (auto& tc : track_chunks) out.insert(out.end(), tc.begin(), tc.end());

    std::ofstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("save_midi: cannot open for writing: " + path);
    f.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
}

}

inline void save_midi(const audio::Arrangement& arr, const std::string& path, int ppq = 480) {
    using namespace writer_detail;
    double bpm = arr.tempo.bpm > 0.0 ? arr.tempo.bpm : 120.0;
    double ticks_per_sec = ppq * bpm / 60.0;
    uint32_t mpq = static_cast<uint32_t>(std::llround(60000000.0 / bpm));

    std::vector<std::vector<uint8_t>> chunks;

    MidiTrackBuilder tempo_track;
    tempo_track.add_tempo(0, mpq);
    chunks.push_back(tempo_track.serialize());

    for (size_t ti = 0; ti < arr.track_count(); ++ti) {
        const audio::Track& t = arr.track_at(ti);
        int channel = static_cast<int>(ti < 9 ? ti : ti + 1) % 16;

        MidiTrackBuilder tb;
        if (!t.name.empty()) tb.add_track_name(0, t.name);
        tb.add_program_change(0, channel, 0);

        auto onsets = t.entry_onsets();
        double base_offset = arr.start_time_at(ti);
        for (size_t ei = 0; ei < t.entry_count(); ++ei) {
            double onset_sec = base_offset + onsets[ei];
            uint64_t onset_tick = static_cast<uint64_t>(std::llround(onset_sec * ticks_per_sec));

            if (t.entry_is_chord(ei)) {
                const audio::Chord& c = t.entry_chord(ei);
                uint64_t end_tick = static_cast<uint64_t>(std::llround((onset_sec + c.duration) * ticks_per_sec));
                int vel = static_cast<int>(std::lround(audio::dynamics_to_velocity(c.dynamics) * 127.0));
                for (int note : c.midi_notes) tb.add_note_on(onset_tick, channel, note, vel);
                for (int note : c.midi_notes) tb.add_note_off(end_tick, channel, note);
            } else {
                const audio::Note& n = t.entry_note(ei);
                if (n.is_rest()) continue;
                uint64_t end_tick = static_cast<uint64_t>(std::llround((onset_sec + n.duration) * ticks_per_sec));
                int vel = static_cast<int>(std::lround(audio::dynamics_to_velocity(n.dynamics) * 127.0));
                tb.add_note_on(onset_tick, channel, n.midi_note, vel);
                tb.add_note_off(end_tick, channel, n.midi_note);
            }
        }
        chunks.push_back(tb.serialize());
    }

    write_file(path, 1, static_cast<uint16_t>(chunks.size()), static_cast<uint16_t>(ppq), chunks);
}

inline void save_midi(const MidiMusic& music, const std::string& path) {
    using namespace writer_detail;
    int ppq = music.ticks_per_quarter > 0 ? music.ticks_per_quarter : 480;

    midi::TempoMap tempo_map;
    if (music.raw_midi) {
        for (auto& tr : music.raw_midi->tracks)
            for (auto& ev : tr.events)
                if (ev.type == midi::MidiEventType::Meta && ev.meta_type == 0x51 && ev.meta_data.size() == 3) {
                    uint32_t mpq = (static_cast<uint32_t>(ev.meta_data[0]) << 16) |
                                   (static_cast<uint32_t>(ev.meta_data[1]) << 8) |
                                   static_cast<uint32_t>(ev.meta_data[2]);
                    if (mpq > 0) tempo_map.add_tempo_event(ev.abs_tick, mpq);
                }
        tempo_map.finalize(static_cast<uint16_t>(ppq));
    } else {
        tempo_map.add_tempo_event(0, static_cast<uint32_t>(std::llround(60000000.0 / std::max(1.0, music.initial_bpm))));
        tempo_map.finalize(static_cast<uint16_t>(ppq));
    }

    std::vector<std::vector<uint8_t>> chunks;

    MidiTrackBuilder tempo_track;
    for (auto& te : tempo_map.tempo_events()) tempo_track.add_tempo(te.first, te.second);
    chunks.push_back(tempo_track.serialize());

    for (size_t pi = 0; pi < music.part_count(); ++pi) {
        const MidiPart& p = music.part(pi);
        MidiTrackBuilder tb;
        if (!p.name.empty()) tb.add_track_name(0, p.name);
        if (p.bank_msb != 0) tb.add_control_change(0, p.channel, 0, p.bank_msb);
        if (p.bank_lsb != 0) tb.add_control_change(0, p.channel, 32, p.bank_lsb);
        if (!p.is_percussion) tb.add_program_change(0, p.channel, p.gm_program);

        for (auto& n : p.notes) {
            uint64_t onset_tick = tempo_map.seconds_to_ticks(n.t);
            uint64_t end_tick = tempo_map.seconds_to_ticks(n.t + n.dur);
            int vel = static_cast<int>(std::lround(audio::dynamics_to_velocity(n.dyn) * 127.0));
            tb.add_note_on(onset_tick, p.channel, n.midi_note, vel);
            tb.add_note_off(end_tick, p.channel, n.midi_note);
        }
        chunks.push_back(tb.serialize());
    }

    write_file(path, 1, static_cast<uint16_t>(chunks.size()), static_cast<uint16_t>(ppq), chunks);
}

}
}
