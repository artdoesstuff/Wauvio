#include "wauvio.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <thread>

using namespace wauvio;

constexpr int    SR    = 44100;
constexpr double BPM   = 360.0;
constexpr double BAR   = 60.0 / BPM * 4.0;
constexpr double BEAT  = BAR / 4.0;
constexpr double STEP  = BAR / 16.0;
constexpr double STEP8 = BAR / 8.0;

struct HarmonicArea {
    std::vector<double> pad;
    double              root;
    std::vector<double> mel_a;
    std::vector<double> mel_b;
    std::vector<double> stab;
};

const std::vector<HarmonicArea> AREAS = {
    {
        {notes::C4, notes::E4, notes::G4},
        notes::C3 * 0.5,   // C2
        { notes::C5, notes::G4, notes::E5, notes::C5,
          notes::G5, notes::E5, notes::G4, notes::C5 },
        { notes::E5, notes::C5, notes::G5, notes::E5,
          notes::C5, notes::G4, notes::E4, notes::G4 },
        { notes::C4, notes::G4 }
    },
    {
        {notes::A3, notes::C4, notes::E4},
        notes::A2 * 0.5,   // A1
        { notes::A4, notes::E4, notes::C5, notes::A4,
          notes::E5, notes::C5, notes::A5, notes::E5 },
        { notes::C5, notes::A4, notes::E5, notes::C5,
          notes::A4, notes::E4, notes::C4, notes::E4 },
        { notes::A3, notes::E4 }
    },
    {
        {notes::F3, notes::A3, notes::C4},
        notes::F3 * 0.5,   // F1
        { notes::F4, notes::C4, notes::A4, notes::F4,
          notes::C5, notes::A4, notes::F5, notes::C5 },
        { notes::A4, notes::F4, notes::C5, notes::A4,
          notes::F4, notes::C4, notes::A3, notes::C4 },
        { notes::F3, notes::C4 }
    },
    {
        {notes::G3, notes::B3, notes::D4},
        notes::G3 * 0.5,   // G1
        { notes::G4, notes::D4, notes::B4, notes::G4,
          notes::D5, notes::B4, notes::G5, notes::D5 },
        { notes::B4, notes::G4, notes::D5, notes::B4,
          notes::G4, notes::D4, notes::B3, notes::D4 },
        { notes::G3, notes::D4 }
    },
};

Buffer make_kick(int sr) {
    constexpr double DUR = 0.18;
    Buffer buf = make_buffer(DUR, sr);

    double phase = 0.0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        const double f = 38.0 + 152.0 * std::exp(-t * 55.0);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        const double env = std::exp(-t * 14.0);
        buf[i] = static_cast<float>(std::sin(phase) * env);
    }

    {
        double ph2 = 0.0;
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            const double f = 19.0 + 70.0 * std::exp(-t * 70.0);
            ph2 += TWO_PI * f / sr;
            if (ph2 >= TWO_PI) ph2 -= TWO_PI;
            buf[i] += static_cast<float>(std::sin(ph2) * std::exp(-t * 20.0) * 0.45);
        }
    }

    {
        NoiseGenerator ng(1984u);
        SVFilter bp(SVFilter::Mode::BandPass, 3500.0, 1.8);
        const size_t click_samps = static_cast<size_t>(0.006 * sr);
        for (size_t i = 0; i < click_samps && i < buf.size(); ++i) {
            const double t   = static_cast<double>(i) / sr;
            const float  n   = ng.tick();
            const float  flt = bp.tick(n, sr);
            buf[i] += flt * static_cast<float>(std::exp(-t * 350.0)) * 1.2f;
        }
    }

    distortion::apply_waveshaper(buf, 0.85f);
    distortion::apply_hard_clip(buf, 0.82f);
    distortion::apply_soft_clip(buf, 3.2f);
    normalize(buf, 0.94f);
    return buf;
}

Buffer make_snare(int sr) {
    constexpr double DUR = 0.14;
    Buffer buf = make_buffer(DUR, sr);

    {
        Oscillator o1(WaveShape::Sine, 185.0, 0.55);
        Oscillator o2(WaveShape::Sine, 230.0, 0.30);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t   = static_cast<double>(i) / sr;
            const double env = std::exp(-t * 40.0);
            buf[i] = (o1.tick(sr) + o2.tick(sr)) * static_cast<float>(env);
        }
    }

    {
        NoiseGenerator ng(2112u);
        Buffer noise = ng.render(DUR, sr);
        HighPassFilter hpf(2800.0, sr);
        hpf.process(noise);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t   = static_cast<double>(i) / sr;
            const double env = std::exp(-t * 28.0);
            buf[i] += noise[i] * static_cast<float>(env) * 0.85f;
        }
    }

    {
        SVFilter bp(SVFilter::Mode::BandPass, 800.0, 3.5);
        bp.process(buf, sr);
    }

    distortion::apply_soft_clip(buf, 2.0f);
    normalize(buf, 0.82f);
    return buf;
}

Buffer make_hat_closed(int sr) {
    constexpr double DUR = 0.025;
    NoiseGenerator ng(13u);
    Buffer buf = ng.render(DUR, sr);
    HighPassFilter hpf(10000.0, sr);
    hpf.process(buf);
    SVFilter bp(SVFilter::Mode::BandPass, 14000.0, 0.8);
    bp.process(buf, sr);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 120.0));
    }
    normalize(buf, 0.32f);
    return buf;
}

Buffer make_hat_open(int sr) {
    constexpr double DUR = 0.18;
    NoiseGenerator ng(37u);
    Buffer buf = ng.render(DUR, sr);
    HighPassFilter hpf(8500.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 12.0));
    }
    Oscillator ping(WaveShape::Sine, 11000.0, 0.15);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] += ping.tick(sr) * static_cast<float>(std::exp(-t * 25.0));
    }
    normalize(buf, 0.45f);
    return buf;
}

Buffer make_clang(int sr) {
    constexpr double DUR = 0.09;
    Buffer buf = make_buffer(DUR, sr);
    Oscillator o1(WaveShape::Square, 420.0, 0.40);
    Oscillator o2(WaveShape::Square, 437.0, 0.35);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t   = static_cast<double>(i) / sr;
        const double env = std::exp(-t * 55.0);
        buf[i] = (o1.tick(sr) + o2.tick(sr)) * static_cast<float>(env);
    }
    SVFilter bp(SVFilter::Mode::BandPass, 2200.0, 4.0);
    bp.process(buf, sr);
    distortion::apply_hard_clip(buf, 0.45f);
    normalize(buf, 0.75f);
    return buf;
}

Buffer make_pulse_bass(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0) return Buffer();
    Buffer buf = make_buffer(dur, sr);

    Oscillator o1(WaveShape::Pulse, freq,        0.55); o1.duty = 0.40;
    Oscillator o2(WaveShape::Pulse, freq * 1.015, 0.35); o2.duty = 0.45;
    Oscillator sub(WaveShape::Sine,  freq * 0.5,  0.38);

    for (size_t i = 0; i < buf.size(); ++i)
        buf[i] = o1.tick(sr) + o2.tick(sr) + sub.tick(sr);

    std::vector<AutoPoint> fc = {
        {0.0,             3200.0f},
        {dur * 0.08,      1200.0f},
        {dur * 0.3,        500.0f},
        {dur,              180.0f}
    };
    buf = apply_filter_automation(buf, fc, 5.0, SVFilter::Mode::LowPass, sr);
    distortion::apply_waveshaper(buf, 0.78f);

    ADSR env(0.001, dur * 0.12, 0.72, dur * 0.15);
    env.apply(buf, dur, dur * 0.82, sr);
    normalize(buf, 0.85f);
    return buf;
}

Buffer make_fm_bass(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0) return Buffer();

    Buffer buf = make_buffer(dur, sr);
    const double c_inc = TWO_PI * freq        / sr;
    const double m_inc = TWO_PI * freq * 2.0  / sr;
    double cp = 0.0, mp = 0.0;

    for (size_t i = 0; i < buf.size(); ++i) {
        const double t     = static_cast<double>(i) / sr;
        const double index = 0.3 + 3.7 * std::exp(-t * 18.0);
        buf[i] = static_cast<float>(0.85 * std::sin(cp + index * std::sin(mp)));
        cp += c_inc; mp += m_inc;
        if (cp >= TWO_PI) cp -= TWO_PI;
        if (mp >= TWO_PI) mp -= TWO_PI;
    }

    {
        Oscillator sub(WaveShape::Sine, freq * 0.5, 0.35);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            buf[i] += sub.tick(sr) * static_cast<float>(std::exp(-t * 8.0));
        }
    }

    distortion::apply_soft_clip(buf, 2.5f);

    DAHDSR env(0.0, 0.001, 0.0, dur * 0.15, 0.68, dur * 0.18);
    env.apply(buf, dur, std::max(0.0, dur - dur * 0.18), sr);
    normalize(buf, 0.88f);
    return buf;
}

StereoBuffer make_pulse_lead(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0)
        return make_stereo(std::max(dur, 0.001), sr);
    Buffer mono = make_buffer(dur, sr);

    Oscillator o1(WaveShape::Pulse, freq,         0.55); o1.duty = 0.28;
    Oscillator o2(WaveShape::Pulse, freq * 1.008, 0.42); o2.duty = 0.32;
    Oscillator o3(WaveShape::Triangle, freq * 2.0, 0.18);

    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] = o1.tick(sr) + o2.tick(sr) + o3.tick(sr);

    std::vector<AutoPoint> fc = {
        {0.0,              400.0f},
        {dur * 0.05,      5000.0f},
        {dur * 0.4,       3200.0f},
        {dur,             1800.0f}
    };
    mono = apply_filter_automation(mono, fc, 2.8, SVFilter::Mode::LowPass, sr);
    distortion::apply_soft_clip(mono, 1.3f);

    const double rl = std::max(0.003, dur * 0.18);
    DAHDSR env(0.0, 0.001, 0.0, std::max(0.003, dur * 0.12), 0.65, rl);
    env.apply(mono, dur, std::max(0.0, dur - rl), sr);

    return haas_widen(mono, 8.0, sr);
}

StereoBuffer make_acid(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0)
        return make_stereo(std::max(dur, 0.001), sr);
    Buffer mono = make_buffer(dur, sr);

    Oscillator osc(WaveShape::Triangle, freq, 0.90);
    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] = osc.tick(sr);

    std::vector<AutoPoint> fc = {
        {0.0,              120.0f},
        {dur * 0.2,       7500.0f},
        {dur * 0.5,       4000.0f},
        {dur,              800.0f}
    };
    mono = apply_filter_automation(mono, fc, 8.5, SVFilter::Mode::LowPass, sr);
    distortion::apply_hard_clip(mono, 0.55f);

    const double rl = std::max(0.003, dur * 0.20);
    ADSR env(0.001, dur * 0.10, 0.60, rl);
    env.apply(mono, dur, std::max(0.0, dur - rl), sr);

    return haas_widen(mono, 18.0, sr);
}

Buffer make_stab(const std::vector<double>& freqs, double dur, int sr) {
    if (dur <= 0.0 || freqs.empty()) return Buffer();
    Buffer buf = make_buffer(dur, sr);
    for (double f : freqs) {
        Oscillator o(WaveShape::Sawtooth, f, 0.38);
        for (size_t i = 0; i < buf.size(); ++i) buf[i] += o.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 4500.0, 1.8);
    lp.process(buf, sr);
    distortion::apply_hard_clip(buf, 0.42f);
    DAHDSR env(0.0, 0.001, 0.0, 0.02, 0.0, std::max(0.002, dur - 0.022));
    env.apply(buf, dur, 0.022, sr);
    normalize(buf, 0.80f);
    return buf;
}

StereoBuffer make_pad(const std::vector<double>& freqs, double dur, int sr) {
    if (dur <= 0.0 || freqs.empty()) return make_stereo(dur, sr);
    Buffer mono = make_buffer(dur, sr);
    for (double f : freqs) {
        Oscillator o1(WaveShape::Sawtooth, f,        0.18);
        Oscillator o2(WaveShape::Triangle, f * 2.01, 0.09);
        Oscillator o3(WaveShape::Sine,     f * 0.5,  0.12);
        for (size_t i = 0; i < mono.size(); ++i)
            mono[i] += o1.tick(sr) + o2.tick(sr) + o3.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 1200.0, 0.75);
    lp.process(mono, sr);

    const double att = std::min(0.5, dur * 0.30);
    const double rel = std::min(0.4, dur * 0.25);
    DAHDSR env(0.0, att, 0.0, std::min(0.3, dur * 0.22), 0.78, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);

    StereoBuffer out = haas_widen(mono, 25.0, sr);
    Reverb rv;
    rv.room_size = 0.80f; rv.damping = 0.55f; rv.wet = 0.35f; rv.dry = 0.78f;
    rv.init(sr);
    rv.process(out.L);
    rv.room_size = 0.85f;
    rv.process(out.R);
    return out;
}

StereoBuffer make_fm_drone(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0) return make_stereo(dur, sr);
    FMSynth fm;
    fm.carrier_freq   = freq;
    fm.modulator_freq = freq * 1.5;
    fm.mod_index      = 1.8;
    fm.amplitude      = 0.55;
    Buffer mono = fm.render(dur, sr);

    Oscillator sub(WaveShape::Sine, freq * 0.5, 0.30);
    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] += sub.tick(sr);

    SVFilter lp(SVFilter::Mode::LowPass, 600.0, 0.9);
    lp.process(mono, sr);

    fade_in(mono, dur * 0.20, sr);
    fade_out(mono, dur * 0.20, sr);

    normalize(mono, 0.55f);
    return haas_widen(mono, 30.0, sr);
}

StereoBuffer make_riser(double dur, int sr) {
    NoiseGenerator ng(999u);
    Buffer buf = ng.render(dur, sr);

    std::vector<AutoPoint> fc = { {0.0, 150.0f}, {dur, 14000.0f} };
    buf = apply_filter_automation(buf, fc, 1.4, SVFilter::Mode::BandPass, sr);

    std::vector<AutoPoint> vol = { {0.0, 0.0f}, {dur * 0.5, 0.2f}, {dur, 1.0f} };
    apply_volume_automation(buf, vol, sr);

    Oscillator osc(WaveShape::Sawtooth, 40.0, 0.20);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        osc.frequency = 40.0 + 2000.0 * (t / dur);
        buf[i] += osc.tick(sr) * static_cast<float>(t / dur) * 0.5f;
    }

    clamp_buffer(buf);
    return haas_widen(buf, 18.0, sr);
}

StereoBuffer make_impact(double dur, int sr) {
    NoiseGenerator ng(5150u);
    Buffer buf = ng.render(dur, sr);
    HighPassFilter hpf(800.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 1.8));
    }
    Reverb rv;
    rv.room_size = 0.95f; rv.damping = 0.30f; rv.wet = 0.60f; rv.dry = 0.55f;
    rv.init(sr);
    rv.process(buf);
    distortion::apply_soft_clip(buf, 1.5f);
    normalize(buf, 0.90f);
    return haas_widen(buf, 32.0, sr);
}

struct BarSpec {
    bool kick_on        = false;
    bool kick_beat      = false;
    bool kick_8th       = false;
    bool kick_16th      = false;
    bool kick_syncopated = false;

    bool snare_on       = false;
    bool snare_roll     = false;
    bool hat_c_on       = false;
    bool hat_c_roll     = false;
    bool hat_o_beat     = false;
    bool clang_on       = false;

    double bass_freq    = 0.0;
    bool   bass_fm      = false;
    int    bass_div     = 0;
    float  bass_gain    = 0.40f;

    std::vector<double> stab_freqs;
    std::vector<int>    stab_steps;
    float stab_gain     = 0.30f;

    std::vector<double> pad_freqs;
    float pad_gain      = 0.0f;

    double drone_freq   = 0.0;
    float  drone_gain   = 0.0f;

    std::vector<std::pair<double,double>> lead;
    float lead_gain     = 0.50f;

    std::vector<std::pair<double,double>> acid;
    float acid_gain     = 0.28f;
};

Buffer render_kick_track(const BarSpec& spec, size_t bar_len,
                          const Buffer& kick, int sr)
{
    Buffer out(bar_len, 0.0f);
    if (!spec.kick_on) return out;

    std::vector<int> steps;

    if      (spec.kick_16th)       { for (int s = 0; s < 16; ++s) steps.push_back(s); }
    else if (spec.kick_syncopated) { steps = {0, 1, 3, 4, 6, 8, 9, 11, 12, 14}; }
    else if (spec.kick_8th)        { for (int s = 0; s < 16; s += 2) steps.push_back(s); }
    else if (spec.kick_beat)       { steps = {0, 4, 8, 12}; }
    else                            { steps = {0, 2, 4, 6, 8, 10, 12, 14}; }

    for (int s : steps) {
        const size_t off = static_cast<size_t>(s * STEP * sr + 0.5);
        for (size_t i = 0; i < kick.size() && off + i < out.size(); ++i)
            out[off + i] += kick[i];
    }
    return out;
}

StereoBuffer render_rest_track(const BarSpec& spec, size_t bar_len,
                                const Buffer& snare,
                                const Buffer& hat_c,
                                const Buffer& hat_o,
                                const Buffer& clang,
                                int sr)
{
    StereoBuffer out(bar_len, 0.0f);

    auto place_mono = [&](const Buffer& src, int step, float gain) {
        const size_t off = static_cast<size_t>(step * STEP * sr + 0.5);
        for (size_t i = 0; i < src.size() && off + i < out.size(); ++i) {
            out.L[off + i] += src[i] * gain;
            out.R[off + i] += src[i] * gain;
        }
    };

    auto place_stereo = [&](const StereoBuffer& src, double time_sec, float gain) {
        const size_t off = static_cast<size_t>(time_sec * sr + 0.5);
        for (size_t i = 0; i < src.size() && off + i < out.size(); ++i) {
            out.L[off + i] += src.L[i] * gain;
            out.R[off + i] += src.R[i] * gain;
        }
    };

    if (spec.snare_roll) {
        for (int s = 0; s < 16; ++s)
            place_mono(snare, s, (s % 4 == 0) ? 0.70f : 0.38f);
    } else if (spec.snare_on) {
        place_mono(snare, 4,  0.72f);
        place_mono(snare, 12, 0.72f);
    }

    if (spec.hat_c_roll) {
        for (int s = 0; s < 16; ++s)
            place_mono(hat_c, s, (s % 2 == 0) ? 0.30f : 0.20f);
    } else if (spec.hat_c_on) {
        for (int s = 0; s < 16; s += 2)
            place_mono(hat_c, s, 0.28f);
    }

    if (spec.hat_o_beat) {
        place_mono(hat_o, 9, 0.40f);
    }

    if (spec.clang_on) {
        place_mono(clang, 2,  0.38f);
        place_mono(clang, 10, 0.38f);
        if (spec.snare_on) place_mono(clang, 6, 0.22f);
    }

    if (!spec.stab_freqs.empty() && !spec.stab_steps.empty()) {
        for (int s : spec.stab_steps) {
            const double dur = STEP * 1.5;
            Buffer stab_buf  = make_stab(spec.stab_freqs, dur, sr);
            place_mono(stab_buf, s, spec.stab_gain);
        }
    }

    if (spec.bass_freq > 0.0 && spec.bass_div > 0) {
        const int slots = 16 / spec.bass_div;
        const double note_dur = STEP * static_cast<double>(spec.bass_div) * 0.88;

        for (int h = 0; h < slots; ++h) {
            const double freq = (spec.bass_div >= 8 && (h % 4 == 2))
                ? spec.bass_freq * 1.5
                : spec.bass_freq;

            const Buffer bass_buf = spec.bass_fm
                ? make_fm_bass(freq, note_dur, sr)
                : make_pulse_bass(freq, note_dur, sr);

            const int step = h * spec.bass_div;
            place_mono(bass_buf, step, spec.bass_gain);
        }
    }

    if (!spec.pad_freqs.empty() && spec.pad_gain > 0.0f) {
        StereoBuffer pad = make_pad(spec.pad_freqs,
                                    static_cast<double>(bar_len) / sr, sr);
        mix_stereo(out, pad, spec.pad_gain);
    }

    if (spec.drone_freq > 0.0 && spec.drone_gain > 0.0f) {
        StereoBuffer drone = make_fm_drone(spec.drone_freq,
                                            static_cast<double>(bar_len) / sr, sr);
        mix_stereo(out, drone, spec.drone_gain);
    }

    if (!spec.lead.empty()) {
        double t = 0.0;
        for (auto& [freq, dur_8ths] : spec.lead) {
            const double dur = dur_8ths * STEP8;
            if (freq > 0.0 && dur > 0.0) {
                StereoBuffer note = make_pulse_lead(freq, dur, sr);
                place_stereo(note, t, spec.lead_gain);
            }
            t += dur;
        }
    }

    if (!spec.acid.empty()) {
        double t = 0.0;
        for (auto& [freq, dur_8ths] : spec.acid) {
            const double dur = dur_8ths * STEP8;
            if (freq > 0.0 && dur > 0.0) {
                StereoBuffer note = make_acid(freq, dur, sr);
                place_stereo(note, t, spec.acid_gain);
            }
            t += dur;
        }
    }

    return out;
}

std::vector<BarSpec> build_song() {
    std::vector<BarSpec> song;
    song.reserve(360);

    auto A = [](int i) -> const HarmonicArea& { return AREAS[i % 4]; };

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        if (i >= 3)  b.hat_c_on  = true;
        if (i >= 6)  b.clang_on  = true;
        if (i >= 10) {
            b.kick_on   = true;
            b.kick_beat = true;
        }
        if (i >= 12) {
            const HarmonicArea& ar = A(i);
            b.pad_freqs = ar.pad;
            b.pad_gain  = 0.05f + 0.04f * (i - 12);
        }
        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.kick_on  = true;
        b.kick_beat = (i < 8);
        b.kick_8th = (i >= 8 && i < 24);
        b.kick_syncopated = (i >= 24);

        b.hat_c_on   = true;
        b.hat_c_roll = (i >= 24);
        b.hat_o_beat = (i >= 8);
        b.clang_on   = true;
        b.snare_on   = (i >= 4);

        b.bass_freq = ar.root;
        b.bass_fm   = false;
        b.bass_div  = (i < 16) ? 4 : 8;
        b.bass_gain = 0.30f + 0.004f * i;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.10f + 0.005f * i;

        if (i >= 16) {
            b.stab_freqs = ar.stab;
            b.stab_steps = {6, 14};
            b.stab_gain  = 0.20f + 0.005f * (i - 16);
        }

        if (i >= 24) {
            b.lead = { {ar.mel_a[0], 4.0}, {ar.mel_a[2], 4.0} };
            b.lead_gain = 0.22f + 0.03f * (i - 24);
        }

        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on = true;
        b.kick_16th       = (loop % 2 == 0);
        b.kick_syncopated = (loop % 2 == 1);
        if (i % 8 == 7) { b.kick_16th = false; b.kick_syncopated = false; b.kick_8th = true; }

        b.snare_on   = true;
        b.snare_roll = (i % 8 == 7 && loop >= 4);
        b.hat_c_on   = true;
        b.hat_c_roll = (i % 4 == 3);
        b.hat_o_beat = (loop % 2 == 0);
        b.clang_on   = true;

        b.bass_freq = ar.root;
        b.bass_fm   = false;
        b.bass_div  = (i % 4 == 3) ? 16 : 8;
        b.bass_gain = 0.38f + 0.003f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.14f + 0.004f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = (loop % 2 == 0) ? std::vector<int>{2, 10} : std::vector<int>{4, 12, 14};
        b.stab_gain  = 0.26f + 0.004f * loop;

        const auto& mel = (loop % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.46f + 0.004f * loop;

        if (loop >= 4) {
            const auto& acid_mel = (loop % 2 == 0) ? ar.mel_b : ar.mel_a;
            for (int n = 0; n < 4; ++n)
                b.acid.push_back({acid_mel[n], 2.0});
            b.acid_gain = 0.14f + 0.006f * (loop - 4);
        }

        song.push_back(b);
    }

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.bass_freq = ar.root;
        b.bass_fm   = false;
        b.bass_div  = 4;
        b.bass_gain = 0.36f;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.50f - 0.02f * i;

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = 0.10f + 0.02f * i;

        if (i == 0 || i == 8) { b.kick_on = true; b.kick_beat = true; }
        if (i % 4 == 0) b.clang_on = true;

        b.lead = { {ar.mel_b[0], 8.0} };
        b.lead_gain = 0.20f + 0.02f * i;

        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on = (i >= 2);
        if (i >= 2) {
            b.kick_syncopated = (loop % 2 == 0);
            b.kick_16th       = (loop % 2 == 1);
            if (i % 8 == 7 && loop >= 4) { b.kick_syncopated = false; b.kick_16th = true; }
        }

        b.snare_on   = (i >= 4);
        b.snare_roll = (i % 8 == 7 && loop >= 5);
        b.hat_c_on   = (i >= 1);
        b.hat_c_roll = (i % 4 == 3 && loop >= 2);
        b.hat_o_beat = (loop % 2 == 1);
        b.clang_on   = (i >= 3);

        b.bass_freq = ar.root;
        b.bass_fm   = true;
        b.bass_div  = (i % 4 == 3 && loop >= 4) ? 16 : 8;
        b.bass_gain = 0.36f + 0.004f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.10f + 0.004f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = (loop % 2 == 0) ? std::vector<int>{1, 9, 13} : std::vector<int>{3, 7, 15};
        b.stab_gain  = 0.28f + 0.004f * loop;

        const auto& mel = (loop % 2 == 0) ? ar.mel_b : ar.mel_a;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.40f + 0.005f * loop;

        const auto& acid_mel = (loop % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (int n = 0; n < 4; ++n)
            b.acid.push_back({acid_mel[(n * 2) % 8], 2.0});
        b.acid_gain = 0.22f + 0.005f * loop;

        song.push_back(b);
    }

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i + 1);

        b.kick_on   = true;
        b.kick_beat = true;

        b.hat_c_on   = (i % 2 == 0);
        b.hat_o_beat = (i % 4 == 2);
        if (i % 2 == 1) b.snare_on = true;
        if (i % 4 == 0) b.clang_on = true;

        b.bass_freq = ar.root;
        b.bass_fm   = (i >= 8);
        b.bass_div  = 4;
        b.bass_gain = 0.34f + 0.01f * i;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.22f + 0.012f * i;

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = 0.08f + 0.012f * i;

        b.lead = { {ar.mel_a[0], 8.0} };
        b.lead_gain = 0.30f + 0.01f * i;

        if (i >= 12) {
            b.acid = { {ar.mel_b[2], 4.0}, {ar.mel_b[6], 4.0} };
            b.acid_gain = 0.16f + 0.04f * (i - 12);
        }

        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on  = true;
        b.kick_8th = true;
        if (i % 8 == 7) { b.kick_8th = false; b.kick_16th = true; }

        b.snare_on   = true;
        b.hat_c_on   = true;
        b.hat_c_roll = (i % 8 == 7);
        b.hat_o_beat = (loop % 2 == 0);
        b.clang_on   = (loop % 2 == 1);

        b.bass_freq = ar.root;
        b.bass_fm   = false;
        b.bass_div  = 8;
        b.bass_gain = 0.36f + 0.003f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.18f + 0.003f * loop;

        if (i % 4 == 2) {
            b.stab_freqs = ar.stab;
            b.stab_steps = {8};
            b.stab_gain  = 0.24f;
        }

        const auto& mel = (i % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.50f + 0.003f * loop;

        if (loop >= 6) {
            const auto& acid_mel = (i % 2 == 0) ? ar.mel_b : ar.mel_a;
            for (int n = 0; n < 4; ++n)
                b.acid.push_back({acid_mel[n * 2], 2.0});
            b.acid_gain = 0.16f + 0.01f * (loop - 6);
        }

        song.push_back(b);
    }

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.kick_on         = true;
        b.kick_syncopated = (i < 8);
        b.kick_16th       = (i >= 8);

        b.snare_on   = true;
        b.snare_roll = (i >= 12);
        b.hat_c_roll = (i >= 4);
        b.hat_o_beat = true;
        b.clang_on   = true;

        b.bass_freq = ar.root;
        b.bass_fm   = (i >= 8);
        b.bass_div  = (i < 8) ? 8 : 16;
        b.bass_gain = 0.38f + 0.006f * i;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.16f + 0.006f * i;

        b.stab_freqs = ar.stab;
        b.stab_steps = {0, 4, 8, 12};
        b.stab_gain  = 0.26f + 0.006f * i;

        const auto& mel = (i % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.46f + 0.006f * i;

        const auto& acid_mel = (i % 2 == 0) ? ar.mel_b : ar.mel_a;
        for (int n = 0; n < 4; ++n)
            b.acid.push_back({acid_mel[n], 2.0});
        b.acid_gain = 0.18f + 0.008f * i;

        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on         = true;
        b.kick_16th       = (loop % 2 == 0);
        b.kick_syncopated = (loop % 2 == 1);
        if (!b.kick_16th && !b.kick_syncopated) b.kick_8th = true;

        b.snare_on   = true;
        b.snare_roll = (i % 4 == 3);
        b.hat_c_roll = true;
        b.hat_o_beat = (loop % 2 == 0);
        b.clang_on   = true;

        b.bass_freq = ar.root;
        b.bass_fm   = (loop % 2 == 1);
        b.bass_div  = 16;
        b.bass_gain = 0.36f + 0.003f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.16f + 0.003f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = {0, 3, 6, 9, 12};
        b.stab_gain  = 0.26f + 0.003f * loop;

        const auto& mel = (i % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.52f + 0.003f * loop;

        const auto& acid_mel = (i % 2 == 0) ? ar.mel_b : ar.mel_a;
        for (double f : acid_mel) b.acid.push_back({f, 1.0});
        b.acid_gain = 0.28f + 0.003f * loop;

        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i + 1);
        const int loop = i / 4;

        b.kick_on         = true;
        b.kick_16th       = (loop % 2 == 1);
        b.kick_syncopated = (loop % 2 == 0);
        if (!b.kick_16th && !b.kick_syncopated) b.kick_8th = true;

        b.snare_on   = true;
        b.snare_roll = (i % 4 == 3);
        b.hat_c_roll = true;
        b.hat_o_beat = (loop % 2 == 1);
        b.clang_on   = true;

        b.bass_freq = ar.root;
        b.bass_fm   = (loop % 2 == 0);
        b.bass_div  = 16;
        b.bass_gain = 0.37f + 0.003f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.17f + 0.003f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = {1, 3, 5, 8, 11, 13};
        b.stab_gain  = 0.27f + 0.003f * loop;

        const auto& mel = (i % 2 == 1) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.53f + 0.003f * loop;

        const auto& acid_mel = (i % 2 == 1) ? ar.mel_b : ar.mel_a;
        for (double f : acid_mel) b.acid.push_back({f, 1.0});
        b.acid_gain = 0.29f + 0.003f * loop;

        song.push_back(b);
    }

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.45f - 0.02f * i;

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = 0.30f + 0.01f * i;

        if (i % 4 == 0) b.clang_on = true;

        b.lead = { {ar.mel_a[0], 8.0} };
        b.lead_gain = 0.16f + 0.01f * i;

        if (i == 15) { b.kick_on = true; b.kick_beat = true; }

        song.push_back(b);
    }

    for (int i = 0; i < 40; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on   = true;
        b.kick_16th = true;

        b.snare_on   = true;
        b.snare_roll = (i % 4 == 3);
        b.hat_c_roll = true;
        b.hat_o_beat = true;
        b.clang_on   = true;

        b.bass_freq = ar.root;
        b.bass_fm   = (loop % 2 == 0);
        b.bass_div  = 16;
        b.bass_gain = 0.40f + 0.002f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.18f + 0.002f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = {0, 2, 4, 6, 8, 10, 12, 14};
        b.stab_gain  = 0.24f + 0.003f * loop;

        const auto& mel = (i % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.lead.push_back({f, 1.0});
        b.lead_gain = 0.55f + 0.002f * loop;

        const auto& acid_mel = (i % 2 == 0) ? ar.mel_b : ar.mel_a;
        for (double f : acid_mel) b.acid.push_back({f, 1.0});
        b.acid_gain = 0.30f + 0.002f * loop;

        song.push_back(b);
    }
    
    for (int i = 0; i < 48; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.kick_on         = (i < 40);
        b.kick_16th       = (i < 8);
        b.kick_syncopated = (i >= 8  && i < 18);
        b.kick_8th        = (i >= 18 && i < 28);
        b.kick_beat       = (i >= 28 && i < 40);

        b.snare_on   = (i < 28);
        b.hat_c_roll = (i < 8);
        b.hat_c_on   = (i >= 8  && i < 24);
        b.hat_o_beat = (i >= 8  && i < 18);
        b.clang_on   = (i < 14);

        b.bass_freq = ar.root;
        b.bass_fm   = false;
        if      (i < 14) { b.bass_div = 16; b.bass_gain = 0.40f; }
        else if (i < 24) { b.bass_div = 8;  b.bass_gain = 0.38f - 0.015f * (i - 14); }
        else if (i < 36) { b.bass_div = 4;  b.bass_gain = 0.26f - 0.012f * (i - 24); }

        b.pad_freqs = ar.pad;
        b.pad_gain  = std::min(0.55f, 0.035f * i);

        if (i < 10) {
            const auto& mel = (i % 2 == 0) ? ar.mel_a : ar.mel_b;
            for (double f : mel) b.lead.push_back({f, 1.0});
            b.lead_gain = 0.50f - 0.03f * i;
        } else if (i < 22) {
            b.lead = { {ar.mel_a[0], 4.0}, {ar.mel_a[4], 4.0} };
            b.lead_gain = 0.24f - 0.015f * (i - 10);
        } else if (i < 34) {
            b.lead = { {ar.mel_a[0], 8.0} };
            b.lead_gain = 0.18f - 0.012f * (i - 22);
        }
        if (i < 6) {
            for (int n = 0; n < 4; ++n)
                b.acid.push_back({ar.mel_b[n * 2], 2.0});
            b.acid_gain = 0.26f - 0.04f * i;
        }
        if (i >= 24) {
            b.drone_freq = ar.root * 2.0;
            b.drone_gain = std::min(0.50f, 0.02f * (i - 24));
        }

        song.push_back(b);
    }

    for (size_t bi = 0; bi < song.size(); ++bi) {
        auto& b = song[bi];

        b.kick_8th        = false;
        b.kick_16th       = false;
        b.kick_syncopated = false;
        b.snare_roll      = false;
        b.hat_c_roll      = false;

        if (b.kick_on) {
            const int cyc = static_cast<int>(bi) % 4;
            if (cyc == 3) {
                b.kick_on = false;
            } else {
                b.kick_beat = true;
            }
        }

        b.bass_gain  *= 0.65f;
        b.stab_gain  *= 0.60f;
        if (b.clang_on && bi % 2 == 1) b.clang_on = false;
        if (!b.lead.empty()) b.lead_gain = 0.65f;
    }

    return song;
}

void print_help(const char* prog) {
    std::cout <<
        "Deserves Treatment\n"
        "Usage:\n"
        "  " << prog << " [options]\n\n"
        "Options:\n"
        "  -o, --output <file>   Output WAV (default: deserves_treatment.wav)\n"
        "      --play            Play after rendering (press Enter to stop)\n"
        "  -h, --help            Show this help\n";
}

int main(int argc, char** argv) {
    std::string output  = "deserves_treatment.wav";
    bool        do_play = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-o" || a == "--output") {
            if (i + 1 < argc) output = argv[++i];
            else { std::cerr << "Missing argument for " << a << "\n"; return 1; }
        } else if (a == "--play") {
            do_play = true;
        } else if (a == "-h" || a == "--help") {
            print_help(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << a << "\n";
            print_help(argv[0]);
            return 1;
        }
    }

    global_config().sample_rate = SR;
    global_config().channels    = 2;
    global_config().bits        = 16;

    std::cout << "\n  Deserves Treatment\n";
    std::cout << "  Tempo : " << BPM << " BPM\n";
    std::cout << "  Key   : C Major\n";

    const std::vector<BarSpec> song = build_song();
    const size_t bar_len = static_cast<size_t>(BAR * SR + 0.5);
    const size_t total   = bar_len * song.size();

    std::cout << "  Bars  : " << song.size() << "\n";
    std::cout << "  Length: " << std::fixed << std::setprecision(1)
              << static_cast<double>(total) / SR << " s\n\n";

    const Buffer kick  = make_kick(SR);
    const Buffer snare = make_snare(SR);
    const Buffer hat_c = make_hat_closed(SR);
    const Buffer hat_o = make_hat_open(SR);
    const Buffer clang = make_clang(SR);

    StereoBuffer kick_track(total, 0.0f);
    StereoBuffer rest_track(total, 0.0f);

    for (size_t i = 0; i < song.size(); ++i) {
        Buffer       kb = render_kick_track(song[i], bar_len, kick, SR);
        StereoBuffer rb = render_rest_track(song[i], bar_len,
                                             snare, hat_c, hat_o, clang, SR);
        const size_t off = i * bar_len;
        for (size_t s = 0; s < bar_len; ++s) {
            kick_track.L[off + s] = kb[s];
            kick_track.R[off + s] = kb[s];
            rest_track.L[off + s] = rb.L[s];
            rest_track.R[off + s] = rb.R[s];
        }
        std::cout << "  Rendering bar " << (i + 1) << " / " << song.size() << "\r" << std::flush;
    }
    std::cout << "\n";

    auto mix_stereo_at = [&](StereoBuffer& dst, const StereoBuffer& src,
                              size_t bar_off, float gain) {
        const size_t off = bar_off * bar_len;
        for (size_t i = 0; i < src.size() && off + i < dst.size(); ++i) {
            dst.L[off + i] += src.L[i] * gain;
            dst.R[off + i] += src.R[i] * gain;
        }
    };

    { auto r = make_riser(4.0 * BAR, SR); mix_stereo_at(rest_track, r, 20, 0.50f); }
    { auto m = make_impact(2.0 * BAR, SR); mix_stereo_at(rest_track, m, 22, 0.60f); }

    { auto r = make_riser(4.0 * BAR, SR); mix_stereo_at(rest_track, r, 60, 0.60f); }
    { auto m = make_impact(2.0 * BAR, SR); mix_stereo_at(rest_track, m, 62, 0.72f); }

    { auto r = make_riser(4.0 * BAR, SR); mix_stereo_at(rest_track, r, 100, 0.72f); }
    { auto m = make_impact(2.0 * BAR, SR); mix_stereo_at(rest_track, m, 102, 0.86f); }

    SidechainCompressor sc;
    sc.attack_ms  = 1.5f;
    sc.release_ms = 90.0f;
    sc.strength   = 0.06f;
    sc.threshold  = 0.018f;
    Buffer kick_mono = kick_track.to_mono();
    sc.apply(rest_track, kick_mono, SR);

    StereoBuffer mix(total, 0.0f);
    mix_into(mix, kick_track, 0.28f);
    mix_into(mix, rest_track, 1.0f);

    distortion::apply_soft_clip(mix.L, 1.35f);
    distortion::apply_soft_clip(mix.R, 1.35f);
    fade_in (mix, 0.08, SR);
    fade_out(mix, 3.0,  SR);
    normalize(mix, 0.92f);
    clamp_buffer(mix);

    std::cout << "Writing WAV: " << output << "\n";
    save_wav_stereo(mix, output, SR);
    std::cout << "Done.\n";

    if (do_play) {
        std::cout << "Playing (press Enter to stop early)...\n";
        PlaybackHandle handle = play_async(mix, SR);
        std::thread waiter([handle]() mutable {
            std::cin.get();
            handle.stop();
        });
        waiter.detach();
        handle.wait();
        std::cout << "Playback finished.\n";
    }

    return 0;
}