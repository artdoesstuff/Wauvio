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

constexpr int    SR   = 44100;
constexpr double BPM  = 200.0;
constexpr double BAR  = 60.0 / BPM * 4.0;
constexpr double BEAT = BAR / 4.0;
constexpr double STEP = BAR / 16.0;
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
        { notes::E3, notes::G3, notes::B3 },
        notes::E2,
        { notes::E5, notes::B4, notes::G5, notes::E5,
          notes::D5, notes::B4, notes::G4, notes::B4 },
        { notes::B5, notes::G5, notes::E5, notes::D5,
          notes::B4, notes::G4, notes::E4, notes::D4 },
        { notes::E3, notes::B3 }
    },
    {
        { notes::C4, notes::Ds4, notes::G4 },
        notes::C2,
        { notes::C5, notes::G4, notes::Ds5, notes::C5,
          notes::As4, notes::G4, notes::Ds4, notes::G4 },
        { notes::Ds5, notes::C5, notes::G5, notes::Ds5,
          notes::C5, notes::As4, notes::G4, notes::As4 },
        { notes::C4, notes::G4 }
    },
    {
        { notes::A3, notes::C4, notes::E4 },
        notes::A1,
        { notes::A4, notes::E4, notes::C5, notes::A4,
          notes::G4, notes::E4, notes::C4, notes::E4 },
        { notes::C5, notes::A4, notes::E5, notes::C5,
          notes::A4, notes::G4, notes::E4, notes::G4 },
        { notes::A3, notes::E4 }
    },
    {
        { notes::B3, notes::D4, notes::Fs4 },
        notes::B1,
        { notes::B4, notes::Fs4, notes::D5, notes::B4,
          notes::A4, notes::Fs4, notes::D4, notes::Fs4 },
        { notes::D5, notes::B4, notes::Fs5, notes::D5,
          notes::B4, notes::A4, notes::Fs4, notes::A4 },
        { notes::B3, notes::Fs4 }
    },
};

Buffer make_kick(int sr) {
    constexpr double DUR = 0.22;
    Buffer buf = make_buffer(DUR, sr);

    double phase = 0.0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        const double f = 42.0 + 200.0 * std::exp(-t * 50.0);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        buf[i] = static_cast<float>(std::sin(phase) * std::exp(-t * 14.0));
    }
    {
        double ph2 = 0.0;
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            const double f = 28.0 + 80.0 * std::exp(-t * 80.0);
            ph2 += TWO_PI * f / sr;
            if (ph2 >= TWO_PI) ph2 -= TWO_PI;
            buf[i] += static_cast<float>(std::sin(ph2) * std::exp(-t * 22.0) * 0.50);
        }
    }
    {
        NoiseGenerator ng(666u);
        SVFilter bp(SVFilter::Mode::BandPass, 4200.0, 2.0);
        const size_t click_n = static_cast<size_t>(0.007 * sr);
        for (size_t i = 0; i < click_n && i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            buf[i] += bp.tick(ng.tick(), sr) * static_cast<float>(std::exp(-t * 400.0)) * 1.4f;
        }
    }
    distortion::apply_waveshaper(buf, 0.90f);
    distortion::apply_hard_clip(buf, 0.84f);
    distortion::apply_soft_clip(buf, 3.8f);
    normalize(buf, 0.96f);
    return buf;
}

Buffer make_snare(int sr) {
    constexpr double DUR = 0.18;
    Buffer buf = make_buffer(DUR, sr);
    {
        Oscillator o1(WaveShape::Sine, 160.0, 0.60);
        Oscillator o2(WaveShape::Sine, 200.0, 0.38);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            buf[i] = (o1.tick(sr) + o2.tick(sr)) * static_cast<float>(std::exp(-t * 35.0));
        }
    }
    {
        NoiseGenerator ng(1312u);
        Buffer noise = ng.render(DUR, sr);
        HighPassFilter hpf(2200.0, sr);
        hpf.process(noise);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            buf[i] += noise[i] * static_cast<float>(std::exp(-t * 22.0)) * 0.90f;
        }
    }
    distortion::apply_soft_clip(buf, 2.2f);
    normalize(buf, 0.85f);
    return buf;
}

Buffer make_hat_closed(int sr) {
    constexpr double DUR = 0.030;
    NoiseGenerator ng(23u);
    Buffer buf = ng.render(DUR, sr);
    HighPassFilter hpf(9500.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 100.0));
    }
    normalize(buf, 0.28f);
    return buf;
}

Buffer make_hat_open(int sr) {
    constexpr double DUR = 0.20;
    NoiseGenerator ng(51u);
    Buffer buf = ng.render(DUR, sr);
    HighPassFilter hpf(7500.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 9.0));
    }
    normalize(buf, 0.42f);
    return buf;
}

Buffer make_clang(int sr) {
    constexpr double DUR = 0.40;
    Buffer buf = make_buffer(DUR, sr);
    const double partials[][2] = {
        {320.0,  0.50},
        {512.0,  0.35},
        {735.0,  0.25},
        {1024.0, 0.18},
        {1460.0, 0.12},
    };
    for (auto& p : partials) {
        Oscillator o(WaveShape::Sine, p[0], p[1]);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            buf[i] += o.tick(sr) * static_cast<float>(std::exp(-t * 12.0));
        }
    }
    SVFilter bp(SVFilter::Mode::BandPass, 600.0, 3.5);
    bp.process(buf, sr);
    distortion::apply_hard_clip(buf, 0.50f);
    normalize(buf, 0.70f);
    Reverb rv;
    rv.room_size = 0.92f; rv.damping = 0.25f; rv.wet = 0.55f; rv.dry = 0.75f;
    rv.init(sr);
    rv.process(buf);
    normalize(buf, 0.65f);
    return buf;
}

Buffer make_fm_bass(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0) return Buffer();
    Buffer buf = make_buffer(dur, sr);

    const double c_inc = TWO_PI * freq       / sr;
    const double m_inc = TWO_PI * freq * 1.5 / sr;
    double cp = 0.0, mp = 0.0;

    for (size_t i = 0; i < buf.size(); ++i) {
        const double t     = static_cast<double>(i) / sr;
        const double index = 0.5 + 4.5 * std::exp(-t * 12.0);
        buf[i] = static_cast<float>(0.80 * std::sin(cp + index * std::sin(mp)));
        cp += c_inc; mp += m_inc;
        if (cp >= TWO_PI) cp -= TWO_PI;
        if (mp >= TWO_PI) mp -= TWO_PI;
    }
    {
        Oscillator sub(WaveShape::Sine, freq * 0.5, 0.42);
        for (size_t i = 0; i < buf.size(); ++i) {
            const double t = static_cast<double>(i) / sr;
            buf[i] += sub.tick(sr) * static_cast<float>(std::exp(-t * 6.0));
        }
    }
    std::vector<AutoPoint> fc = {
        {0.0,        2800.0f},
        {dur * 0.10, 900.0f},
        {dur * 0.40, 400.0f},
        {dur,        180.0f}
    };
    buf = apply_filter_automation(buf, fc, 5.5, SVFilter::Mode::LowPass, sr);
    distortion::apply_waveshaper(buf, 0.80f);
    distortion::apply_soft_clip(buf, 2.0f);

    DAHDSR env(0.0, 0.001, 0.0, dur * 0.12, 0.75, dur * 0.15);
    env.apply(buf, dur, std::max(0.0, dur - dur * 0.15), sr);
    normalize(buf, 0.88f);
    return buf;
}

StereoBuffer make_screech(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0)
        return make_stereo(std::max(dur, 0.001), sr);

    Supersaw saw;
    saw.frequency    = freq * 2.0;
    saw.voices       = 7;
    saw.detune_cents = 28.0;
    saw.amplitude    = 0.45;
    saw.mix_center   = 0.60;
    Buffer mono = saw.render(dur, sr);

    std::vector<AutoPoint> fc = {
        {0.0,            1800.0f},
        {dur * 0.30,     9000.0f},
        {dur * 0.65,     4500.0f},
        {dur,            1200.0f}
    };
    mono = apply_filter_automation(mono, fc, 10.0, SVFilter::Mode::BandPass, sr);
    distortion::apply_hard_clip(mono, 0.48f);

    const double rl = std::max(0.003, dur * 0.20);
    DAHDSR env(0.0, 0.001, 0.0, std::max(0.003, dur * 0.22), 0.60, rl);
    env.apply(mono, dur, std::max(0.0, dur - rl), sr);

    StereoBuffer out = haas_widen(mono, 18.0, sr);
    Reverb rvL; rvL.room_size = 0.85f; rvL.wet = 0.30f; rvL.dry = 0.82f; rvL.init(sr);
    Reverb rvR; rvR.room_size = 0.90f; rvR.wet = 0.30f; rvR.dry = 0.82f; rvR.init(sr);
    rvL.process(out.L); rvR.process(out.R);
    return out;
}

StereoBuffer make_gothic_pad(const std::vector<double>& freqs, double dur, int sr) {
    if (dur <= 0.0 || freqs.empty()) return make_stereo(dur, sr);
    Buffer mono = make_buffer(dur, sr);
    for (double f : freqs) {
        Oscillator o1(WaveShape::Sawtooth, f,         0.20);
        Oscillator o2(WaveShape::Triangle, f * 2.005, 0.10);
        Oscillator o3(WaveShape::Sine,     f * 0.5,   0.18);
        Oscillator o4(WaveShape::Square,   f * 3.01,  0.05);
        for (size_t i = 0; i < mono.size(); ++i)
            mono[i] += o1.tick(sr) + o2.tick(sr) + o3.tick(sr) + o4.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 1400.0, 0.80);
    lp.process(mono, sr);

    const double att = std::min(0.6, dur * 0.35);
    const double rel = std::min(0.5, dur * 0.28);
    DAHDSR env(0.0, att, 0.0, std::min(0.35, dur * 0.25), 0.80, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);

    StereoBuffer out = haas_widen(mono, 22.0, sr);
    Reverb rvL; rvL.room_size = 0.88f; rvL.damping = 0.40f; rvL.wet = 0.45f; rvL.dry = 0.78f; rvL.init(sr);
    Reverb rvR; rvR.room_size = 0.92f; rvR.damping = 0.42f; rvR.wet = 0.45f; rvR.dry = 0.78f; rvR.init(sr);
    rvL.process(out.L); rvR.process(out.R);
    return out;
}

StereoBuffer make_acid(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0)
        return make_stereo(std::max(dur, 0.001), sr);
    Buffer mono = make_buffer(dur, sr);

    Oscillator osc(WaveShape::Sawtooth, freq, 0.88);
    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] = osc.tick(sr);

    std::vector<AutoPoint> fc = {
        {0.0,         100.0f},
        {dur * 0.15, 6500.0f},
        {dur * 0.55, 3000.0f},
        {dur,         600.0f}
    };
    mono = apply_filter_automation(mono, fc, 9.0, SVFilter::Mode::LowPass, sr);
    distortion::apply_hard_clip(mono, 0.52f);

    const double rl = std::max(0.003, dur * 0.18);
    ADSR env(0.001, dur * 0.08, 0.62, rl);
    env.apply(mono, dur, std::max(0.0, dur - rl), sr);

    return haas_widen(mono, 14.0, sr);
}

Buffer make_stab(const std::vector<double>& freqs, double dur, int sr) {
    if (dur <= 0.0 || freqs.empty()) return Buffer();
    Buffer buf = make_buffer(dur, sr);
    for (double f : freqs) {
        Oscillator o(WaveShape::Sawtooth, f * 2.0, 0.40);
        for (size_t i = 0; i < buf.size(); ++i) buf[i] += o.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 5000.0, 2.0);
    lp.process(buf, sr);
    distortion::apply_hard_clip(buf, 0.44f);
    DAHDSR env(0.0, 0.001, 0.0, 0.02, 0.0, std::max(0.002, dur - 0.022));
    env.apply(buf, dur, 0.022, sr);
    normalize(buf, 0.82f);
    return buf;
}

StereoBuffer make_drone(double freq, double dur, int sr) {
    if (dur <= 0.0 || freq <= 0.0) return make_stereo(dur, sr);
    FMSynth fm;
    fm.carrier_freq   = freq;
    fm.modulator_freq = freq * 2.0;
    fm.mod_index      = 2.5;
    fm.amplitude      = 0.50;
    Buffer mono = fm.render(dur, sr);

    Oscillator sub(WaveShape::Sine, freq * 0.5, 0.35);
    for (size_t i = 0; i < mono.size(); ++i) mono[i] += sub.tick(sr);

    SVFilter lp(SVFilter::Mode::LowPass, 500.0, 0.85);
    lp.process(mono, sr);

    fade_in(mono, dur * 0.18, sr);
    fade_out(mono, dur * 0.18, sr);
    normalize(mono, 0.55f);
    return haas_widen(mono, 28.0, sr);
}

StereoBuffer make_riser(double dur, int sr) {
    NoiseGenerator ng(777u);
    Buffer buf = ng.render(dur, sr);
    std::vector<AutoPoint> fc = { {0.0, 120.0f}, {dur, 16000.0f} };
    buf = apply_filter_automation(buf, fc, 1.6, SVFilter::Mode::BandPass, sr);
    std::vector<AutoPoint> vol = { {0.0, 0.0f}, {dur * 0.5, 0.15f}, {dur, 1.0f} };
    apply_volume_automation(buf, vol, sr);
    Oscillator pitch_osc(WaveShape::Sawtooth, 35.0, 0.22);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        pitch_osc.frequency = 35.0 + 2200.0 * (t / dur);
        buf[i] += pitch_osc.tick(sr) * static_cast<float>(t / dur) * 0.55f;
    }
    clamp_buffer(buf);
    return haas_widen(buf, 20.0, sr);
}

StereoBuffer make_impact(double dur, int sr) {
    NoiseGenerator ng(3030u);
    Buffer buf = ng.render(dur, sr);
    HighPassFilter hpf(1200.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 2.0));
    }
    Reverb rv;
    rv.room_size = 0.96f; rv.damping = 0.20f; rv.wet = 0.65f; rv.dry = 0.60f;
    rv.init(sr);
    rv.process(buf);
    distortion::apply_soft_clip(buf, 1.6f);
    normalize(buf, 0.92f);
    return haas_widen(buf, 30.0, sr);
}

struct BarSpec {
    bool kick_on         = false;
    bool kick_beat       = false;
    bool kick_8th        = false;
    bool kick_16th       = false;
    bool kick_syncopated = false;

    bool snare_on    = false;
    bool snare_roll  = false;
    bool hat_c_on    = false;
    bool hat_c_roll  = false;
    bool hat_o_beat  = false;
    bool clang_on    = false;

    double bass_freq = 0.0;
    int    bass_div  = 0;
    float  bass_gain = 0.42f;

    std::vector<double> stab_freqs;
    std::vector<int>    stab_steps;
    float stab_gain  = 0.32f;

    std::vector<double> pad_freqs;
    float pad_gain   = 0.0f;

    double drone_freq = 0.0;
    float  drone_gain = 0.0f;

    std::vector<std::pair<double,double>> screech;
    float screech_gain = 0.50f;

    std::vector<std::pair<double,double>> acid;
    float acid_gain    = 0.28f;
};

Buffer render_kick(const BarSpec& spec, size_t bar_len,
                   const Buffer& kick, int sr)
{
    Buffer out(bar_len, 0.0f);
    if (!spec.kick_on) return out;

    std::vector<int> steps;
    if      (spec.kick_16th)       { for (int s = 0;  s < 16; ++s)      steps.push_back(s); }
    else if (spec.kick_syncopated) { steps = {0, 1, 3, 5, 6, 8, 9, 11, 13, 14}; }
    else if (spec.kick_8th)        { for (int s = 0;  s < 16; s += 2)   steps.push_back(s); }
    else if (spec.kick_beat)       { steps = {0, 4, 8, 12}; }
    else                            { steps = {0, 2, 4, 6, 8, 10, 12, 14}; }

    for (int s : steps) {
        const size_t off = static_cast<size_t>(s * STEP * sr + 0.5);
        for (size_t i = 0; i < kick.size() && off + i < out.size(); ++i)
            out[off + i] += kick[i];
    }
    return out;
}

StereoBuffer render_rest(const BarSpec& spec, size_t bar_len,
                          const Buffer& snare, const Buffer& hat_c,
                          const Buffer& hat_o,  const Buffer& clang,
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
    auto place_stereo = [&](const StereoBuffer& src, double t_sec, float gain) {
        const size_t off = static_cast<size_t>(t_sec * sr + 0.5);
        for (size_t i = 0; i < src.size() && off + i < out.size(); ++i) {
            out.L[off + i] += src.L[i] * gain;
            out.R[off + i] += src.R[i] * gain;
        }
    };

    if (spec.snare_roll) {
        for (int s = 0; s < 16; ++s)
            place_mono(snare, s, (s % 4 == 0) ? 0.75f : 0.40f);
    } else if (spec.snare_on) {
        place_mono(snare, 4,  0.78f);
        place_mono(snare, 12, 0.78f);
    }

    if (spec.hat_c_roll) {
        for (int s = 0; s < 16; ++s)
            place_mono(hat_c, s, (s % 2 == 0) ? 0.28f : 0.18f);
    } else if (spec.hat_c_on) {
        for (int s = 0; s < 16; s += 2)
            place_mono(hat_c, s, 0.24f);
    }
    if (spec.hat_o_beat) place_mono(hat_o, 8, 0.38f);

    if (spec.clang_on) {
        place_mono(clang, 2,  0.40f);
        place_mono(clang, 10, 0.40f);
    }

    if (!spec.stab_freqs.empty() && !spec.stab_steps.empty()) {
        for (int s : spec.stab_steps) {
            Buffer stab = make_stab(spec.stab_freqs, STEP * 2.0, sr);
            place_mono(stab, s, spec.stab_gain);
        }
    }

    if (spec.bass_freq > 0.0 && spec.bass_div > 0) {
        const int slots = 16 / spec.bass_div;
        const double note_dur = STEP * static_cast<double>(spec.bass_div) * 0.90;
        for (int h = 0; h < slots; ++h) {
            const double freq = (spec.bass_div >= 4 && (h % 4 == 2))
                ? spec.bass_freq * 1.5
                : spec.bass_freq;
            Buffer b = make_fm_bass(freq, note_dur, sr);
            place_mono(b, h * spec.bass_div, spec.bass_gain);
        }
    }

    if (!spec.pad_freqs.empty() && spec.pad_gain > 0.0f) {
        StereoBuffer pad = make_gothic_pad(spec.pad_freqs,
                                            static_cast<double>(bar_len) / sr, sr);
        mix_stereo(out, pad, spec.pad_gain);
    }

    if (spec.drone_freq > 0.0 && spec.drone_gain > 0.0f) {
        StereoBuffer dr = make_drone(spec.drone_freq,
                                      static_cast<double>(bar_len) / sr, sr);
        mix_stereo(out, dr, spec.drone_gain);
    }

    if (!spec.screech.empty()) {
        double t = 0.0;
        for (auto& [freq, dur_8ths] : spec.screech) {
            const double dur = dur_8ths * STEP8;
            if (freq > 0.0 && dur > 0.0) {
                StereoBuffer note = make_screech(freq, dur, sr);
                place_stereo(note, t, spec.screech_gain);
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
    song.reserve(240);

    auto A = [](int i) -> const HarmonicArea& { return AREAS[i % 4]; };

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = 0.08f + 0.04f * i;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.03f + 0.02f * i;

        if (i >= 4)  b.clang_on = true;
        if (i >= 8)  b.hat_c_on = true;
        if (i >= 12) {
            b.kick_on   = true;
            b.kick_beat = true;
        }
        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on   = true;
        b.kick_beat = (i < 8);
        b.kick_8th  = (i >= 8);

        b.hat_c_on   = true;
        b.hat_c_roll = (i >= 24);
        b.hat_o_beat = (i >= 16);
        b.clang_on   = true;
        b.snare_on   = (i >= 8);
        b.snare_roll = (i >= 28 && i % 8 == 7);

        b.bass_freq = ar.root;
        b.bass_div  = (i < 16) ? 4 : 2;
        b.bass_gain = 0.28f + 0.005f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.12f + 0.007f * loop;

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = 0.30f - 0.005f * loop;

        if (i >= 20) {
            b.screech = { {ar.mel_a[0], 8.0} };
            b.screech_gain = 0.18f + 0.02f * (i - 20);
        }
        song.push_back(b);
    }

    for (int i = 0; i < 48; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on         = true;
        b.kick_8th        = (loop % 3 != 2);
        b.kick_syncopated = (loop % 3 == 2);
        if (i % 8 == 7) { b.kick_8th = false; b.kick_syncopated = false; b.kick_16th = true; }

        b.snare_on   = true;
        b.snare_roll = (i % 8 == 7 && loop >= 6);
        b.hat_c_on   = true;
        b.hat_c_roll = (i % 4 == 3);
        b.hat_o_beat = (loop % 2 == 0);
        b.clang_on   = true;

        b.bass_freq = ar.root;
        b.bass_div  = (i % 4 == 3) ? 1 : 2;
        b.bass_gain = 0.38f + 0.003f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = (loop % 2 == 0) ? std::vector<int>{6, 14}
                                        : std::vector<int>{2, 10, 14};
        b.stab_gain  = 0.28f + 0.003f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.16f + 0.003f * loop;

        const auto& mel = (loop % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.screech.push_back({f, 1.0});
        b.screech_gain = 0.46f + 0.003f * loop;

        if (loop >= 4) {
            const auto& acid_mel = (loop % 2 == 0) ? ar.mel_b : ar.mel_a;
            for (int n = 0; n < 4; ++n)
                b.acid.push_back({acid_mel[n * 2], 2.0});
            b.acid_gain = 0.16f + 0.006f * (loop - 4);
        }
        song.push_back(b);
    }

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i + 1);

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = 0.50f + 0.01f * i;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.40f - 0.015f * i;

        if (i == 0 || i == 8) { b.kick_on = true; b.kick_beat = true; }
        if (i % 4 == 0)        b.clang_on = true;

        b.acid = { {ar.mel_b[0], 4.0}, {ar.mel_b[4], 4.0} };
        b.acid_gain = 0.26f + 0.015f * i;

        if (i >= 12) {
            b.screech = { {ar.mel_a[0], 8.0} };
            b.screech_gain = 0.08f + 0.06f * (i - 12);
        }
        song.push_back(b);
    }

    for (int i = 0; i < 32; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on         = (i >= 2);
        b.kick_syncopated = (i >= 2  && loop % 2 == 0);
        b.kick_8th        = (i >= 2  && loop % 2 == 1);
        if (i >= 2 && i % 8 == 7 && loop >= 4) {
            b.kick_syncopated = false;
            b.kick_8th = false;
            b.kick_16th = true;
        }

        b.snare_on   = (i >= 4);
        b.snare_roll = (i % 8 == 7 && loop >= 5);
        b.hat_c_on   = (i >= 2);
        b.hat_c_roll = (i % 4 == 3 && loop >= 3);
        b.hat_o_beat = (loop % 2 == 1);
        b.clang_on   = (i >= 4);

        b.bass_freq = ar.root;
        b.bass_div  = (i < 16) ? 2 : 1;
        b.bass_gain = 0.34f + 0.004f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.14f + 0.005f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = (loop % 2 == 0) ? std::vector<int>{3, 11}
                                        : std::vector<int>{1, 7, 13};
        b.stab_gain  = 0.22f + 0.005f * loop;

        const auto& mel = (loop % 2 == 0) ? ar.mel_a : ar.mel_b;
        for (double f : mel) b.screech.push_back({f, 1.0});
        b.screech_gain = 0.38f + 0.005f * loop;

        const auto& acid_mel = (loop % 2 == 0) ? ar.mel_b : ar.mel_a;
        for (int n = 0; n < 4; ++n)
            b.acid.push_back({acid_mel[n], 2.0});
        b.acid_gain = 0.20f + 0.006f * loop;

        song.push_back(b);
    }

    for (int i = 0; i < 64; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);
        const int loop = i / 4;

        b.kick_on         = true;
        b.kick_16th       = (loop % 3 == 0);
        b.kick_syncopated = (loop % 3 == 1);
        b.kick_8th        = (loop % 3 == 2);
        if (i % 8 == 7) { b.kick_16th = true; b.kick_syncopated = false; b.kick_8th = false; }

        b.snare_on   = true;
        b.snare_roll = (i % 4 == 3);
        b.hat_c_roll = true;
        b.hat_o_beat = (loop % 2 == 0);
        b.clang_on   = (loop % 2 == 1);

        b.bass_freq = ar.root;
        b.bass_div  = 1;
        b.bass_gain = 0.38f + 0.002f * loop;

        b.stab_freqs = ar.stab;
        b.stab_steps = {0, 3, 7, 10, 13};
        b.stab_gain  = 0.26f + 0.002f * loop;

        b.pad_freqs = ar.pad;
        b.pad_gain  = 0.18f + 0.002f * loop;

        const auto& mel       = (i % 2 == 0) ? ar.mel_a : ar.mel_b;
        const auto& acid_mel  = (i % 2 == 0) ? ar.mel_b : ar.mel_a;

        for (double f : mel)      b.screech.push_back({f, 1.0});
        for (double f : acid_mel) b.acid.push_back({f, 1.0});

        b.screech_gain = 0.52f + 0.002f * loop;
        b.acid_gain    = 0.28f + 0.002f * loop;

        song.push_back(b);
    }

    for (int i = 0; i < 16; ++i) {
        BarSpec b;
        const HarmonicArea& ar = A(i);

        b.pad_freqs = ar.pad;
        b.pad_gain  = std::max(0.0f, 0.44f - 0.03f * i);

        b.drone_freq = ar.root * 2.0;
        b.drone_gain = std::min(0.55f, 0.02f * i);

        if (i < 4) {
            b.kick_on   = true;
            b.kick_beat = true;
            b.snare_on  = true;
            b.clang_on  = true;
            b.bass_freq = ar.root;
            b.bass_div  = 4;
            b.bass_gain = 0.35f;

            const auto& mel = ar.mel_a;
            for (double f : mel) b.screech.push_back({f, 1.0});
            b.screech_gain = 0.40f;
        } else if (i < 8) {
            b.kick_on     = true;
            b.kick_beat   = true;
            b.bass_freq   = ar.root;
            b.bass_div    = 4;
            b.bass_gain   = 0.32f - 0.04f * (i - 4);
            b.screech = { {ar.mel_a[0], 8.0} };
            b.screech_gain = std::max(0.0f, 0.35f - 0.06f * (i - 4));
        } else if (i == 8 || i == 9) {
            b.kick_on     = true;
            b.kick_beat   = true;
        }
        song.push_back(b);
    }

    return song;
}

void print_help(const char* prog) {
    std::cout <<
        "I really need better names - gothic hardcore\n"
        "Usage:\n"
        "  " << prog << " [options]\n\n"
        "Options:\n"
        "  -o, --output <file>   Output WAV (default: hard.wav)\n"
        "      --play            Play after rendering (press Enter to stop)\n"
        "  -h, --help            Show this help\n";
}

int main(int argc, char** argv) {
    std::string output  = "hard.wav";
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

    std::cout << "\n  It's pretty hard\n";
    std::cout << "  Tempo : " << BPM << " BPM\n";
    std::cout << "  Key   : E minor\n";

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
        Buffer       kb = render_kick(song[i], bar_len, kick, SR);
        StereoBuffer rb = render_rest(song[i], bar_len,
                                      snare, hat_c, hat_o, clang, SR);
        const size_t off = i * bar_len;
        for (size_t s = 0; s < bar_len; ++s) {
            kick_track.L[off + s] = kb[s];
            kick_track.R[off + s] = kb[s];
            rest_track.L[off + s] = rb.L[s];
            rest_track.R[off + s] = rb.R[s];
        }
        std::cout << "  Rendering bar " << (i+1) << " / " << song.size() << "\r" << std::flush;
    }
    std::cout << "\n";

    auto mix_at = [&](StereoBuffer& dst, const StereoBuffer& src,
                       size_t bar_off, float gain) {
        const size_t off = bar_off * bar_len;
        for (size_t i = 0; i < src.size() && off + i < dst.size(); ++i) {
            dst.L[off + i] += src.L[i] * gain;
            dst.R[off + i] += src.R[i] * gain;
        }
    };

    { auto r = make_riser(4.0*BAR, SR); mix_at(rest_track, r, 12, 0.55f); }
    { auto m = make_impact(2.0*BAR, SR); mix_at(rest_track, m, 16, 0.65f); }

    { auto r = make_riser(4.0*BAR, SR); mix_at(rest_track, r, 44, 0.65f); }
    { auto m = make_impact(2.0*BAR, SR); mix_at(rest_track, m, 48, 0.80f); }

    { auto m = make_impact(2.0*BAR, SR); mix_at(rest_track, m, 96, 0.72f); }

    { auto r = make_riser(4.0*BAR, SR); mix_at(rest_track, r, 108, 0.60f); }
    { auto m = make_impact(2.0*BAR, SR); mix_at(rest_track, m, 112, 0.78f); }

    { auto r = make_riser(4.0*BAR, SR); mix_at(rest_track, r, 140, 0.75f); }
    { auto m = make_impact(2.0*BAR, SR); mix_at(rest_track, m, 144, 0.92f); }

    { auto r = make_riser(2.0*BAR, SR); mix_at(rest_track, r, 174, 0.60f); }
    { auto m = make_impact(1.0*BAR, SR); mix_at(rest_track, m, 176, 0.85f); }

    SidechainCompressor sc;
    sc.attack_ms  = 2.0f;
    sc.release_ms = 100.0f;
    sc.strength   = 0.60f;
    sc.threshold  = 0.022f;
    Buffer kick_mono = kick_track.to_mono();
    sc.apply(rest_track, kick_mono, SR);

    StereoBuffer mix(total, 0.0f);
    mix_into(mix, kick_track, 0.72f);
    mix_into(mix, rest_track, 1.0f);

    distortion::apply_soft_clip(mix.L, 1.40f);
    distortion::apply_soft_clip(mix.R, 1.40f);
    fade_in (mix, 0.12, SR);
    fade_out(mix, 3.0,  SR);
    normalize(mix, 0.95f);
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
