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

enum class InterpolationQuality { Nearest, Linear, Cubic };

inline InterpolationQuality& default_interpolation_quality() {
    static InterpolationQuality q = InterpolationQuality::Linear;
    return q;
}

enum class LoopMode { None, Forward, PingPong };

struct SampleZone {
    StereoBuffer audio;
    std::shared_ptr<const StereoBuffer> shared_audio;
    int          sample_rate = 44100;

    int  root_note = 60;
    int  low_note  = 0,   high_note = 127;
    int  low_vel   = 0,   high_vel  = 127;

    size_t start = 0, end = 0;
    bool   looping    = false;
    LoopMode loop_mode = LoopMode::None;
    size_t loop_start = 0, loop_end = 0;
    size_t loop_crossfade_samples = 0;

    double coarse_tune_semitones = 0.0;
    double fine_tune_cents       = 0.0;
    double pan                   = 0.0;
    double attenuation_db        = 0.0;
    double filter_cutoff_hz      = 20000.0;
    double filter_q              = 0.7;
    double scale_tuning          = 1.0;
    DAHDSR envelope { 0.0, 0.0, 0.0, 0.0, 1.0, 0.005 };

    int          round_robin_group = 0;
    int          exclusive_class   = 0;
    Articulation articulation      = Articulation::Sustain;
    bool         is_release_sample = false;

    const StereoBuffer& buffer() const { return shared_audio ? *shared_audio : audio; }

    LoopMode effective_loop_mode() const {
        if (loop_mode != LoopMode::None) return loop_mode;
        return looping ? LoopMode::Forward : LoopMode::None;
    }

    size_t effective_end() const { return end > 0 ? end : buffer().size(); }

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
    static StereoBuffer render(const SampleZone& zone, const Note& note, int sample_rate,
                                InterpolationQuality quality = InterpolationQuality::Linear)
    {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t n_out = static_cast<size_t>(std::max(0.0, note.duration) * sample_rate);
        StereoBuffer out(n_out);
        const StereoBuffer& src = zone.buffer();
        if (n_out == 0 || src.empty()) return out;

        const double sr_ratio  = static_cast<double>(zone.sample_rate) / static_cast<double>(sample_rate);
        const double expr_gain = std::max(0.0, std::min(1.0, note.expression));
        const double atten_gain = std::pow(10.0, zone.attenuation_db / 20.0);
        const double velocity_gain = dynamics_to_velocity(note.dynamics);
        const double tune_ratio = std::pow(2.0, (zone.coarse_tune_semitones + zone.fine_tune_cents / 100.0) / 12.0);
        const bool has_continuous_bend = !note.pitch_bend_curve.empty();
        const double const_bend_ratio = std::pow(2.0, note.pitch_bend_semitones / 12.0);

        const size_t start = std::min(zone.start, src.size());
        const size_t end   = std::min(zone.effective_end(), src.size());
        const size_t loop_s = std::min(zone.loop_start, src.size());
        const size_t loop_e = (zone.loop_end > 0) ? std::min(zone.loop_end, src.size()) : end;
        const LoopMode loop_mode = zone.effective_loop_mode();
        const bool can_loop = loop_mode != LoopMode::None && loop_e > loop_s;

        size_t crossfade = 0;
        if (can_loop && loop_mode == LoopMode::Forward) {
            size_t loop_len = loop_e - loop_s;
            size_t requested = zone.loop_crossfade_samples > 0 ? zone.loop_crossfade_samples : 32;
            crossfade = std::min(requested, loop_len / 4);
        }

        auto sample_raw = [&](size_t idx, int ch) -> float {
            idx = std::min(idx, src.size() - 1);
            return ch == 0 ? src.L[idx] : src.R[idx];
        };

        auto interp_at = [&](double pos, int ch) -> float {
            long i0 = static_cast<long>(std::floor(pos));
            double frac = pos - static_cast<double>(i0);
            if (i0 < 0) i0 = 0;
            size_t u0 = static_cast<size_t>(i0);

            switch (quality) {
                case InterpolationQuality::Nearest:
                    return sample_raw(frac < 0.5 ? u0 : u0 + 1, ch);
                case InterpolationQuality::Cubic: {
                    size_t um1 = (u0 > 0) ? u0 - 1 : 0;
                    size_t u1  = u0 + 1;
                    size_t u2  = u0 + 2;
                    float ym1 = sample_raw(um1, ch), y0 = sample_raw(u0, ch);
                    float y1  = sample_raw(u1, ch),  y2 = sample_raw(u2, ch);
                    float f = static_cast<float>(frac);
                    float a0 = y0, a1 = 0.5f * (y1 - ym1);
                    float a2 = ym1 - 2.5f * y0 + 2.0f * y1 - 0.5f * y2;
                    float a3 = 0.5f * (y2 - ym1) + 1.5f * (y0 - y1);
                    return ((a3 * f + a2) * f + a1) * f + a0;
                }
                case InterpolationQuality::Linear:
                default: {
                    float y0 = sample_raw(u0, ch), y1 = sample_raw(u0 + 1, ch);
                    return y0 * (1.0f - static_cast<float>(frac)) + y1 * static_cast<float>(frac);
                }
            }
        };

        auto read_ch = [&](double pos, int ch) -> float {
            if (crossfade > 0 && pos >= static_cast<double>(loop_e) - static_cast<double>(crossfade) &&
                pos < static_cast<double>(loop_e)) {
                double tail_pos = pos;
                double head_pos = static_cast<double>(loop_s) + (pos - (static_cast<double>(loop_e) - static_cast<double>(crossfade)));
                double fade = (pos - (static_cast<double>(loop_e) - static_cast<double>(crossfade))) / static_cast<double>(crossfade);
                float tail_v = interp_at(tail_pos, ch);
                float head_v = interp_at(head_pos, ch);
                return static_cast<float>(tail_v * (1.0 - fade) + head_v * fade);
            }
            return interp_at(pos, ch);
        };

        double pos = static_cast<double>(start);
        bool forward_dir = true;
        const bool needs_filter = zone.filter_cutoff_hz < 19500.0;
        SVFilter filt_l(SVFilter::Mode::LowPass, std::max(20.0, zone.filter_cutoff_hz), std::max(0.1, zone.filter_q));
        SVFilter filt_r(SVFilter::Mode::LowPass, std::max(20.0, zone.filter_cutoff_hz), std::max(0.1, zone.filter_q));

        for (size_t i = 0; i < n_out; ++i) {
            double t = static_cast<double>(i) / sample_rate;
            double bend_ratio = has_continuous_bend ? std::pow(2.0, note.bend_at(t) / 12.0) : const_bend_ratio;
            double pitch_ratio = std::pow(2.0, (note.midi_note - zone.root_note) * zone.scale_tuning / 12.0)
                                * bend_ratio * tune_ratio;
            double read_step = pitch_ratio * sr_ratio;

            if (can_loop) {
                if (loop_mode == LoopMode::Forward) {
                    if (pos >= static_cast<double>(loop_e)) {
                        double loop_len = static_cast<double>(loop_e - loop_s);
                        pos = static_cast<double>(loop_s) + std::fmod(pos - static_cast<double>(loop_e), loop_len);
                    }
                } else if (loop_mode == LoopMode::PingPong) {
                    double loop_len = static_cast<double>(loop_e - loop_s);
                    if (forward_dir && pos >= static_cast<double>(loop_e)) {
                        double over = pos - static_cast<double>(loop_e);
                        pos = static_cast<double>(loop_e) - std::fmod(over, loop_len);
                        forward_dir = false;
                    } else if (!forward_dir && pos <= static_cast<double>(loop_s)) {
                        double under = static_cast<double>(loop_s) - pos;
                        pos = static_cast<double>(loop_s) + std::fmod(under, loop_len);
                        forward_dir = true;
                    }
                }
            } else if (pos >= static_cast<double>(end)) {
                pos = static_cast<double>(end) - 1e-6;
            }

            float l = read_ch(pos, 0) * static_cast<float>(expr_gain * atten_gain * velocity_gain);
            float r = src.size() > 0 ? read_ch(pos, 1) * static_cast<float>(expr_gain * atten_gain * velocity_gain) : l;
            if (needs_filter) {
                l = filt_l.tick(l, sample_rate);
                r = filt_r.tick(r, sample_rate);
            }
            out.L[i] = l;
            out.R[i] = r;

            double step = read_step * (loop_mode == LoopMode::PingPong && !forward_dir ? -1.0 : 1.0);
            pos += step;
        }

        zone.envelope.apply(out.L, note.duration, -1.0, sample_rate);
        zone.envelope.apply(out.R, note.duration, -1.0, sample_rate);
        fade_in(out, 0.002, sample_rate);
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
                return SamplePlayer::render(*z, note, sample_rate, default_interpolation_quality());
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
