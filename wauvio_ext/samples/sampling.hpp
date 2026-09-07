#pragma once

#include "../core/core.hpp"

#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <algorithm>

namespace wauvio {
namespace audio {

struct WavData {
    StereoBuffer audio;
    bool         was_mono = true;
    int          sample_rate = 44100;
};

namespace wav_detail {

inline uint32_t read_u32(const unsigned char* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}
inline uint16_t read_u16(const unsigned char* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

}

inline WavData load_wav(const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) throw std::runtime_error("load_wav: cannot open " + path);

    std::vector<unsigned char> raw;
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (sz <= 44) { std::fclose(f); throw std::runtime_error("load_wav: file too small: " + path); }
    raw.resize(static_cast<size_t>(sz));
    size_t got = std::fread(raw.data(), 1, raw.size(), f);
    std::fclose(f);
    if (got != raw.size()) throw std::runtime_error("load_wav: short read: " + path);

    if (raw.size() < 12 || std::memcmp(raw.data(), "RIFF", 4) != 0 ||
        std::memcmp(raw.data() + 8, "WAVE", 4) != 0)
        throw std::runtime_error("load_wav: not a RIFF/WAVE file: " + path);

    uint16_t format_tag = 1, channels = 1, bits_per_sample = 16;
    uint32_t sample_rate = 44100;
    const unsigned char* data_ptr = nullptr;
    uint32_t data_size = 0;

    size_t pos = 12;
    while (pos + 8 <= raw.size()) {
        char id[5] = {0};
        std::memcpy(id, &raw[pos], 4);
        uint32_t chunk_size = wav_detail::read_u32(&raw[pos + 4]);
        size_t body = pos + 8;
        if (body + chunk_size > raw.size()) chunk_size = static_cast<uint32_t>(raw.size() - body);

        if (std::memcmp(id, "fmt ", 4) == 0 && chunk_size >= 16) {
            format_tag      = wav_detail::read_u16(&raw[body]);
            channels        = wav_detail::read_u16(&raw[body + 2]);
            sample_rate     = wav_detail::read_u32(&raw[body + 4]);
            bits_per_sample = wav_detail::read_u16(&raw[body + 14]);
        } else if (std::memcmp(id, "data", 4) == 0) {
            data_ptr  = &raw[body];
            data_size = chunk_size;
        }
        pos = body + chunk_size + (chunk_size & 1);
    }

    if (!data_ptr || channels == 0) throw std::runtime_error("load_wav: missing fmt/data chunk: " + path);

    const size_t bytes_per_sample = bits_per_sample / 8;
    const size_t frame_bytes      = bytes_per_sample * channels;
    const size_t n_frames         = frame_bytes > 0 ? data_size / frame_bytes : 0;

    WavData out;
    out.sample_rate = static_cast<int>(sample_rate);
    out.was_mono    = (channels == 1);
    out.audio.resize(n_frames);

    auto sample_at = [&](size_t frame, int ch) -> float {
        const unsigned char* p = data_ptr + frame * frame_bytes + ch * bytes_per_sample;
        if (format_tag == 3 && bits_per_sample == 32) {
            float v; std::memcpy(&v, p, 4); return v;
        }
        if (bits_per_sample == 8) {
            return (static_cast<int>(p[0]) - 128) / 128.0f;
        } else if (bits_per_sample == 16) {
            int16_t v = static_cast<int16_t>(wav_detail::read_u16(p));
            return v / 32768.0f;
        } else if (bits_per_sample == 24) {
            int32_t v = (p[0]) | (p[1] << 8) | (p[2] << 16);
            if (v & 0x800000) v |= ~0xFFFFFF;
            return v / 8388608.0f;
        } else if (bits_per_sample == 32) {
            int32_t v = static_cast<int32_t>(wav_detail::read_u32(p));
            return v / 2147483648.0f;
        }
        return 0.0f;
    };

    for (size_t i = 0; i < n_frames; ++i) {
        float l = sample_at(i, 0);
        float r = (channels >= 2) ? sample_at(i, 1) : l;
        out.audio.L[i] = l;
        out.audio.R[i] = r;
    }
    return out;
}

struct SampleZone {
    StereoBuffer audio;
    int          sample_rate = 44100;

    int  root_note = 60;
    int  low_note  = 0,   high_note = 127;
    int  low_vel   = 0,   high_vel  = 127;

    size_t start = 0, end = 0;
    bool   looping    = false;
    size_t loop_start = 0, loop_end = 0;

    int          round_robin_group = 0;
    Articulation articulation      = Articulation::Sustain;
    bool         is_release_sample = false;

    size_t effective_end() const { return end > 0 ? end : audio.size(); }

    bool matches(int midi_note, int velocity127, Articulation art) const {
        if (midi_note < low_note || midi_note > high_note) return false;
        if (velocity127 < low_vel || velocity127 > high_vel) return false;
        if (art != articulation) return false;
        return true;
    }
};

class MultiSample {
public:
    void add_zone(SampleZone z) { zones_.push_back(std::move(z)); }
    bool empty() const { return zones_.empty(); }
    size_t zone_count() const { return zones_.size(); }

    const SampleZone* find_zone(int midi_note, double velocity01, Articulation art) const {
        if (zones_.empty()) return nullptr;
        const int vel127 = static_cast<int>(std::round(std::max(0.0, std::min(1.0, velocity01)) * 127.0));

        std::vector<const SampleZone*> candidates;
        for (auto& z : zones_)
            if (!z.is_release_sample && z.matches(midi_note, vel127, art))
                candidates.push_back(&z);

        if (candidates.empty() && art != Articulation::Sustain) {
            for (auto& z : zones_)
                if (!z.is_release_sample && z.matches(midi_note, vel127, Articulation::Sustain))
                    candidates.push_back(&z);
        }

        if (candidates.empty()) {
            const SampleZone* best = nullptr;
            int best_dist = INT32_MAX;
            for (auto& z : zones_) {
                if (z.is_release_sample) continue;
                int dist = (midi_note < z.low_note) ? z.low_note - midi_note
                         : (midi_note > z.high_note) ? midi_note - z.high_note : 0;
                if (dist < best_dist) { best_dist = dist; best = &z; }
            }
            return best;
        }
        if (candidates.size() == 1) return candidates[0];

        int key = midi_note * 100 + static_cast<int>(art);
        int& counter = rr_counters_[key];
        const SampleZone* chosen = candidates[static_cast<size_t>(counter) % candidates.size()];
        counter++;
        return chosen;
    }

private:
    std::vector<SampleZone> zones_;
    mutable std::unordered_map<int, int> rr_counters_;
};

class SamplePlayer {
public:
    static StereoBuffer render(const SampleZone& zone, const Note& note, int sample_rate) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t n_out = static_cast<size_t>(std::max(0.0, note.duration) * sample_rate);
        StereoBuffer out(n_out);
        if (n_out == 0 || zone.audio.empty()) return out;

        const double bend_ratio  = std::pow(2.0, note.pitch_bend_semitones / 12.0);
        const double pitch_ratio = (midi_to_freq(note.midi_note) / midi_to_freq(zone.root_note)) * bend_ratio;
        const double sr_ratio    = static_cast<double>(zone.sample_rate) / static_cast<double>(sample_rate);
        const double read_step   = pitch_ratio * sr_ratio;
        const double expr_gain   = std::max(0.0, std::min(1.0, note.expression));

        const size_t start = std::min(zone.start, zone.audio.size());
        const size_t end   = std::min(zone.effective_end(), zone.audio.size());
        const size_t loop_s = std::min(zone.loop_start, zone.audio.size());
        const size_t loop_e = (zone.loop_end > 0) ? std::min(zone.loop_end, zone.audio.size()) : end;

        double pos = static_cast<double>(start);
        for (size_t i = 0; i < n_out; ++i) {
            if (zone.looping && loop_e > loop_s && pos >= static_cast<double>(loop_e))
                pos = loop_s + std::fmod(pos - loop_s, static_cast<double>(loop_e - loop_s));
            if (!zone.looping && pos >= static_cast<double>(end)) {
                pos = static_cast<double>(end) - 1e-6;
            }

            size_t i0 = static_cast<size_t>(pos);
            size_t i1 = std::min(i0 + 1, zone.audio.size() - 1);
            float  frac = static_cast<float>(pos - static_cast<double>(i0));

            out.L[i] = (zone.audio.L[i0] * (1.0f - frac) + zone.audio.L[i1] * frac) * static_cast<float>(expr_gain);
            out.R[i] = (zone.audio.R[i0] * (1.0f - frac) + zone.audio.R[i1] * frac) * static_cast<float>(expr_gain);

            pos += read_step;
        }

        fade_in(out, 0.002, sample_rate);
        fade_out(out, 0.006, sample_rate);
        return out;
    }
};

class SampledInstrument : public Instrument {
public:
    using SynthModel = std::function<StereoBuffer(const Note&, int sample_rate)>;

    MultiSample                samples;
    SynthModel                 fallback_model;
    std::vector<Articulation>  articulations = { Articulation::Sustain };

    StereoBuffer render_note(const Note& note, int sample_rate = 0) const override {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        if (note.is_rest() || note.duration <= 0.0)
            return make_stereo(std::max(0.0, note.duration), sample_rate);

        if (!samples.empty()) {
            double vel = dynamics_to_velocity(note.dynamics);
            if (const SampleZone* z = samples.find_zone(note.midi_note, vel, note.articulation))
                return SamplePlayer::render(*z, note, sample_rate);
        }
        if (fallback_model) return fallback_model(note, sample_rate);
        return make_stereo(note.duration, sample_rate);
    }

    std::vector<Articulation> supported_articulations() const override { return articulations; }

    SampledInstrument& load_samples(std::initializer_list<SampleZone> zones) {
        for (auto& z : zones) samples.add_zone(z);
        return *this;
    }

    SampledInstrument& load_sample_file(const std::string& path, int root_note,
                                         int low_note = 0, int high_note = 127,
                                         int low_vel = 0, int high_vel = 127,
                                         Articulation art = Articulation::Sustain,
                                         int round_robin_group = 0)
    {
        WavData w = load_wav(path);
        SampleZone z;
        z.audio = std::move(w.audio);
        z.sample_rate = w.sample_rate;
        z.root_note = root_note;
        z.low_note = low_note; z.high_note = high_note;
        z.low_vel = low_vel; z.high_vel = high_vel;
        z.articulation = art;
        z.round_robin_group = round_robin_group;
        samples.add_zone(std::move(z));
        return *this;
    }
};

}
}
