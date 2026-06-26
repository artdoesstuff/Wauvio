#include "wauvio.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <thread>

using namespace wauvio;

constexpr int SR = 44100;

struct NoteEvent { float t, dur, freq; };

static const float hat_times[] = {
    8.250f,  8.750f,  9.250f,  9.750f,  10.250f, 10.750f, 11.250f, 11.750f,
    12.250f, 12.750f, 13.250f, 13.750f, 14.250f, 14.750f, 15.250f, 15.750f,
    16.250f, 16.375f, 16.750f, 17.250f, 17.375f, 17.750f,
    18.250f, 18.375f, 18.750f, 19.250f, 19.375f, 19.750f,
    20.250f, 20.375f, 20.750f, 21.250f, 21.375f, 21.750f,
    22.250f, 22.375f, 22.750f, 23.250f, 23.375f, 23.750f,
    24.250f, 24.375f, 24.750f, 25.250f, 25.375f, 25.750f,
    26.250f, 26.375f, 26.750f, 27.250f, 27.375f, 27.750f,
    28.250f, 28.375f, 28.750f, 29.250f, 29.375f, 29.750f,
    30.250f, 30.375f, 30.750f, 31.250f, 31.375f, 31.750f,
    40.250f, 40.375f, 40.750f, 41.250f, 41.375f, 41.750f,
    42.250f, 42.375f, 42.750f, 43.250f, 43.375f, 43.750f,
    44.250f, 44.375f, 44.750f, 45.250f, 45.375f, 45.750f,
    48.250f, 48.750f, 49.250f, 49.750f, 50.250f, 50.750f, 51.250f, 51.750f,
    52.250f, 52.750f, 53.250f, 53.750f, 54.250f, 54.750f, 55.250f, 55.750f,
    56.250f, 56.375f, 56.750f, 57.250f, 57.375f, 57.750f,
    58.250f, 58.375f, 58.750f, 59.250f, 59.375f, 59.750f,
    60.250f, 60.375f, 60.750f, 61.250f, 61.375f, 61.750f,
    62.250f, 62.375f, 62.750f, 63.250f, 63.375f, 63.750f,
    64.250f, 64.750f, 65.250f, 65.750f, 66.250f, 66.750f, 67.250f, 67.750f,
    68.250f, 68.750f, 69.250f, 69.750f, 70.250f, 70.750f, 71.250f, 71.750f,
    72.250f, 72.375f, 72.750f, 73.250f, 73.375f, 73.750f,
    74.250f, 74.375f, 74.750f, 75.250f, 75.375f, 75.750f,
    76.250f, 76.375f, 76.750f, 77.250f, 77.375f, 77.750f,
    78.250f, 78.375f, 78.750f, 79.250f, 79.375f, 79.750f
};
static const int hat_count = sizeof(hat_times) / sizeof(hat_times[0]);

static const float ride_times[] = {
    88.250f, 88.750f, 89.250f, 89.750f, 90.250f, 90.750f, 91.250f, 91.750f,
    92.250f, 92.750f, 93.250f, 
};
static const int ride_count = sizeof(ride_times) / sizeof(ride_times[0]);

static const float snare_times[] = {
    16.500f, 17.500f, 18.500f, 19.500f, 20.500f, 21.500f, 22.500f, 23.500f,
    24.500f, 25.500f, 26.500f, 27.500f, 28.500f, 29.500f, 30.500f, 31.500f,
    40.500f, 41.500f, 42.500f, 43.500f, 44.500f, 45.500f,
    48.500f, 49.500f, 50.500f, 51.500f, 52.500f, 53.500f, 54.500f, 55.500f,
    56.500f, 57.500f, 58.500f, 59.500f, 60.500f, 61.500f, 62.500f, 63.500f,
    64.500f, 65.500f, 66.500f, 67.500f, 68.500f, 69.500f, 70.500f, 71.500f,
    72.500f, 73.500f, 74.500f, 75.500f, 76.500f, 77.500f, 78.500f, 79.500f
};
static const int snare_count = sizeof(snare_times) / sizeof(snare_times[0]);

static const float hat_fill_times[] = {46.250f, 46.375f, 46.500f, 46.750f};
static const int   hat_fill_count   = 4;

static const float clap_times[] = {40.500f, 41.500f, 42.500f, 43.500f, 44.500f, 45.500f};
static const int   clap_count   = 6;

static const NoteEvent shaker_notes[] = {
    {40.250f, 0.124f, 0}, {40.375f, 0.124f, 0}, {40.750f, 0.249f, 0},
    {41.250f, 0.124f, 0}, {41.375f, 0.124f, 0}, {41.750f, 0.249f, 0},
    {42.250f, 0.124f, 0}, {42.375f, 0.124f, 0}, {42.750f, 0.249f, 0},
    {43.250f, 0.124f, 0}, {43.375f, 0.124f, 0}, {43.750f, 0.249f, 0},
    {44.250f, 0.124f, 0}, {44.375f, 0.124f, 0}, {44.750f, 0.249f, 0},
    {45.250f, 0.124f, 0}, {45.375f, 0.124f, 0}, {45.750f, 0.249f, 0}
};
static const int shaker_count = sizeof(shaker_notes) / sizeof(shaker_notes[0]);

static const NoteEvent abass_end[] = {
    {88.250f, 0.249f, notes::D2},  {88.750f, 0.249f, notes::D2},  {89.250f, 0.249f, notes::D2},  {89.750f, 0.249f, notes::D2},
    {90.250f, 0.249f, notes::As1}, {90.750f, 0.249f, notes::As1}, {91.250f, 0.249f, notes::As1}, {91.750f, 0.249f, notes::As1},
    {92.250f, 0.249f, notes::F1},  {92.750f, 0.249f, notes::F1},  {93.250f, 0.249f, notes::F1},  {93.750f, 0.249f, notes::F1}
};
static const int abass_end_count = sizeof(abass_end) / sizeof(abass_end[0]);

static const NoteEvent abass_end2[] = {
    {88.250f, 0.249f, notes::D2},  {88.750f, 0.249f, notes::D2},  {89.250f, 0.249f, notes::D2},  {89.750f, 0.249f, notes::D2},
    {90.250f, 0.249f, notes::As1}, {90.750f, 0.249f, notes::As1}, {91.250f, 0.249f, notes::As1}, {91.750f, 0.249f, notes::As1},
    {92.250f, 0.249f, notes::F1},  {92.750f, 0.249f, notes::F1},  {93.250f, 0.249f, notes::F1},  {93.750f, 0.249f, notes::F1}
};
static const int abass_end2_count = sizeof(abass_end2) / sizeof(abass_end2[0]);

static const NoteEvent bass_late[] = {
    {117.000f, 0.499f, notes::D2},  {117.500f, 0.499f, notes::G2}, {118.000f, 0.999f, notes::F2},
    {119.000f, 0.499f, notes::C2},  {119.500f, 0.499f, notes::G2}, {120.000f, 0.999f, notes::F2},
    {121.000f, 0.499f, notes::As1}, {121.500f, 0.499f, notes::E2}, {122.000f, 0.999f, notes::D2},
    {123.000f, 0.499f, notes::F2},  {123.500f, 0.499f, notes::G2}, {124.000f, 0.999f, notes::G2},
    {125.000f, 0.499f, notes::D2},  {125.500f, 0.499f, notes::G2}, {126.000f, 0.999f, notes::F2},
    {127.000f, 0.499f, notes::C2},  {127.500f, 0.499f, notes::G2}, {128.000f, 0.999f, notes::F2},
    {129.000f, 0.499f, notes::As1}, {129.500f, 0.499f, notes::E2}, {130.000f, 0.999f, notes::D2},
    {131.000f, 0.499f, notes::F2},  {131.500f, 0.499f, notes::G2}, {132.000f, 0.999f, notes::G2}
};
static const int bass_late_count = sizeof(bass_late) / sizeof(bass_late[0]);

static const double bass_roots[]     = {notes::D2, notes::As1, notes::F1, notes::Gs1};
static const double piano_chords[][3] = {
    {notes::D3,  notes::F3,  notes::A3},
    {notes::As2, notes::D3,  notes::F3},
    {notes::F2,  notes::Gs2, notes::C3},
    {notes::Gs2, notes::C3,  notes::Ds3}
};
static const double bright_melody[] = {
    notes::F4, notes::A4, notes::F4, notes::G4, notes::D4, notes::A3, notes::F4, notes::D4,
    notes::F4, notes::A4, notes::F4, notes::G4, notes::D4, notes::A3, notes::F4, notes::D4
};
static const double choir_base[] = {
    notes::C4, notes::F4, notes::F4, notes::A4, notes::C4, notes::F4, notes::F4, notes::G4,
    notes::A3, notes::D4, notes::C4, notes::A3, notes::E4, notes::F4, notes::E4, notes::E4
};
static const double metal_intro[] = {
    notes::D6, notes::As5, notes::G5, notes::Gs5,
    notes::D6, notes::As5, notes::G5, notes::Gs5,
};
static const double metal_high[] = {
    notes::B4,  notes::E5,  notes::Ds5, notes::Gs5,
    notes::B4,  notes::E5,  notes::Ds5, notes::Fs5,
    notes::Gs4, notes::Cs5, notes::B4,  notes::Gs4,
    notes::Ds5, notes::E5,  notes::Ds5, notes::Cs5
};
static const double scifi_hi[] = {
    notes::C5, notes::C4, notes::F5, notes::F4, notes::E5, notes::E4, notes::A5, notes::A4,
    notes::C5, notes::C4, notes::F5, notes::F4, notes::E5, notes::E4, notes::G5, notes::G4,
    notes::A4, notes::A3, notes::D5, notes::D4, notes::C5, notes::C4, notes::A4, notes::A3,
    notes::E5, notes::E4, notes::F5, notes::F4, notes::E5, notes::E4, notes::D5, notes::D4
};
static const double sine_pattern[] = {
    notes::C4, notes::F4, notes::E4, notes::A4,
    notes::C4, notes::F4, notes::E4, notes::G4,
    notes::A3, notes::D4, notes::C4, notes::A3,
    notes::E4, notes::F4, notes::E4, notes::D4
};
static const double sine_pattern_2[] = {
    notes::B4,  notes::E5,  notes::Ds5, notes::Gs5,
    notes::B4,  notes::E5,  notes::Ds5, notes::Fs5,
    notes::Gs4, notes::Cs5, notes::B4,  notes::Gs4,
    notes::Ds5, notes::E5,  notes::Ds5, notes::Cs5
};
static const double phase3_melody[] = {
    notes::D5,  notes::G5,  notes::F5,  notes::As5,
    notes::C5,  notes::G5,  notes::F5,  notes::As4,
    notes::As4, notes::E5,  notes::D5,  notes::As4,
    notes::F5,  notes::G5,  notes::G5,  notes::F5
};
static const double echoes_pat[] = {
    notes::C4, notes::F4, notes::E4, notes::A4,
    notes::C4, notes::F4, notes::E4, notes::G4,
    notes::A3, notes::D4, notes::C4, notes::A3,
    notes::E4, notes::F4, notes::E4, notes::D4
};
static const float tv_beep_times[] = {15.500f, 20.000f, 27.500f};
static const int   tv_beep_count   = 3;

Buffer make_kick(int sr = SR) {
    const double dur = 0.38;
    Buffer buf = make_buffer(dur, sr);
    double phase = 0.0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        const double f = 42.0 + 118.0 * std::exp(-t * 28.0);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        const double env = std::exp(-t * 9.0) * (1.0 - std::exp(-t * 80.0));
        buf[i] = static_cast<float>(std::sin(phase) * env);
    }
    Oscillator sub(WaveShape::Sine, 42.0, 0.25);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] += sub.tick(sr) * static_cast<float>(std::exp(-t * 12.0));
    }
    distortion::apply_soft_clip(buf, 0.900f);
    normalize(buf, 0.880f);
    return buf;
}

Buffer make_chat(int sr = SR) {
    const double dur = 0.065;
    NoiseGenerator noise(7u);
    Buffer buf = noise.render(dur, sr);
    HighPassFilter hpf(7000.0, sr);
    hpf.process(buf);
    SVFilter bp(SVFilter::Mode::BandPass, 9000.0, 2.5);
    bp.process(buf, sr);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 45.0));
    }
    normalize(buf, 0.280f);
    return buf;
}

Buffer make_ohat(int sr = SR) {
    const double dur = 0.22;
    NoiseGenerator noise(77u);
    Buffer buf = noise.render(dur, sr);
    HighPassFilter hpf(6500.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 8.0));
    }
    normalize(buf, 0.350f);
    return buf;
}

Buffer make_ride(int sr = SR) {
    const double dur = 0.40;
    NoiseGenerator noise(55u);
    Buffer buf = noise.render(dur, sr);
    HighPassFilter hpf(8000.0, sr);
    hpf.process(buf);
    SVFilter bp(SVFilter::Mode::BandPass, 10000.0, 2.0);
    bp.process(buf, sr);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 6.0));
    }
    normalize(buf, 0.320f);
    return buf;
}

Buffer make_snare(int sr = SR) {
    const double dur = 0.16;
    Buffer buf = make_buffer(dur, sr);
    double phase = 0.0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        const double f = 180.0 + 120.0 * std::exp(-t * 60.0);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        buf[i] = static_cast<float>(std::sin(phase) * std::exp(-t * 28.0) * 0.550f);
    }
    NoiseGenerator noise(303u);
    Buffer nb = noise.render(dur, sr);
    SVFilter bp(SVFilter::Mode::BandPass, 2500.0, 1.8);
    bp.process(nb, sr);
    for (size_t i = 0; i < nb.size(); ++i) {
        const double t = (double)i / sr;
        nb[i] *= static_cast<float>(std::exp(-t * 22.0));
    }
    for (size_t i = 0; i < buf.size(); ++i) buf[i] += nb[i] * 0.550f;
    distortion::apply_soft_clip(buf, 1.100f);
    normalize(buf, 0.700f);
    return buf;
}

Buffer make_shaker(double dur, int sr = SR) {
    NoiseGenerator noise(42u);
    Buffer buf = noise.render(dur, sr);
    HighPassFilter hpf(5500.0, sr);
    hpf.process(buf);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 55.0 / dur));
    }
    normalize(buf, 0.200f);
    return buf;
}

Buffer make_chat_ping(int sr = SR) {
    const double dur = 0.07;
    NoiseGenerator noise(99u);
    Buffer buf = noise.render(dur, sr);
    SVFilter r1(SVFilter::Mode::BandPass, 7200.0, 18.0);
    SVFilter r2(SVFilter::Mode::BandPass, 11400.0, 12.0);
    Buffer b2 = buf;
    r1.process(buf, sr);
    r2.process(b2, sr);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = buf[i] * 0.600f + b2[i] * 0.400f;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 55.0));
    }
    normalize(buf, 0.340f);
    return buf;
}

Buffer make_ride_ping(int sr = SR) {
    const double dur = 0.45;
    NoiseGenerator noise(55u);
    Buffer buf = noise.render(dur, sr);
    SVFilter r1(SVFilter::Mode::BandPass, 9500.0, 20.0);
    SVFilter r2(SVFilter::Mode::BandPass, 14000.0, 15.0);
    Buffer b2 = buf;
    r1.process(buf, sr);
    r2.process(b2, sr);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = buf[i] * 0.500f + b2[i] * 0.500f;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>(std::exp(-t * 5.5));
    }
    normalize(buf, 0.380f);
    return buf;
}

StereoBuffer make_sine_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    Oscillator o1(WaveShape::Sine,     freq,       vel_frac * 0.55);
    Oscillator o2(WaveShape::Triangle, freq,       vel_frac * 0.28);
    Oscillator o3(WaveShape::Sine,     freq * 2.0, vel_frac * 0.14);
    Oscillator o4(WaveShape::Sine,     freq * 0.5, vel_frac * 0.10);
    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] = o1.tick(sr) + o2.tick(sr) + o3.tick(sr) + o4.tick(sr);
    SVFilter lp(SVFilter::Mode::LowPass, freq * 5.5, 0.65);
    lp.process(mono, sr);
    const double atk = std::min(0.018, dur * 0.12);
    const double rel = std::max(0.06, dur * 0.22);
    DAHDSR env(0.0, atk, 0.0, dur * 0.12, 0.82, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);
    Chorus ch; ch.rate_hz = 0.55; ch.depth_ms = 1.8; ch.delay_ms = 8.0;
    ch.wet = 0.400f; ch.dry = 0.850f; ch.init(sr); ch.process(mono);
    Reverb rv; rv.room_size = 0.700f; rv.wet = 0.280f; rv.dry = 0.900f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.680f * (float)vel_frac);
    return haas_widen(mono, 14.0, sr);
}

StereoBuffer make_saw_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Supersaw saw;
    saw.frequency    = freq;
    saw.voices       = 7;
    saw.detune_cents = 12.0;
    saw.amplitude    = 0.48 * vel_frac;
    saw.mix_center   = 0.60;
    Buffer mono = saw.render(dur, sr);
    std::vector<AutoPoint> pts = {
        {0.0,        (float)(freq * 1.2)},
        {dur * 0.25, (float)(freq * 4.5)},
        {dur,        (float)(freq * 2.0)},
    };
    mono = apply_filter_automation(mono, pts, 0.80, SVFilter::Mode::LowPass, sr);
    const double atk = std::min(0.025, dur * 0.15);
    const double rel = std::max(0.08, dur * 0.28);
    DAHDSR env(0.0, atk, 0.0, dur * 0.18, 0.78, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);
    Chorus ch; ch.rate_hz = 0.40; ch.depth_ms = 2.5; ch.delay_ms = 12.0;
    ch.wet = 0.450f; ch.dry = 0.880f; ch.init(sr); ch.process(mono);
    Reverb rv; rv.room_size = 0.600f; rv.wet = 0.220f; rv.dry = 0.920f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.720f * (float)vel_frac);
    return haas_widen(mono, 18.0, sr);
}

StereoBuffer make_scifi_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    Oscillator r1(WaveShape::Sine,     freq,        vel_frac * 0.58);
    Oscillator r2(WaveShape::Triangle, freq * 2.76, vel_frac * 0.22);
    Oscillator r3(WaveShape::Sine,     freq * 4.07, vel_frac * 0.10);
    Oscillator r4(WaveShape::Sine,     freq * 0.5,  vel_frac * 0.12);
    for (size_t i = 0; i < mono.size(); ++i) {
        const double t  = (double)i / sr;
        const float  e1 = static_cast<float>(std::exp(-t * 2.8)  * vel_frac);
        const float  e2 = static_cast<float>(std::exp(-t * 9.0)  * vel_frac);
        const float  e3 = static_cast<float>(std::exp(-t * 14.0) * vel_frac);
        mono[i] = r1.tick(sr)*e1 + r2.tick(sr)*e2 + r3.tick(sr)*e3 + r4.tick(sr)*e1*0.500f;
    }
    Reverb rv; rv.room_size = 0.550f; rv.wet = 0.300f; rv.dry = 0.880f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.700f * (float)vel_frac);
    return haas_widen(mono, 10.0, sr);
}

StereoBuffer make_square_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    const double c_inc = TWO_PI * freq       / sr;
    const double m_inc = TWO_PI * freq * 2.0 / sr;
    double c_phase = 0.0, m_phase = 0.0;
    for (size_t i = 0; i < mono.size(); ++i) {
        const double t   = (double)i / sr;
        const double idx = 0.55 * std::exp(-t * 6.0) + 0.04;
        const double env = std::exp(-t * (2.5 / dur + 0.5)) * vel_frac;
        mono[i] = static_cast<float>(std::sin(c_phase + idx * std::sin(m_phase)) * env);
        c_phase = std::fmod(c_phase + c_inc, TWO_PI);
        m_phase = std::fmod(m_phase + m_inc, TWO_PI);
    }
    Oscillator tine(WaveShape::Sine, freq * 5.02, vel_frac * 0.08);
    for (size_t i = 0; i < mono.size(); ++i) {
        const double t = (double)i / sr;
        mono[i] += tine.tick(sr) * static_cast<float>(std::exp(-t * 18.0));
    }
    const double trem_depth = 0.06;
    double trem_phase = 0.0;
    for (size_t i = 0; i < mono.size(); ++i) {
        mono[i] *= static_cast<float>(1.0 - trem_depth * (0.5 - 0.5 * std::sin(trem_phase)));
        trem_phase += TWO_PI * 5.8 / sr;
    }
    Reverb rv; rv.room_size = 0.450f; rv.wet = 0.180f; rv.dry = 0.950f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.720f * (float)vel_frac);
    return haas_widen(mono, 8.0, sr);
}

StereoBuffer make_metallic_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    const double detunes[] = {0.0, 4.0, -3.5, 7.0, -6.0, 10.5, -9.0, 2.5};
    for (int v = 0; v < 8; ++v) {
        const double f = freq * std::pow(2.0, detunes[v] / 1200.0);
        Oscillator osc(WaveShape::BL_Sawtooth, f, vel_frac * 0.125);
        for (size_t i = 0; i < mono.size(); ++i) mono[i] += osc.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, freq * 4.0, 0.60);
    lp.process(mono, sr);
    const double atk = std::min(0.18, dur * 0.35);
    const double rel = std::max(0.12, dur * 0.30);
    DAHDSR env(0.0, atk, 0.0, std::min(0.20, dur * 0.15), 0.85, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);
    Reverb rv; rv.room_size = 0.800f; rv.wet = 0.350f; rv.dry = 0.880f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.650f * (float)vel_frac);
    return haas_widen(mono, 20.0, sr);
}

StereoBuffer make_bright_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    Oscillator o1(WaveShape::Sine, freq,       vel_frac * 0.60);
    Oscillator o2(WaveShape::Sine, freq * 3.0, vel_frac * 0.15);
    Oscillator o3(WaveShape::Sine, freq * 5.0, vel_frac * 0.06);
    NoiseGenerator ng(uint32_t(freq) * 13u);
    for (size_t i = 0; i < mono.size(); ++i) {
        const double t    = (double)i / sr;
        const double frac = t / dur;
        const float breath = ng.tick_pink() * static_cast<float>(
            (std::exp(-frac * 8.0) * 0.20 + 0.06) * vel_frac);
        mono[i] = o1.tick(sr) + o2.tick(sr) + o3.tick(sr) + breath;
    }
    HighPassFilter hpf(freq * 0.7, sr);
    hpf.process(mono);
    SVFilter lp(SVFilter::Mode::LowPass, freq * 7.0, 0.70);
    lp.process(mono, sr);
    const double atk = std::min(0.025, dur * 0.15);
    const double rel = std::max(0.05, dur * 0.20);
    DAHDSR env(0.0, atk, 0.0, dur * 0.10, 0.80, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);
    Reverb rv; rv.room_size = 0.550f; rv.wet = 0.220f; rv.dry = 0.920f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.600f * (float)vel_frac);
    return haas_widen(mono, 9.0, sr);
}

StereoBuffer make_choir_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    const double freqs[] = {freq, freq * 1.003, freq * 0.997, freq * 2.0, freq * 1.5};
    const double amps[]  = {0.35, 0.28, 0.28, 0.15, 0.10};
    for (int v = 0; v < 5; ++v) {
        Oscillator osc(WaveShape::Sine, freqs[v], vel_frac * amps[v]);
        for (size_t i = 0; i < mono.size(); ++i) mono[i] += osc.tick(sr);
    }
    SVFilter lp(SVFilter::Mode::LowPass, freq * 5.0, 0.65);
    lp.process(mono, sr);
    const double atk = std::min(0.30, dur * 0.45);
    const double rel = std::max(0.10, dur * 0.32);
    DAHDSR env(0.0, atk, 0.0, std::min(0.20, dur * 0.15), 0.88, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);
    Reverb rv; rv.room_size = 0.880f; rv.wet = 0.420f; rv.dry = 0.850f; rv.init(sr);
    rv.process(mono);
    Chorus ch; ch.rate_hz = 0.30; ch.depth_ms = 2.0; ch.delay_ms = 15.0;
    ch.wet = 0.350f; ch.dry = 0.900f; ch.init(sr); ch.process(mono);
    normalize(mono, 0.620f * (float)vel_frac);
    return haas_widen(mono, 22.0, sr);
}

StereoBuffer make_echoes_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    Oscillator o1(WaveShape::Triangle, freq,         vel_frac * 0.50);
    Oscillator o2(WaveShape::Triangle, freq * 1.004, vel_frac * 0.32);
    Oscillator o3(WaveShape::Triangle, freq * 0.996, vel_frac * 0.28);
    Oscillator o4(WaveShape::Sine,     freq * 2.0,   vel_frac * 0.10);
    for (size_t i = 0; i < mono.size(); ++i)
        mono[i] = o1.tick(sr) + o2.tick(sr) + o3.tick(sr) + o4.tick(sr);
    SVFilter lp(SVFilter::Mode::LowPass, 2200.0, 0.65);
    lp.process(mono, sr);
    const double atk = std::min(0.08, dur * 0.25);
    const double rel = std::max(0.10, dur * 0.35);
    DAHDSR env(0.0, atk, 0.0, std::min(0.12, dur * 0.18), 0.82, rel);
    env.apply(mono, dur, std::max(0.0, dur - rel), sr);
    DelayLine dl;
    dl.time_ms  = 375.0;
    dl.feedback = 0.420f; dl.wet = 0.320f; dl.dry = 1.000f; dl.init(sr);
    dl.process(mono);
    Reverb rv; rv.room_size = 0.820f; rv.wet = 0.380f; rv.dry = 0.880f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.620f * (float)vel_frac);
    return haas_widen(mono, 20.0, sr);
}

StereoBuffer make_pad_note(double freq, double dur, double vel_frac, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    Buffer mono = make_buffer(dur, sr);
    const double c_inc = TWO_PI * freq       / sr;
    const double m_inc = TWO_PI * freq * 2.0 / sr;
    double cp = 0.0, mp = 0.0;
    for (size_t i = 0; i < mono.size(); ++i) {
        const double t   = (double)i / sr;
        const double idx = 0.45 * std::exp(-t * 5.0) + 0.03;
        const double env = (1.0 - std::exp(-t * 12.0)) * std::exp(-t * 1.2) * vel_frac;
        mono[i] = static_cast<float>(std::sin(cp + idx * std::sin(mp)) * env * 0.8);
        cp = std::fmod(cp + c_inc, TWO_PI);
        mp = std::fmod(mp + m_inc, TWO_PI);
    }
    double tp = 0.0;
    for (size_t i = 0; i < mono.size(); ++i) {
        mono[i] *= static_cast<float>(1.0 - 0.05 * (0.5 - 0.5 * std::sin(tp)));
        tp += TWO_PI * 4.2 / sr;
    }
    Chorus ch; ch.rate_hz = 0.60; ch.depth_ms = 1.5; ch.delay_ms = 7.0;
    ch.wet = 0.300f; ch.dry = 0.920f; ch.init(sr); ch.process(mono);
    Reverb rv; rv.room_size = 0.520f; rv.wet = 0.200f; rv.dry = 0.950f; rv.init(sr);
    rv.process(mono);
    normalize(mono, 0.650f * (float)vel_frac);
    return haas_widen(mono, 16.0, sr);
}

Buffer make_bass_note(double freq, double dur, int sr = SR) {
    if (dur <= 0.0) return Buffer();
    Buffer buf = make_buffer(dur, sr);
    Oscillator o1(WaveShape::BL_Sawtooth, freq,       0.55);
    Oscillator o2(WaveShape::Sine,        freq * 0.5, 0.30);
    Oscillator o3(WaveShape::Triangle,    freq * 2.0, 0.14);
    for (size_t i = 0; i < buf.size(); ++i)
        buf[i] = o1.tick(sr) + o2.tick(sr) + o3.tick(sr);
    std::vector<AutoPoint> pts = {
        {0.0,        (float)(freq * 1.5)},
        {dur * 0.12, (float)(freq * 4.2)},
        {dur,        (float)(freq * 1.8)},
    };
    buf = apply_filter_automation(buf, pts, 0.70, SVFilter::Mode::LowPass, sr);
    distortion::apply_waveshaper(buf, 0.250f);
    const double rel = std::max(0.025, dur * 0.18);
    DAHDSR env(0.0, 0.008, 0.0, dur * 0.18, 0.84, rel);
    env.apply(buf, dur, std::max(0.0, dur - rel), sr);
    normalize(buf, 0.820f);
    return buf;
}

Buffer make_abass_note(double freq, double dur, int sr = SR) {
    if (dur <= 0.0) return Buffer();
    const double freq_start = freq * 1.35;
    Buffer buf = make_buffer(dur, sr);
    double phase = 0.0;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        const double f = freq + (freq_start - freq) * std::exp(-t * 45.0);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        buf[i] = static_cast<float>(std::sin(phase));
    }
    Oscillator body(WaveShape::Sine, freq * 2.0, 0.18);
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] += body.tick(sr) * static_cast<float>(std::exp(-t * 9.0));
    }
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = (double)i / sr;
        buf[i] *= static_cast<float>((1.0 - std::exp(-t * 50.0)) * std::exp(-t * 4.5));
    }
    SVFilter lp(SVFilter::Mode::LowPass, 700.0, 0.65);
    lp.process(buf, sr);
    normalize(buf, 0.780f);
    return buf;
}

Buffer make_tv_beep(double dur, int sr = SR) {
    if (dur <= 0.0) return Buffer();
    const size_t n = (size_t)((dur + 1.8) * sr);
    Buffer buf(n, 0.000f);
    double pm = 0.0, ph = 0.0;
    for (size_t i = 0; i < n; ++i) {
        const double t = (double)i / sr;
        pm += TWO_PI * notes::B4 / sr; if (pm >= TWO_PI) pm -= TWO_PI;
        ph += TWO_PI * 60.0      / sr; if (ph >= TWO_PI) ph -= TWO_PI;
        buf[i] = static_cast<float>(
            (std::sin(pm) + 0.08 * std::sin(pm * 3.0) + 0.03 * std::sin(pm * 5.0))
            * (1.0 + 0.018 * std::sin(ph)) * (t < dur ? 1.0 : 0.0));
    }
    const size_t ramp = (size_t)(0.002 * sr);
    for (size_t i = 0; i < ramp && i < n; ++i) buf[i] *= (float)i / ramp;
    normalize(buf, 0.780f);
    return buf;
}

StereoBuffer make_square_stab(double dur, int sr = SR) {
    if (dur <= 0.0) return make_stereo(0.001, sr);
    const size_t n = (size_t)(dur * sr);
    NoiseGenerator ng(777u);
    Buffer mono(n, 0.000f);
    for (size_t i = 0; i < n; ++i) mono[i] = ng.tick();
    distortion::apply_hard_clip(mono, 0.080f);
    distortion::apply_waveshaper(mono, 0.999f);
    distortion::apply_hard_clip(mono, 0.150f);
    normalize(mono, 0.950f);
    return haas_widen(mono, 10.0, sr);
}

StereoBuffer make_gliss_down(double freq_start, double freq_end, int sr = SR) {
    const double dur = 0.58;
    Buffer mono = make_buffer(dur, sr);
    double phase = 0.0;
    for (size_t i = 0; i < mono.size(); ++i) {
        const double frac = (double)i / mono.size();
        const double f    = freq_start * std::pow(freq_end / freq_start, frac);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        mono[i] = static_cast<float>(
            std::sin(phase) * std::exp(-frac * 1.8) * (1.0 - std::exp(-frac * 20.0)) * 0.5);
    }
    Reverb rv; rv.room_size = 0.550f; rv.wet = 0.250f; rv.dry = 0.880f; rv.init(sr);
    rv.process(mono);
    return haas_widen(mono, 12.0, sr);
}

StereoBuffer make_gliss_up(double dur, double freq_start, double freq_end, int sr = SR) {
    Buffer mono = make_buffer(dur, sr);
    double phase = 0.0;
    for (size_t i = 0; i < mono.size(); ++i) {
        const double frac = (double)i / mono.size();
        const double f    = freq_start * std::pow(freq_end / freq_start, frac);
        phase += TWO_PI * f / sr;
        if (phase >= TWO_PI) phase -= TWO_PI;
        mono[i] = static_cast<float>(std::sin(phase) * 0.45);
    }
    Reverb rv; rv.room_size = 0.550f; rv.wet = 0.250f; rv.dry = 0.880f; rv.init(sr);
    rv.process(mono);
    return haas_widen(mono, 12.0, sr);
}

static void place_mono(StereoBuffer& mix, const Buffer& src, double t, float gain) {
    const size_t off = (size_t)(t * SR);
    for (size_t i = 0; i < src.size() && off + i < mix.size(); ++i) {
        mix.L[off+i] += src[i] * gain;
        mix.R[off+i] += src[i] * gain;
    }
}

static void place_stereo(StereoBuffer& mix, const StereoBuffer& src, double t, float gain) {
    const size_t off = (size_t)(t * SR);
    for (size_t i = 0; i < src.size() && off + i < mix.size(); ++i) {
        mix.L[off+i] += src.L[i] * gain;
        mix.R[off+i] += src.R[i] * gain;
    }
}

int main(int argc, char** argv) {
    std::string output  = "society.wav";
    bool        do_play = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if ((a == "-o" || a == "--output") && i + 1 < argc) output = argv[++i];
        else if (a == "--play") do_play = true;
    }

    global_config().sample_rate = SR;
    global_config().channels    = 2;
    global_config().bits        = 16;

    const size_t TOTAL = (size_t)(136.0 * SR);


    const Buffer kick      = make_kick();
    const Buffer chat      = make_chat();
    const Buffer ohat      = make_ohat();
    const Buffer ride      = make_ride();
    const Buffer snare     = make_snare();
    const Buffer chat_ping = make_chat_ping();
    const Buffer ride_ping = make_ride_ping();

    StereoBuffer mix(TOTAL, 0.000f);

    for (int i = 0; i < hat_count; ++i) {
        place_mono(mix, chat,      hat_times[i], 0.200f);
        place_mono(mix, chat_ping, hat_times[i], 0.110f);
    }
    for (int i = 0; i < ride_count; ++i) {
        place_mono(mix, ride,      ride_times[i], 0.280f);
        place_mono(mix, ride_ping, ride_times[i], 0.150f);
    }
    for (int i = 0; i < snare_count;    ++i) place_mono(mix, snare, snare_times[i],    0.580f);
    for (int i = 0; i < hat_fill_count; ++i) place_mono(mix, chat,  hat_fill_times[i], 0.160f);
    for (int i = 0; i < clap_count;     ++i) place_mono(mix, snare, clap_times[i],     0.500f);
    for (double t = 16.0; t < 32.0; t += 0.5) place_mono(mix, kick, t, 0.800f);
    for (double t = 48.0; t < 80.0; t += 0.5) place_mono(mix, kick, t, 0.800f);
    for (int i = 0; i < shaker_count; ++i) {
        Buffer shk = make_shaker(shaker_notes[i].dur);
        place_mono(mix, shk, shaker_notes[i].t, 0.320f);
    }
    for (int i = 0; i < tv_beep_count; ++i) {
        Buffer beep = make_tv_beep(0.5);
        place_mono(mix, beep, tv_beep_times[i], 0.900f);
    }

    { double t = 0.25; int ri = 0;
      while (t < 14.0) { Buffer bn = make_bass_note(bass_roots[ri%4], 0.249); place_mono(mix, bn, t, 0.420f); t += 0.5; if (std::fmod(t, 2.0) < 0.001) ri++; } }
    { double t = 16.0; int ri = 0;
      while (t < 32.0) { double root = bass_roots[ri%4]; double freq = (((int)((t-16.0)/0.25))%2==0) ? root : root*2.0; Buffer bn = make_bass_note(freq, 0.249); place_mono(mix, bn, t, 0.500f); t += 0.25; if (std::fmod(t-16.0, 2.0) < 0.001) ri++; } }
    { double t = 32.0; int ri = 0;
      while (t < 46.5) { double root = bass_roots[ri%4]; double freq = (((int)((t-32.0)/0.125))%2==0) ? root : root*2.0; Buffer bn = make_bass_note(freq, 0.124); place_mono(mix, bn, t, 0.440f); t += 0.125; if (std::fmod(t-32.0, 2.0) < 0.001) ri++; } }
    { double t = 48.0; int ri = 0;
      while (t < 80.0) { double root = bass_roots[ri%4]; double freq = (((int)((t-48.0)/0.25))%2==0) ? root : root*2.0; Buffer bn = make_bass_note(freq, 0.249); place_mono(mix, bn, t, 0.500f); t += 0.25; if (std::fmod(t-48.0, 2.0) < 0.001) ri++; } }
    for (int i = 0; i < abass_end2_count; ++i) { Buffer bn = make_bass_note(abass_end2[i].freq, abass_end2[i].dur); place_mono(mix, bn, abass_end2[i].t, 0.280f); }
    { const double longs[] = {notes::D2,notes::C2,notes::As1,notes::F1,notes::D2,notes::C2,notes::As1,notes::F1};
      for (int i = 0; i < 8; ++i) { Buffer bn = make_bass_note(longs[i], 1.999); place_mono(mix, bn, 101.0 + i*2.0, 0.340f); } }
    for (int i = 0; i < bass_late_count; ++i) { Buffer bn = make_bass_note(bass_late[i].freq, bass_late[i].dur); place_mono(mix, bn, bass_late[i].t, 0.400f); }

    { double t = 0.25; int ri = 0;
      while (t < 14.0) { Buffer bn = make_abass_note(bass_roots[ri%4], 0.249); place_mono(mix, bn, t, 0.340f); t += 0.5; if (std::fmod(t, 2.0) < 0.001) ri++; } }
    { double t = 16.0; int ri = 0;
      while (t < 80.0) { double root = bass_roots[ri%4]; double step = (t < 32.0) ? 0.25 : (t < 48.0 ? 0.125 : 0.25); double freq = (((int)((t-16.0)/step))%2==0) ? root : root*2.0; Buffer bn = make_abass_note(freq, step*0.99); place_mono(mix, bn, t, 0.360f); t += step; if (std::fmod(t-16.0, 2.0) < 0.001) ri++; } }
    for (int i = 0; i < abass_end_count; ++i) { Buffer bn = make_abass_note(abass_end[i].freq, abass_end[i].dur); place_mono(mix, bn, abass_end[i].t, 0.280f); }

    { double t = 0.0;
      for (int rep = 0; rep < 4; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn = make_sine_note(sine_pattern[n], 0.499, (rep==0&&n==0)?1.0:0.8);
        place_stereo(mix, sn, t, 0.280f); t += 0.5;
      } }
    { double t = 32.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 16; ++n) {
        auto s1 = make_sine_note(sine_pattern[n],       0.124,  0.8);
        auto s2 = make_sine_note(sine_pattern[n],       0.124,  0.8);
        auto s3 = make_sine_note(sine_pattern[n] * 2.0, 0.0615, 0.7);
        auto s4 = make_sine_note(sine_pattern[n] * 4.0, 0.0615, 0.6);
        place_stereo(mix, s1, t,        0.240f);
        place_stereo(mix, s2, t+0.125,  0.200f);
        place_stereo(mix, s3, t+0.25,   0.140f);
        place_stereo(mix, s4, t+0.3125, 0.100f);
        t += 0.5;
      } }
    { auto gl = make_gliss_down(notes::E4, notes::C2); place_stereo(mix, gl, 47.17, 0.380f); }
    { double t = 48.0;
      for (int rep = 0; rep < 4; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn = make_sine_note(sine_pattern[n], 0.499, 1.0);
        place_stereo(mix, sn, t, 0.300f); t += 0.5;
      } }
    { double t = 80.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn = make_sine_note(sine_pattern_2[n], 0.499, 1.0);
        place_stereo(mix, sn, t, 0.400f); t += 0.5;
      } }
    { double t = 101.0;
      for (int rep = 0; rep < 3; ++rep) for (int n = 0; n < 16; ++n) {
        const double dur = (rep==2&&n==15) ? 2.999 : 0.499;
        auto sn = make_sine_note(phase3_melody[n], dur, 0.8);
        place_stereo(mix, sn, t, 0.340f); t += (rep==2&&n==15) ? 3.0 : 0.5;
      } }

    { double t = 16.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn = make_saw_note(sine_pattern[n], 0.499, 1.0);
        place_stereo(mix, sn, t, 0.320f); t += 0.5;
      } }
    { double t = 40.0;
      const double freqs[] = {notes::C4,notes::E4,notes::A4,notes::C4,notes::E4,notes::G4,notes::A3,notes::C4,notes::A3};
      const double durs[]  = {1.0,0.5,0.5,1.0,0.5,0.5,1.0,0.5,0.5};
      for (int n = 0; n < 9; ++n) { auto sn = make_saw_note(freqs[n], durs[n]*0.99, 1.0); place_stereo(mix, sn, t, 0.340f); t += durs[n]; } }
    { double t = 48.0;
      for (int rep = 0; rep < 4; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn  = make_saw_note(sine_pattern[n],       0.499, 1.0);
        auto snp = make_saw_note(sine_pattern[n] * 2.0, 0.499, 0.9);
        place_stereo(mix, sn,  t, 0.300f);
        place_stereo(mix, snp, t, 0.180f);
        t += 0.5;
      } }
    { auto gl = make_gliss_down(notes::Ds4, notes::B1); place_stereo(mix, gl, 94.0, 0.420f); }

    { double t = 16.0;
      for (int rep = 0; rep < 4; ++rep) for (int n = 0; n < 32; ++n) {
        auto sn = make_scifi_note(scifi_hi[n%32], 0.249, 0.8);
        place_stereo(mix, sn, t, 0.200f); t += 0.25;
      } }
    { double t = 32.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 32; ++n) {
        const double step = (n%4 < 2) ? 0.124 : 0.0615;
        auto sn = make_scifi_note(scifi_hi[n%32], step*0.99, (n%4 < 2) ? 0.8 : 0.6);
        place_stereo(mix, sn, t, 0.170f); t += step;
      } }
    { auto gl = make_gliss_down(notes::Ds4, notes::B1); place_stereo(mix, gl, 94.0, 0.350f); }
    { double t = 101.0;
      for (int rep = 0; rep < 3; ++rep) for (int n = 0; n < 16; ++n) {
        const double dur = (rep==2&&n==15) ? 2.999 : 0.499;
        auto sn = make_scifi_note(phase3_melody[n], dur, 0.8);
        place_stereo(mix, sn, t, 0.220f); t += (rep==2&&n==15) ? 3.0 : 0.5;
      } }

    { auto sn = make_square_note(notes::D6, 0.499, 0.8);
      place_stereo(mix, sn, 15.5, 0.240f);
      place_stereo(mix, sn, 27.5, 0.240f); }
    { double t = 40.0;
      const double freqs[] = {notes::C4,notes::E4,notes::A4,notes::C4,notes::E4,notes::G4,notes::A3,notes::C4,notes::A3};
      const double durs[]  = {1.0,0.5,0.5,1.0,0.5,0.5,1.0,0.5,0.5};
      for (int n = 0; n < 9; ++n) { auto sn = make_square_note(freqs[n], durs[n]*0.99, 0.8); place_stereo(mix, sn, t, 0.280f); t += durs[n]; } }
    { auto gl  = make_gliss_up(2.0, notes::Ds4, notes::D6);
      auto hit = make_square_note(notes::D6, 0.499, 1.0);
      auto sb1 = make_square_stab(0.010);
      auto sb2 = make_square_stab(0.010);
      place_stereo(mix, gl,  96.0,  0.360f);
      place_stereo(mix, hit, 95.5,  0.320f);
      place_stereo(mix, sb1, 98.25, 0.720f);
      place_stereo(mix, sb2, 100.0, 0.680f); }
    { double t = 101.0;
      for (int rep = 0; rep < 3; ++rep) for (int n = 0; n < 16; ++n) {
        const double dur = (rep==2&&n==15) ? 2.999 : 0.499;
        auto sn = make_square_note(phase3_melody[n], dur, 0.6);
        place_stereo(mix, sn, t, 0.180f); t += (rep==2&&n==15) ? 3.0 : 0.5;
      } }

    for (int i = 0; i < 8; ++i) {
        auto sn = make_metallic_note(metal_intro[i], 1.999, 33.0/127.0);
        place_stereo(mix, sn, i * 2.0, 0.220f);
    }
    { double t = 16.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn = make_metallic_note(sine_pattern[n], 0.499, 1.0);
        place_stereo(mix, sn, t, 0.280f); t += 0.5;
      } }
    { double t = 48.0;
      for (int rep = 0; rep < 4; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn  = make_metallic_note(sine_pattern[n],       0.499, 1.0);
        auto sn2 = make_metallic_note(sine_pattern[n] * 2.0, 0.499, 0.8);
        place_stereo(mix, sn,  t, 0.240f);
        place_stereo(mix, sn2, t, 0.120f);
        t += 0.5;
      } }
    { double t = 80.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn = make_metallic_note(metal_high[n], 0.499, 1.0);
        place_stereo(mix, sn, t, 0.340f); t += 0.5;
      } }

    { double t = 16.5;
      for (int i = 0; i < 16; ++i) {
        auto sn = make_bright_note(bright_melody[i], 0.499, 0.8);
        place_stereo(mix, sn, t, 0.240f); t += 1.0;
      } }

    { double t = 32.0;
      for (int rep = 0; rep < 2; ++rep) for (int n = 0; n < 16; ++n) {
        const double base = choir_base[n];
        auto s1 = make_choir_note(base,       0.124,  0.8);
        auto s2 = make_choir_note(base,       0.124,  0.8);
        auto s3 = make_choir_note(base * 2.0, 0.0615, 0.7);
        auto s4 = make_choir_note(base * 4.0, 0.0615, 0.6);
        place_stereo(mix, s1, t,        0.180f);
        place_stereo(mix, s2, t+0.125,  0.180f);
        place_stereo(mix, s3, t+0.25,   0.140f);
        place_stereo(mix, s4, t+0.3125, 0.100f);
        t += 0.5;
      }
      auto bg = make_choir_note(notes::E4, 4.0, 0.5);
      place_stereo(mix, bg, 44.0, 0.140f); }

    { double t = 48.0;
      for (int i = 0; i < 24; ++i) {
        for (int v = 0; v < 3; ++v) {
            auto pn = make_pad_note(piano_chords[i%4][v], 1.999, 0.8);
            place_stereo(mix, pn, t, 0.300f);
        }
        t += 2.0;
      } }

    { double t = 48.0;
      for (int rep = 0; rep < 4; ++rep) for (int n = 0; n < 16; ++n) {
        auto sn  = make_echoes_note(echoes_pat[n],       0.499, 0.96);
        auto sn2 = make_echoes_note(echoes_pat[n] * 2.0, 0.499, 0.96);
        place_stereo(mix, sn,  t, 0.240f);
        place_stereo(mix, sn2, t, 0.180f);
        t += 0.5;
      }
      auto bg = make_echoes_note(notes::E4, 4.0, 0.5);
      place_stereo(mix, bg, 44.0, 0.120f); }

    distortion::apply_soft_clip(mix.L, 1.100f);
    distortion::apply_soft_clip(mix.R, 1.100f);
    fade_in(mix,  0.25, SR);
    fade_out(mix, 4.0,  SR);
    normalize(mix, 0.900f);
    clamp_buffer(mix);

    save_wav_stereo(mix, output, SR);

    if (do_play) {
        PlaybackHandle h = play_async(mix, SR);
        std::thread([h]() mutable { std::cin.get(); h.stop(); }).detach();
        h.wait();
    }
    return 0;
}