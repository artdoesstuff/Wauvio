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
constexpr double BPM  = 240.0;
constexpr double BAR  = 60.0 / BPM * 4.0;
constexpr double BEAT = BAR / 4.0;
constexpr double STEP = BAR / 16.0;

Buffer make_kick(int sr = SR) {
    const double dur = 0.20;
    Buffer buf = make_buffer(dur, sr);
    double phase = 0.0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        const double f = 40.0 + 180.0 * std::exp(-t * 45.0);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        buf[i] = static_cast<float>(std::sin(phase));
    }
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 18.0));
    }
    NoiseGenerator noise(909u);
    const size_t click_len = static_cast<size_t>(0.005 * sr);
    for (size_t i = 0; i < buf.size() && i < click_len; ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] += noise.tick() * static_cast<float>(std::exp(-t * 450.0)) * 0.7f;
    }
    distortion::apply_waveshaper(buf, 0.9f);
    distortion::apply_hard_clip(buf, 0.88f);
    distortion::apply_soft_clip(buf, 3.5f);
    normalize(buf, 0.90f);
    return buf;
}

Buffer make_hat(int sr, bool open) {
    const double dur   = open ? 0.22 : 0.048;
    const double decay = open ? 10.0 : 65.0;
    NoiseGenerator noise(open ? 77u : 7u);
    Buffer buf = noise.render(dur, sr);
    HighPassFilter hpf(8800.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * decay));
    }
    normalize(buf, open ? 0.55f : 0.40f);
    return buf;
}

Buffer make_clap(int sr) {
    const double dur = 0.15;
    NoiseGenerator noise(303u);
    Buffer buf = noise.render(dur, sr);
    SVFilter bp(SVFilter::Mode::BandPass, 1600.0, 2.2);
    bp.process(buf, sr);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        double env = std::exp(-t * 28.0);
        if (t < 0.025) env *= 0.55 + 0.45 * std::sin(t * 320.0);
        buf[i] *= static_cast<float>(env);
    }
    distortion::apply_soft_clip(buf, 1.5f);
    normalize(buf, 0.70f);
    return buf;
}

Buffer make_bass_note(double freq, double sub_freq, double duration, int sr) {
    if (duration <= 0.0 || freq <= 0.0) return Buffer();
    Buffer buf = make_buffer(duration, sr);
    Oscillator o1(WaveShape::Sawtooth, freq,         0.45);
    Oscillator o2(WaveShape::Sawtooth, freq * 1.014, 0.45);
    for (size_t i = 0; i < buf.size(); ++i)
        buf[i] = o1.tick(sr) + o2.tick(sr);
    std::vector<AutoPoint> pts = {
        {0.0,            1600.0f},
        {duration * 0.2,  800.0f},
        {duration,        300.0f}
    };
    buf = apply_filter_automation(buf, pts, 4.0, SVFilter::Mode::LowPass, sr);
    distortion::apply_waveshaper(buf, 0.65f);
    Oscillator sub(WaveShape::Sine, sub_freq, 0.30);
    for (size_t i = 0; i < buf.size(); ++i)
        buf[i] += sub.tick(sr);
    DAHDSR env(0.0, 0.002, 0.0, duration * 0.25, 0.80, duration * 0.20);
    env.apply(buf, duration, std::max(0.0, duration - duration * 0.20), sr);
    normalize(buf, 0.82f);
    return buf;
}

StereoBuffer make_lead_note(double freq, double duration, int sr) {
    if (duration <= 0.0 || freq <= 0.0)
        return make_stereo(duration > 0 ? duration : 0.001, sr);
    Supersaw saw;
    saw.frequency    = freq;
    saw.voices       = 7;
    saw.detune_cents = 22.0;
    saw.amplitude    = 0.55;
    saw.mix_center   = 0.65;
    Buffer mono = saw.render(duration, sr);
    std::vector<AutoPoint> pts = {
        {0.0,             600.0f},
        {duration * 0.20, 5500.0f},
        {duration,        2400.0f}
    };
    mono = apply_filter_automation(mono, pts, 3.2, SVFilter::Mode::LowPass, sr);
    distortion::apply_soft_clip(mono, 1.4f);
    const double dr = std::max(0.003, duration * 0.20);
    const double rl = std::max(0.003, duration * 0.18);
    DAHDSR env(0.0, 0.002, 0.0, dr, 0.70, rl);
    env.apply(mono, duration, std::max(0.0, duration - rl), sr);
    return haas_widen(mono, 15.0, sr);
}

StereoBuffer make_screech_note(double freq, double duration, int sr) {
    if (duration <= 0.0 || freq <= 0.0)
        return make_stereo(duration > 0 ? duration : 0.001, sr);
    Supersaw saw;
    saw.frequency    = freq * 2.0;
    saw.voices       = 5;
    saw.detune_cents = 30.0;
    saw.amplitude    = 0.35;
    saw.mix_center   = 0.55;
    Buffer mono = saw.render(duration, sr);
    std::vector<AutoPoint> pts = {
        {0.0,            2000.0f},
        {duration * 0.4, 8500.0f},
        {duration,       1400.0f}
    };
    mono = apply_filter_automation(mono, pts, 8.0, SVFilter::Mode::BandPass, sr);
    distortion::apply_hard_clip(mono, 0.50f);
    const double dr = std::max(0.003, duration * 0.25);
    const double rl = std::max(0.003, duration * 0.18);
    DAHDSR env(0.0, 0.002, 0.0, dr, 0.55, rl);
    env.apply(mono, duration, std::max(0.0, duration - rl), sr);
    return haas_widen(mono, 12.0, sr);
}

StereoBuffer make_drone(double root_freq, double duration, int sr) {
    Buffer mono = make_buffer(duration, sr);
    Oscillator o1(WaveShape::Sine, root_freq,        0.50);
    Oscillator o2(WaveShape::Sine, root_freq * 1.008, 0.40);
    Oscillator o3(WaveShape::Sine, root_freq * 0.5,  0.35);
    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] = o1.tick(sr) + o2.tick(sr) + o3.tick(sr);
    for (size_t i = 0; i < mono.size(); ++i) {
        const double t    = static_cast<double>(i) / sr;
        const double fade = std::min(1.0, t / (duration * 0.3));
        const double tail = (t > duration * 0.9)
            ? 1.0 - (t - duration * 0.9) / (duration * 0.1) : 1.0;
        mono[i] *= static_cast<float>(fade * tail);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 400.0, 0.7);
    lp.process(mono, sr);
    normalize(mono, 0.55f);
    return haas_widen(mono, 25.0, sr);
}

Buffer make_stab(const std::vector<double>& freqs, int sr) {
    const double dur = 0.10;
    Buffer buf = make_buffer(dur, sr);
    for (double f : freqs) {
        Oscillator o(WaveShape::Sawtooth, f * 2.0, 0.35);
        for (size_t i = 0; i < buf.size(); ++i) buf[i] += o.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 3000.0, 1.5);
    lp.process(buf, sr);
    distortion::apply_hard_clip(buf, 0.52f);
    DAHDSR env(0.0, 0.001, 0.0, 0.03, 0.0, 0.05);
    env.apply(buf, dur, dur * 0.40, sr);
    normalize(buf, 0.75f);
    return buf;
}

StereoBuffer make_pad_chord(const std::vector<double>& freqs, double duration, int sr) {
    Buffer mono = make_buffer(duration, sr);
    for (double f : freqs) {
        Oscillator o1(WaveShape::Sawtooth, f,       0.22);
        Oscillator o2(WaveShape::Triangle, f * 2.0, 0.10);
        Oscillator s (WaveShape::Sine,     f * 0.5, 0.15);
        for (size_t i = 0; i < mono.size(); ++i)
            mono[i] += o1.tick(sr) + o2.tick(sr) + s.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, 1600.0, 0.85);
    lp.process(mono, sr);
    DAHDSR env(0.0,
               std::min(0.5,  duration * 0.38), 0.0,
               std::min(0.32, duration * 0.28), 0.82,
               std::min(0.38, duration * 0.28));
    env.apply(mono, duration,
              std::max(0.0, duration - std::min(0.38, duration * 0.28)), sr);
    StereoBuffer out = haas_widen(mono, 20.0, sr);
    Reverb rvL; rvL.room_size = 0.75f; rvL.wet = 0.38f; rvL.dry = 0.82f; rvL.init(sr);
    Reverb rvR; rvR.room_size = 0.80f; rvR.wet = 0.38f; rvR.dry = 0.82f; rvR.init(sr);
    rvL.process(out.L);
    rvR.process(out.R);
    return out;
}

StereoBuffer make_riser(double duration, int sr) {
    NoiseGenerator noise(555u);
    Buffer buf = noise.render(duration, sr);
    std::vector<AutoPoint> cutoff = { {0.0, 250.0f}, {duration, 10000.0f} };
    buf = apply_filter_automation(buf, cutoff, 1.8, SVFilter::Mode::BandPass, sr);
    std::vector<AutoPoint> vol = { {0.0, 0.03f}, {duration, 1.0f} };
    apply_volume_automation(buf, vol, sr);
    Oscillator o(WaveShape::Sawtooth, 60.0, 0.28);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        o.frequency = 60.0 + 1400.0 * (t / duration);
        buf[i] += o.tick(sr) * static_cast<float>(0.04 + 0.60 * (t / duration));
    }
    clamp_buffer(buf);
    return haas_widen(buf, 22.0, sr);
}

StereoBuffer make_impact(double duration, int sr) {
    NoiseGenerator noise(7777u);
    Buffer buf = noise.render(duration, sr);
    HighPassFilter hpf(1600.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 2.2));
    }
    Reverb rv; rv.room_size = 0.92f; rv.wet = 0.55f; rv.dry = 0.65f; rv.init(sr);
    rv.process(buf);
    distortion::apply_soft_clip(buf, 1.3f);
    normalize(buf, 0.88f);
    return haas_widen(buf, 28.0, sr);
}

struct ChordEx {
    std::vector<double> pad;
    double              bass_root;
    std::vector<double> riff_a;
    std::vector<double> riff_b;
};

const std::vector<ChordEx> chords = {
    {
        {notes::E3, notes::G3, notes::B3},
        notes::E3 / 2.0,
        {notes::E5, notes::G5, notes::B5, notes::E5, notes::B4, notes::G4, notes::E4, notes::B4},
        {notes::B5, notes::E5, notes::G5, notes::B4, notes::E5, notes::B5, notes::G5, notes::E5}
    },
    {
        {notes::A3, notes::C4, notes::E4},
        notes::A2,
        {notes::A4, notes::C5, notes::E5, notes::A4, notes::E4, notes::C4, notes::A3, notes::E4},
        {notes::E5, notes::A4, notes::C5, notes::E4, notes::A4, notes::E5, notes::C5, notes::A4}
    },
    {
        {notes::C4, notes::E4, notes::G4},
        notes::C3,
        {notes::C5, notes::E5, notes::G5, notes::C5, notes::G4, notes::E4, notes::C4, notes::G4},
        {notes::G5, notes::C5, notes::E5, notes::G4, notes::C5, notes::G5, notes::E5, notes::C5}
    },
    {
        {notes::D4, notes::Fs4, notes::A4},
        notes::D3,
        {notes::D5, notes::Fs4, notes::A4, notes::D5, notes::A4, notes::Fs4, notes::D4, notes::A3},
        {notes::A4, notes::D5, notes::Fs4, notes::A3, notes::D5, notes::A4, notes::Fs4, notes::D5}
    },
};

struct BarSpec {
    bool kick_on        = false;
    bool kick_hardcore  = false;
    bool kick_single    = false;
    bool kick_roll      = false;
    bool half_time      = false;

    bool hat_on         = false;
    bool hat_roll       = false;
    bool open_hat_on    = false;
    bool clap_on        = false;
    bool stab_on        = false;

    double bass_freq    = 0.0;
    int    bass_pattern = 0;
    float  bass_gain    = 0.38f;

    std::vector<double> pad_chord;
    float pad_gain      = 0.0f;

    double drone_freq   = 0.0;
    float  drone_gain   = 0.0f;

    std::vector<std::pair<double,double>> lead_notes;
    float lead_gain     = 0.58f;

    std::vector<std::pair<double,double>> screech_notes;
    float screech_gain  = 0.28f;
};

std::vector<BarSpec> build_song() {
    std::vector<BarSpec> bars;
    bars.reserve(285);
    
    for (int i = 0; i < 4; ++i) {
        BarSpec b;
        b.pad_chord = chords[0].pad;
        b.pad_gain  = 0.04f;
        if (i == 2) {
            b.kick_on     = true;
            b.kick_single = true;
        }
        bars.push_back(b);
    }

    for (int i = 0; i < 12; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 3) % 4];
        b.pad_chord = c.pad;
        b.pad_gain  = 0.18f + 0.04f * i;

        if (i >= 6) {
            b.lead_notes.push_back({c.riff_a[0], 4.0});
            b.lead_gain = 0.22f + 0.06f * (i - 6);
        }
        bars.push_back(b);
    }

    for (int i = 0; i < 8; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 2) % 4];
        b.pad_chord     = c.pad;
        b.pad_gain      = 0.38f;
        b.hat_on        = (i >= 2);
        b.kick_on       = (i >= 3);
        b.kick_hardcore = (i >= 6);
        b.clap_on       = (i == 7);

        b.lead_notes.push_back({c.riff_a[0], 2.0});
        b.lead_notes.push_back({c.riff_a[2], 2.0});
        b.lead_gain = 0.40f + 0.05f * i;

        bars.push_back(b);
    }

    for (int i = 0; i < 48; ++i) {
        BarSpec b;
        const ChordEx& c = chords[i % 4];
        const int loop = i / 4;

        b.kick_on       = true;
        b.kick_hardcore = true;
        b.hat_on        = true;
        b.open_hat_on   = (i % 4 == 3);
        b.clap_on       = (i % 4 == 1 || i % 4 == 3);
        b.stab_on       = (i % 2 == 1);

        if (loop >= 6 && (i % 8 == 7))  b.hat_roll  = true;
        if (loop >= 9 && (i % 4 == 3))  b.kick_roll = true;

        b.bass_freq    = c.bass_root;
        b.bass_pattern = 2;
        b.bass_gain    = 0.35f + 0.004f * loop;

        for (double f : c.riff_a) b.lead_notes.push_back({f, 0.5});
        b.lead_gain = 0.58f + 0.004f * loop;

        b.pad_chord = c.pad;
        b.pad_gain  = 0.15f;
        bars.push_back(b);
    }

    for (int i = 0; i < 8; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 2) % 4];
        b.pad_chord   = c.pad;
        b.pad_gain    = 0.62f;

        b.lead_notes.push_back({c.riff_a[0], 2.0});
        b.lead_notes.push_back({c.riff_a[2], 2.0});
        b.lead_gain   = 0.36f;

        b.open_hat_on = (i % 4 == 3);
        if (i == 0 || i == 4) { b.kick_on = true; b.kick_single = true; }

        bars.push_back(b);
    }

    for (int i = 0; i < 8; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 2) % 4];
        b.pad_chord     = c.pad;
        b.pad_gain      = 0.44f;
        b.hat_on        = true;
        b.kick_on       = (i >= 2);
        b.kick_hardcore = (i >= 4);
        b.clap_on       = (i >= 6);

        b.lead_notes.push_back({c.riff_a[0], 2.0});
        b.lead_notes.push_back({c.riff_a[4], 2.0});
        b.lead_gain = 0.50f + 0.05f * i;

        if (i >= 6) {
            b.screech_notes.push_back({c.riff_a[0], 4.0});
            b.screech_gain = 0.14f;
        }
        bars.push_back(b);
    }

    for (int i = 0; i < 48; ++i) {
        BarSpec b;
        const ChordEx& c = chords[i % 4];
        const int loop = i / 4;

        b.kick_on       = true;
        b.kick_hardcore = true;
        b.kick_roll     = (i % 4 == 3);
        b.hat_on        = true;
        b.hat_roll      = (i % 8 == 7);
        b.open_hat_on   = ((i / 4) % 2 == 0);
        b.clap_on       = true;
        b.stab_on       = true;

        b.bass_freq    = c.bass_root;
        b.bass_pattern = 3;
        b.bass_gain    = 0.32f + 0.003f * loop;

        for (double f : c.riff_b) b.lead_notes.push_back({f, 0.5});
        b.lead_gain = 0.62f + 0.004f * loop;

        for (int n = 0; n < 4; ++n)
            b.screech_notes.push_back({c.riff_b[n * 2], 1.0});
        b.screech_gain = 0.26f + 0.003f * loop;

        b.pad_chord = c.pad;
        b.pad_gain  = 0.18f;
        bars.push_back(b);
    }

    for (int i = 0; i < 8; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 2) % 4];

        b.drone_freq = c.bass_root * 2.0;
        b.drone_gain = 0.55f;

        b.lead_notes.push_back({c.riff_a[0], 4.0});
        b.lead_gain = 0.20f + 0.03f * i;

        bars.push_back(b);
    }
    
    // this part was so sick :fire::fire::fire:
    for (int i = 0; i < 12; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 3) % 4];
        b.pad_chord = c.pad;
        b.pad_gain  = 0.48f;

        b.hat_on        = true;
        b.hat_roll      = (i >= 6);
        b.stab_on       = true;

        b.kick_on       = (i >= 2);
        b.kick_hardcore = (i >= 4);
        b.kick_roll     = (i >= 8);
        b.clap_on       = (i >= 6);

        b.lead_notes.push_back({c.riff_a[0], 2.0});
        b.lead_notes.push_back({c.riff_b[0], 2.0});
        b.lead_gain = 0.52f + 0.04f * i;

        b.screech_notes.push_back({c.riff_a[2], 4.0});
        b.screech_gain = 0.14f + 0.04f * i;

        bars.push_back(b);
    }

    for (int i = 0; i < 64; ++i) {
        BarSpec b;
        const ChordEx& c = chords[i % 4];
        const int loop = i / 4;

        b.kick_on       = true;
        b.kick_hardcore = true;
        b.kick_roll     = (i % 4 == 3) || (i % 8 == 7);
        b.hat_on        = true;
        b.hat_roll      = (i % 4 >= 2);
        b.open_hat_on   = true;
        b.clap_on       = true;
        b.stab_on       = true;

        b.bass_freq    = c.bass_root;
        b.bass_pattern = 3;
        b.bass_gain    = 0.33f + 0.003f * loop;

        const std::vector<double>& riff = (loop % 2 == 0) ? c.riff_a : c.riff_b;
        for (double f : riff) b.lead_notes.push_back({f, 0.5});
        b.lead_gain = 0.70f + 0.002f * loop;

        for (double f : riff) b.screech_notes.push_back({f, 0.5});
        b.screech_gain = 0.32f + 0.002f * loop;

        b.pad_chord = c.pad;
        b.pad_gain  = 0.26f;
        bars.push_back(b);
    }

    for (int i = 0; i < 13; ++i) {
        BarSpec b;
        const ChordEx& c = chords[(i / 3) % 4];
        b.pad_chord = c.pad;
        b.pad_gain  = std::max(0.0f, 0.55f - 0.045f * i);

        if (i == 0) {
            b.kick_on       = true;
            b.kick_hardcore = true;
            for (double f : c.riff_a) b.lead_notes.push_back({f, 0.5});
            b.lead_gain = 0.60f;
        } else if (i == 1) {
            b.kick_on     = true;
            b.kick_single = true;
            b.lead_notes.push_back({c.riff_a[0], 4.0});
            b.lead_gain = 0.42f;
        } else if (i == 3 || i == 6) {
            b.kick_on     = true;
            b.kick_single = true;
        }
        bars.push_back(b);
    }

    return bars;
}

Buffer render_bar_kick(const BarSpec& spec, size_t bar_len, const Buffer& kick) {
    Buffer out(bar_len, 0.0f);
    if (!spec.kick_on) return out;

    std::vector<int> steps;
    if      (spec.kick_single)   steps = {0};
    else if (spec.kick_roll)     steps = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    else if (spec.half_time)     steps = {0, 8};
    else if (spec.kick_hardcore) steps = {0, 2, 4, 6, 8, 10, 12, 14};
    else                          steps = {0, 4, 8, 12};

    for (int s : steps) {
        const size_t off = static_cast<size_t>(s * STEP * SR);
        for (size_t i = 0; i < kick.size() && off + i < out.size(); ++i)
            out[off + i] += kick[i];
    }
    return out;
}

StereoBuffer render_bar_rest(const BarSpec& spec, size_t bar_len,
                              const Buffer& hat_c, const Buffer& hat_o,
                              const Buffer& clap)
{
    StereoBuffer out(bar_len, 0.0f);

    auto mix_mono_at = [&](const Buffer& src, int step, float gain) {
        const size_t off = static_cast<size_t>(step * STEP * SR);
        for (size_t i = 0; i < src.size() && off + i < out.size(); ++i) {
            out.L[off + i] += src[i] * gain;
            out.R[off + i] += src[i] * gain;
        }
    };

    if (spec.hat_roll) {
        for (int s = 0; s < 16; ++s)
            mix_mono_at(hat_c, s, (s % 2 == 0) ? 0.20f : 0.13f);
    } else if (spec.hat_on) {
        for (int s : {1, 3, 5, 7, 9, 11, 13, 15})
            mix_mono_at(hat_c, s, 0.16f);
    }
    if (spec.open_hat_on)
        mix_mono_at(hat_o, 14, 0.14f);

    if (spec.clap_on)
        for (int s : {4, 12}) mix_mono_at(clap, s, 0.24f);

    if (spec.stab_on && !spec.pad_chord.empty()) {
        Buffer stab = make_stab(spec.pad_chord, SR);
        mix_mono_at(stab, 14, 0.24f);
    }

    if (spec.bass_freq > 0.0 && spec.bass_pattern == 2) {
        for (int s = 0; s < 16; s += 2) {
            const double freq = spec.bass_freq * ((s % 4 == 2) ? 2.0 : 1.0);
            Buffer b = make_bass_note(freq, spec.bass_freq, STEP * 2.0 * 0.92, SR);
            mix_mono_at(b, s, spec.bass_gain);
        }
    } else if (spec.bass_freq > 0.0 && spec.bass_pattern == 3) {
        for (int s = 0; s < 16; ++s) {
            const double freq = spec.bass_freq * ((s % 4 == 2) ? 2.0 : 1.0);
            Buffer b = make_bass_note(freq, spec.bass_freq, STEP * 0.88, SR);
            mix_mono_at(b, s, spec.bass_gain * 0.80f);
        }
    }

    if (!spec.pad_chord.empty() && spec.pad_gain > 0.0f) {
        StereoBuffer pad = make_pad_chord(spec.pad_chord,
                                          static_cast<double>(bar_len) / SR, SR);
        mix_stereo(out, pad, spec.pad_gain);
    }

    if (spec.drone_freq > 0.0 && spec.drone_gain > 0.0f) {
        StereoBuffer drone = make_drone(spec.drone_freq,
                                        static_cast<double>(bar_len) / SR, SR);
        mix_stereo(out, drone, spec.drone_gain);
    }

    if (!spec.lead_notes.empty()) {
        double t = 0.0;
        for (auto& nd : spec.lead_notes) {
            const double freq = nd.first;
            const double dur  = nd.second * BEAT;
            if (freq > 0.0 && dur > 0.0) {
                StereoBuffer note = make_lead_note(freq, dur, SR);
                const size_t off  = static_cast<size_t>(t * SR);
                for (size_t i = 0; i < note.size() && off + i < out.size(); ++i) {
                    out.L[off + i] += note.L[i] * spec.lead_gain;
                    out.R[off + i] += note.R[i] * spec.lead_gain;
                }
            }
            t += dur;
        }
    }

    if (!spec.screech_notes.empty()) {
        double t = 0.0;
        for (auto& nd : spec.screech_notes) {
            const double freq = nd.first;
            const double dur  = nd.second * BEAT;
            if (freq > 0.0 && dur > 0.0) {
                StereoBuffer note = make_screech_note(freq, dur, SR);
                const size_t off  = static_cast<size_t>(t * SR);
                for (size_t i = 0; i < note.size() && off + i < out.size(); ++i) {
                    out.L[off + i] += note.L[i] * spec.screech_gain;
                    out.R[off + i] += note.R[i] * spec.screech_gain;
                }
            }
            t += dur;
        }
    }

    return out;
}

void print_help(const char* prog) {
    std::cout <<
        "i hope it's gothic hardcore\n"
        "Usage:\n"
        "  " << prog << " [options]\n\n"
        "Options:\n"
        "  -o, --output <file>   Output WAV (default: nameless.wav)\n"
        "      --play            Play after rendering (press Enter to stop)\n"
        "  -h, --help            Show this help\n";
}

int main(int argc, char** argv) {
    std::string output  = "nameless.wav";
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

    std::cout << "No name here either\n";
    std::cout << "  Tempo : " << BPM << " BPM\n";
    std::cout << "  Key   : E minor\n";

    std::vector<BarSpec> bars = build_song();
    const size_t bar_len = static_cast<size_t>(BAR * SR);
    const size_t total   = bar_len * bars.size();

    std::cout << "  Bars  : " << bars.size() << "\n";
    std::cout << "  Length: " << std::fixed << std::setprecision(1)
              << static_cast<double>(total) / SR << " s\n\n";

    const Buffer kick  = make_kick(SR);
    const Buffer hat_c = make_hat(SR, false);
    const Buffer hat_o = make_hat(SR, true);
    const Buffer clap  = make_clap(SR);

    StereoBuffer kick_track(total, 0.0f);
    StereoBuffer rest_track(total, 0.0f);

    for (size_t i = 0; i < bars.size(); ++i) {
        Buffer       kb = render_bar_kick(bars[i], bar_len, kick);
        StereoBuffer rb = render_bar_rest(bars[i], bar_len, hat_c, hat_o, clap);

        const size_t off = i * bar_len;
        for (size_t s = 0; s < bar_len; ++s) {
            kick_track.L[off + s] = kb[s];
            kick_track.R[off + s] = kb[s];
            rest_track.L[off + s] = rb.L[s];
            rest_track.R[off + s] = rb.R[s];
        }
        std::cout << "  Rendering bar " << (i + 1) << " / " << bars.size() << "\r" << std::flush;
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

    { auto r = make_riser(4.0 * BAR, SR); mix_stereo_at(rest_track, r, 20, 0.55f); }
    { auto m = make_impact(2.0 * BAR, SR); mix_stereo_at(rest_track, m, 22, 0.65f); }

    { auto r = make_riser(4.0 * BAR, SR); mix_stereo_at(rest_track, r, 84, 0.62f); }
    { auto m = make_impact(2.0 * BAR, SR); mix_stereo_at(rest_track, m, 86, 0.72f); }

    { auto r = make_riser(4.0 * BAR, SR); mix_stereo_at(rest_track, r, 152, 0.75f); }
    { auto m = make_impact(2.0 * BAR, SR); mix_stereo_at(rest_track, m, 154, 0.88f); }

    SidechainCompressor sc;
    sc.attack_ms  = 2.5f;
    sc.release_ms = 120.0f;
    sc.strength   = 0.65f;
    sc.threshold  = 0.025f;
    Buffer kick_mono = kick_track.to_mono();
    sc.apply(rest_track, kick_mono, SR);

    StereoBuffer mix(total, 0.0f);
    mix_into(mix, kick_track, 0.68f);
    mix_into(mix, rest_track, 1.0f);

    distortion::apply_soft_clip(mix.L, 1.35f);
    distortion::apply_soft_clip(mix.R, 1.35f);
    fade_in(mix, 0.10, SR);
    fade_out(mix, 2.5, SR);
    normalize(mix, 0.94f);
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