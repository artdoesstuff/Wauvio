// =============================================================================
//  W A U V I O . H P P  -  Advanced Audio Synthesis Engine  v1.3.4
//  Header-only C++ audio synthesis library
//
//  Refer to https://github.com/artdoesstuff/Wauvio/blob/main/worthy if examples are needed.
//
//  §1   Constants
//  §2   Global Configuration
//  §3   Buffer (mono + stereo)
//  §4   Waveform Primitives
//  §5   WaveShape enum
//  §6   Oscillator  (phase-accurate, unison, hard-sync)
//  §7   Supersaw Helper  (7-voice detuned stack)
//  §8   Noise Generator
//  §9   ADSR Envelope
//  §10  AHDSR / DAHDSR Multi-stage Envelope
//  §11  Loopable Envelope
//  §12  LFO System  (sine, tri, square, S&H; pitch/amp/filter/pan targets)
//  §13  State Variable Filter  (LP/HP/BP + resonance Q)
//  §14  Legacy 1-pole filters  (kept for compatibility)
//  §15  FM Synthesis
//  §16  Distortion  (soft clip, hard clip, waveshaper)
//  §17  Delay Line  (feedback, wet/dry)
//  §18  Reverb  (Schroeder comb+allpass)
//  §19  Chorus  (modulated delay)
//  §20  Stereo Buffer + Panning utilities
//  §21  Sidechain Compressor
//  §22  Automation  (parameter-over-time curves)
//  §23  Polyphonic Voice System
//  §24  Music Theory Helpers (notes, midi_to_freq)
//  §25  Note / Instrument / Track / Sequencer
//  §26  Drum Sequencer
//  §27  Master Bus
//  §28  WAV Export  (mono + stereo)
//  §29  Playback  (concurrent, non-blocking, thread-safe runtime)
//
//  Compile:
//    Linux  : g++ -std=c++17 -O2 your_program.cpp -o your_program
//    Windows: cl /std:c++17 /O2 your_program.cpp /link winmm.lib
// =============================================================================

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#  define WAUVIO_WINDOWS
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <mmsystem.h>
#else
#  define WAUVIO_LINUX
#  include <cstdio>
#endif

namespace wauvio {

// =============================================================================
//  §1  CONSTANTS
// =============================================================================

constexpr double TWO_PI = 6.283185307179586476925286766559;
constexpr double PI     = 3.141592653589793238462643383279;

// =============================================================================
//  §2  GLOBAL CONFIGURATION
// =============================================================================

struct Config {
    int sample_rate = 44100;
    int channels    = 1;
    int bits        = 16;
};

inline Config& global_config() {
    static Config cfg;
    return cfg;
}

// =============================================================================
//  §3  BUFFER SYSTEM
//  Mono: std::vector<float>
//  Stereo: StereoBuffer - pair of equal-length mono buffers (L, R)
// =============================================================================

using Buffer = std::vector<float>;

struct StereoBuffer {
    Buffer L, R;

    StereoBuffer() = default;
    StereoBuffer(size_t n, float val = 0.0f) : L(n, val), R(n, val) {}

    size_t size() const noexcept { return L.size(); }
    bool   empty() const noexcept { return L.empty(); }

    void resize(size_t n, float val = 0.0f) { L.assign(n, val); R.assign(n, val); }

    /// Collapse to mono by averaging L+R.
    Buffer to_mono() const {
        Buffer out(L.size());
        for (size_t i = 0; i < out.size(); ++i)
            out[i] = (L[i] + R[i]) * 0.5f;
        return out;
    }

    /// Interleave L/R into LRLRLR… PCM for stereo WAV output.
    Buffer interleave() const {
        Buffer out;
        out.reserve(L.size() * 2);
        for (size_t i = 0; i < L.size(); ++i) {
            out.push_back(L[i]);
            out.push_back(R[i]);
        }
        return out;
    }
};

// ---------------------------------------------------------------------------
//  Mono buffer helpers
// ---------------------------------------------------------------------------

inline Buffer make_buffer(double seconds, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    return Buffer(static_cast<size_t>(seconds * sample_rate), 0.0f);
}

inline StereoBuffer make_stereo(double seconds, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    return StereoBuffer(static_cast<size_t>(seconds * sample_rate), 0.0f);
}

inline void mix_into(Buffer& dst, const Buffer& src, float gain = 1.0f) {
    size_t n = std::min(dst.size(), src.size());
    for (size_t i = 0; i < n; ++i) dst[i] += src[i] * gain;
}

inline void mix_into(StereoBuffer& dst, const StereoBuffer& src, float gain = 1.0f) {
    size_t n = std::min(dst.size(), src.size());
    for (size_t i = 0; i < n; ++i) {
        dst.L[i] += src.L[i] * gain;
        dst.R[i] += src.R[i] * gain;
    }
}

inline void apply_gain(Buffer& buf, float gain) {
    for (auto& s : buf) s *= gain;
}

inline void apply_gain(StereoBuffer& buf, float gain) {
    for (auto& s : buf.L) s *= gain;
    for (auto& s : buf.R) s *= gain;
}

inline void clamp_buffer(Buffer& buf) {
    for (auto& s : buf) s = std::max(-1.0f, std::min(1.0f, s));
}

inline void clamp_buffer(StereoBuffer& buf) {
    for (auto& s : buf.L) s = std::max(-1.0f, std::min(1.0f, s));
    for (auto& s : buf.R) s = std::max(-1.0f, std::min(1.0f, s));
}

inline void normalize(Buffer& buf, float target = 0.9f) {
    float peak = 0.0f;
    for (const auto& s : buf) peak = std::max(peak, std::abs(s));
    if (peak > 1e-7f) { const float inv = target / peak; for (auto& s : buf) s *= inv; }
}

inline void normalize(StereoBuffer& buf, float target = 0.9f) {
    float peak = 0.0f;
    for (const auto& s : buf.L) peak = std::max(peak, std::abs(s));
    for (const auto& s : buf.R) peak = std::max(peak, std::abs(s));
    if (peak > 1e-7f) {
        const float inv = target / peak;
        for (auto& s : buf.L) s *= inv;
        for (auto& s : buf.R) s *= inv;
    }
}

inline void fade_in(Buffer& buf, double fade_seconds, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
    for (size_t i = 0; i < n; ++i)
        buf[i] *= static_cast<float>(i) / static_cast<float>(n);
}

inline void fade_out(Buffer& buf, double fade_seconds, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
    for (size_t i = 0; i < n; ++i)
        buf[buf.size() - 1 - i] *= static_cast<float>(i) / static_cast<float>(n);
}

inline void fade_in(StereoBuffer& buf, double fade_seconds, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
    for (size_t i = 0; i < n; ++i) {
        float f = static_cast<float>(i) / static_cast<float>(n);
        buf.L[i] *= f;  buf.R[i] *= f;
    }
}

inline void fade_out(StereoBuffer& buf, double fade_seconds, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
    for (size_t i = 0; i < n; ++i) {
        float f = static_cast<float>(i) / static_cast<float>(n);
        buf.L[buf.size()-1-i] *= f;
        buf.R[buf.size()-1-i] *= f;
    }
}

inline std::vector<int16_t> to_pcm16(const Buffer& buf) {
    std::vector<int16_t> out;
    out.reserve(buf.size());
    for (float s : buf)
        out.push_back(static_cast<int16_t>(std::max(-1.0f,std::min(1.0f,s)) * 32767.0f));
    return out;
}

// =============================================================================
//  §4  WAVEFORM PRIMITIVES  (stateless, phase in [0,2π))
// =============================================================================

namespace wave {

inline float sine    (double p) noexcept { return static_cast<float>(std::sin(p)); }
inline float square  (double p) noexcept { return (std::fmod(p,TWO_PI) < PI) ? 1.f : -1.f; }
inline float triangle(double p) noexcept {
    double t = std::fmod(p / TWO_PI, 1.0);
    return static_cast<float>(t < 0.5 ? 4.0*t - 1.0 : 3.0 - 4.0*t);
}
inline float sawtooth(double p) noexcept {
    return static_cast<float>(2.0 * std::fmod(p / TWO_PI, 1.0) - 1.0);
}
inline float pulse(double p, double duty = 0.5) noexcept {
    return (std::fmod(p, TWO_PI) < TWO_PI * duty) ? 1.f : -1.f;
}

} // namespace wave

// =============================================================================
//  §5  WAVESHAPE ENUM
// =============================================================================

enum class WaveShape { Sine, Square, Triangle, Sawtooth, Pulse };

// =============================================================================
//  §6  OSCILLATOR
//  Phase-accumulating oscillator with hard-sync support.
//  Hard sync: when sync_source phase resets, this oscillator resets too.
// =============================================================================

class Oscillator {
public:
    WaveShape shape        = WaveShape::Sine;
    double    frequency    = 440.0;
    double    amplitude    = 1.0;
    double    phase_offset = 0.0;
    double    duty         = 0.5;
    bool      hard_sync    = false;   ///< If true, caller must call sync_reset() at slave reset

    Oscillator() = default;
    explicit Oscillator(WaveShape s, double f = 440.0, double a = 1.0)
        : shape(s), frequency(f), amplitude(a) {}

    float tick(int sample_rate) noexcept {
        phase_ += TWO_PI * frequency / static_cast<double>(sample_rate);
        if (phase_ >= TWO_PI) phase_ -= TWO_PI;
        float s = sample_at(phase_ + phase_offset);
        return s * static_cast<float>(amplitude);
    }

    /// Hard sync: reset phase when master crosses zero upward.
    void sync_reset() noexcept { phase_ = 0.0; }

    void  reset()          noexcept { phase_ = 0.0; }
    double phase()   const noexcept { return phase_; }
    void   set_phase(double p) noexcept { phase_ = p; }

    Buffer render(double seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        Buffer buf(static_cast<size_t>(seconds * sample_rate));
        for (auto& s : buf) s = tick(sample_rate);
        return buf;
    }

private:
    double phase_ = 0.0;

    float sample_at(double p) const noexcept {
        switch (shape) {
            case WaveShape::Sine:     return wave::sine(p);
            case WaveShape::Square:   return wave::square(p);
            case WaveShape::Triangle: return wave::triangle(p);
            case WaveShape::Sawtooth: return wave::sawtooth(p);
            case WaveShape::Pulse:    return wave::pulse(p, duty);
        }
        return 0.f;
    }
};

// =============================================================================
//  §7  SUPERSAW HELPER
//  Renders N detuned sawtooth oscillators into a mono buffer.
//  Classic JP-8000 supersaw: 7 voices, center at 0 Hz detune, outer voices
//  spread symmetrically up to ±detune_cents cents.
// =============================================================================

class Supersaw {
public:
    int    voices       = 7;      ///< Number of oscillator voices
    double frequency    = 440.0;
    double detune_cents = 12.0;   ///< Total spread in cents (split across voices)
    double amplitude    = 1.0;
    double mix_center   = 0.7;    ///< Gain of centre voice relative to side voices

    /// Render `seconds` of supersaw audio (mono).
    Buffer render(double seconds, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t n = static_cast<size_t>(seconds * sample_rate);
        Buffer out(n, 0.0f);

        const int    v     = std::max(1, voices);
        const double span  = detune_cents / 100.0 / 12.0; // semitones
        const double step  = (v > 1) ? 2.0 * span / (v - 1) : 0.0;

        for (int i = 0; i < v; ++i) {
            const double detune_st = -span + i * step;
            const double freq      = frequency * std::pow(2.0, detune_st);
            const double gain      = (i == v / 2) ? mix_center : (1.0 - mix_center) / (v - 1);

            double phase = 0.0;
            const double inc = TWO_PI * freq / sample_rate;
            for (size_t s = 0; s < n; ++s) {
                out[s] += static_cast<float>(wave::sawtooth(phase) * gain * amplitude);
                phase += inc;
            }
        }
        return out;
    }

    /// Render into a stereo buffer with panning spread across voices.
    StereoBuffer render_stereo(double seconds, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t n = static_cast<size_t>(seconds * sample_rate);
        StereoBuffer out(n, 0.0f);

        const int    v    = std::max(1, voices);
        const double span = detune_cents / 100.0 / 12.0;
        const double step = (v > 1) ? 2.0 * span / (v - 1) : 0.0;

        for (int i = 0; i < v; ++i) {
            const double detune_st = -span + i * step;
            const double freq      = frequency * std::pow(2.0, detune_st);
            const double gain      = (i == v / 2)
                ? mix_center : (1.0 - mix_center) / (v - 1);

            // Pan: spread voices from full left to full right
            const double pan  = (v > 1) ? static_cast<double>(i) / (v - 1) : 0.5;
            const float  gainL = static_cast<float>(gain * std::cos(pan * PI * 0.5));
            const float  gainR = static_cast<float>(gain * std::sin(pan * PI * 0.5));

            double phase = 0.0;
            const double inc = TWO_PI * freq / sample_rate;
            for (size_t s = 0; s < n; ++s) {
                const float samp = static_cast<float>(wave::sawtooth(phase) * amplitude);
                out.L[s] += samp * gainL;
                out.R[s] += samp * gainR;
                phase += inc;
            }
        }
        return out;
    }
};

// =============================================================================
//  §8  NOISE GENERATOR
// =============================================================================

class NoiseGenerator {
public:
    explicit NoiseGenerator(uint32_t seed = 42u)
        : rng_(seed), dist_(-1.0f, 1.0f) {}

    float  tick()  { return dist_(rng_); }
    void   seed(uint32_t s) { rng_.seed(s); }

    Buffer render(double seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        Buffer buf(static_cast<size_t>(seconds * sample_rate));
        for (auto& s : buf) s = dist_(rng_);
        return buf;
    }

private:
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;
};

// =============================================================================
//  §9  ADSR ENVELOPE  (unchanged from v1, retained for compatibility)
// =============================================================================

class ADSR {
public:
    double attack  = 0.01;
    double decay   = 0.05;
    double sustain = 0.7;
    double release = 0.1;

    ADSR() = default;
    ADSR(double a, double d, double s, double r)
        : attack(a), decay(d), sustain(s), release(r) {}

    void apply(Buffer& buf, double total_seconds,
               double note_off = -1.0, int sample_rate = 0) const
    {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const double sr = static_cast<double>(sample_rate);
        const size_t n  = buf.size();

        const size_t a_end   = static_cast<size_t>(attack * sr);
        const size_t d_end   = a_end + static_cast<size_t>(decay * sr);
        if (note_off < 0.0) note_off = total_seconds - release;
        note_off = std::max(0.0, note_off);
        const size_t r_start = static_cast<size_t>(note_off * sr);
        const size_t r_end   = r_start + static_cast<size_t>(release * sr);

        for (size_t i = 0; i < n; ++i) {
            double env;
            if      (i < a_end)   env = a_end > 0 ? (double)i / a_end : 1.0;
            else if (i < d_end)   env = 1.0 - (double)(i-a_end)/(d_end-a_end) * (1.0-sustain);
            else if (i < r_start) env = sustain;
            else if (i < r_end)   env = sustain * (1.0 - (double)(i-r_start)/(r_end-r_start+1));
            else                  env = 0.0;
            buf[i] *= static_cast<float>(env);
        }
    }

    /// Sample-by-sample stateful version (for voice systems).
    struct State {
        enum Phase { ATTACK, DECAY, SUSTAIN, RELEASE, DONE } phase = ATTACK;
        double value    = 0.0;
        double note_off = -1.0;  ///< Call note_off() to trigger release
        size_t sample   = 0;

        void note_off_trigger() { phase = RELEASE; }
    };

    float tick(State& st, int sample_rate) const noexcept {
        const double sr     = static_cast<double>(sample_rate);
        const double a_rate = (attack  > 0) ? 1.0 / (attack  * sr) : 1.0;
        const double d_rate = (decay   > 0) ? 1.0 / (decay   * sr) : 1.0;
        const double r_rate = (release > 0) ? 1.0 / (release * sr) : 1.0;

        switch (st.phase) {
            case State::ATTACK:
                st.value += a_rate;
                if (st.value >= 1.0) { st.value = 1.0; st.phase = State::DECAY; }
                break;
            case State::DECAY:
                st.value -= d_rate * (1.0 - sustain);
                if (st.value <= sustain) { st.value = sustain; st.phase = State::SUSTAIN; }
                break;
            case State::SUSTAIN:
                st.value = sustain;
                break;
            case State::RELEASE:
                st.value -= r_rate * sustain;
                if (st.value <= 0.0) { st.value = 0.0; st.phase = State::DONE; }
                break;
            case State::DONE:
                st.value = 0.0;
                break;
        }
        return static_cast<float>(st.value);
    }
};

// =============================================================================
//  §10  AHDSR / DAHDSR MULTI-STAGE ENVELOPE
//  Hold stage between attack and decay.
//  Optional pre-delay (D = initial delay before attack).
// =============================================================================

class DAHDSR {
public:
    double pre_delay = 0.0;   ///< D: seconds of silence before attack
    double attack    = 0.01;
    double hold      = 0.0;   ///< H: seconds at peak before decay
    double decay     = 0.05;
    double sustain   = 0.7;
    double release   = 0.1;

    DAHDSR() = default;
    DAHDSR(double d, double a, double h, double dec, double s, double r)
        : pre_delay(d), attack(a), hold(h), decay(dec), sustain(s), release(r) {}

    void apply(Buffer& buf, double total_seconds,
               double note_off = -1.0, int sample_rate = 0) const
    {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const double sr = sample_rate;
        const size_t n  = buf.size();
        if (note_off < 0.0) note_off = total_seconds - release;
        note_off = std::max(0.0, note_off);

        const size_t d_end   = static_cast<size_t>(pre_delay * sr);
        const size_t a_end   = d_end + static_cast<size_t>(attack * sr);
        const size_t h_end   = a_end + static_cast<size_t>(hold * sr);
        const size_t dec_end = h_end + static_cast<size_t>(decay * sr);
        const size_t r_start = static_cast<size_t>(note_off * sr);
        const size_t r_end   = r_start + static_cast<size_t>(release * sr);

        for (size_t i = 0; i < n; ++i) {
            double env;
            if      (i < d_end)   env = 0.0;
            else if (i < a_end)   env = a_end > d_end ? (double)(i-d_end)/(a_end-d_end) : 1.0;
            else if (i < h_end)   env = 1.0;
            else if (i < dec_end) env = 1.0 - (double)(i-h_end)/(dec_end-h_end)*(1.0-sustain);
            else if (i < r_start) env = sustain;
            else if (i < r_end)   env = sustain*(1.0-(double)(i-r_start)/(r_end-r_start+1));
            else                  env = 0.0;
            buf[i] *= static_cast<float>(env);
        }
    }
};

// =============================================================================
//  §11  LOOPABLE ENVELOPE
//  Loops the attack→decay→sustain cycle indefinitely - behaves like a
//  slow LFO with a shaped waveform.
// =============================================================================

class LoopableEnvelope {
public:
    double attack  = 0.1;
    double decay   = 0.2;
    double sustain = 0.5;   ///< Level held between decay and next attack
    bool   loop    = true;

    struct State {
        enum Phase { ATT, DEC, SUS } phase = ATT;
        double value = 0.0;
    };

    float tick(State& st, int sample_rate) const noexcept {
        const double sr     = sample_rate;
        const double a_rate = attack > 0 ? 1.0 / (attack * sr) : 1.0;
        const double d_rate = decay  > 0 ? 1.0 / (decay  * sr) : 1.0;

        switch (st.phase) {
            case State::ATT:
                st.value += a_rate;
                if (st.value >= 1.0) { st.value = 1.0; st.phase = State::DEC; }
                break;
            case State::DEC:
                st.value -= d_rate * (1.0 - sustain);
                if (st.value <= sustain) {
                    st.value = sustain;
                    st.phase = loop ? State::ATT : State::SUS;
                }
                break;
            case State::SUS:
                break;
        }
        return static_cast<float>(st.value);
    }
};

// =============================================================================
//  §12  LFO SYSTEM
//  Can target: pitch (freq multiplier), amplitude, filter cutoff, pan.
// =============================================================================

enum class LFOShape { Sine, Triangle, Square, SampleHold };
enum class LFOTarget { Pitch, Amplitude, FilterCutoff, Pan };

class LFO {
public:
    LFOShape  shape     = LFOShape::Sine;
    LFOTarget target    = LFOTarget::Amplitude;
    double    rate_hz   = 4.0;    ///< Oscillation frequency
    double    depth     = 0.5;    ///< Modulation depth [0,1]
    double    phase_off = 0.0;    ///< Phase offset (radians)

    LFO() = default;
    LFO(LFOShape sh, LFOTarget tgt, double rate, double depth_)
        : shape(sh), target(tgt), rate_hz(rate), depth(depth_) {}

    /// Returns the current LFO value in [-1, 1].
    float tick(int sample_rate) noexcept {
        phase_ += TWO_PI * rate_hz / static_cast<double>(sample_rate);
        if (phase_ >= TWO_PI) phase_ -= TWO_PI;

        const double p = phase_ + phase_off;
        switch (shape) {
            case LFOShape::Sine:       return static_cast<float>(std::sin(p));
            case LFOShape::Triangle:   return wave::triangle(p);
            case LFOShape::Square:     return wave::square(p);
            case LFOShape::SampleHold:
                // New random value on each zero crossing from below
                if (phase_ < prev_phase_) {
                    sh_value_ = dist_(rng_) * 2.0f - 1.0f;
                }
                prev_phase_ = static_cast<float>(phase_);
                return sh_value_;
        }
        return 0.f;
    }

    /// Returns depth-scaled modulation value.
    float modulate(int sample_rate) noexcept {
        return tick(sample_rate) * static_cast<float>(depth);
    }

    void reset() noexcept { phase_ = 0.0; }

private:
    double phase_      = 0.0;
    float  prev_phase_ = 0.0f;
    float  sh_value_   = 0.0f;
    std::mt19937 rng_{0xABCDu};
    std::uniform_real_distribution<float> dist_{0.0f, 1.0f};
};

// =============================================================================
//  §13  STATE VARIABLE FILTER (SVF)
//  Topology-preserving TPT form (Vadim Zavalishin) - no pre-warping needed.
//  Provides simultaneous LP, HP, and BP outputs.
//  Q > 0.5: resonance; Q = 0.707: Butterworth; Q → ∞: self-oscillation.
// =============================================================================

class SVFilter {
public:
    enum class Mode { LowPass, HighPass, BandPass };

    Mode   mode     = Mode::LowPass;
    double cutoff   = 1000.0;  ///< Hz
    double Q        = 0.707;   ///< Resonance (0.5–30 typical range)

    SVFilter() = default;
    SVFilter(Mode m, double fc, double q = 0.707) : mode(m), cutoff(fc), Q(q) {}

    void set_params(double fc, double q, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        cutoff = fc;  Q = q;
        update_coeffs(sample_rate);
    }

    /// Process one sample.  Call update_coeffs() after changing params.
    float tick(float x, int sample_rate) noexcept {
        // Lazy coefficient update if sr hasn't been cached
        if (cached_sr_ != sample_rate) update_coeffs(sample_rate);

        // SVF core (TPT)
        const float hp = (x - (2.0f * Q_ + g_) * s1_ - s2_) * h_;
        const float bp = g_ * hp + s1_;
        const float lp = g_ * bp + s2_;

        s1_ = g_ * hp + bp;
        s2_ = g_ * bp + lp;

        switch (mode) {
            case Mode::LowPass:  return lp;
            case Mode::HighPass: return hp;
            case Mode::BandPass: return bp;
        }
        return lp;
    }

    void process(Buffer& buf, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        update_coeffs(sample_rate);
        for (auto& s : buf) s = tick(s, sample_rate);
    }

    void reset() noexcept { s1_ = s2_ = 0.0f; }

    void update_coeffs(int sample_rate) noexcept {
        cached_sr_ = sample_rate;
        const double w  = std::tan(PI * cutoff / sample_rate);
        g_  = static_cast<float>(w);
        Q_  = static_cast<float>(1.0 / (2.0 * Q));
        h_  = 1.0f / (1.0f + 2.0f * Q_ * g_ + g_ * g_);
    }

private:
    float g_ = 0.0f, Q_ = 0.707f, h_ = 1.0f;
    float s1_ = 0.0f, s2_ = 0.0f;
    int   cached_sr_ = 0;
};

// =============================================================================
//  §14  LEGACY 1-POLE FILTERS  (kept for compatibility with existing modules)
// =============================================================================

class LowPassFilter {
public:
    explicit LowPassFilter(double cutoff_hz = 1000.0, int sample_rate = 0) {
        set_cutoff(cutoff_hz, sample_rate ? sample_rate : global_config().sample_rate);
    }
    void set_cutoff(double fc, int sr = 0) {
        if (sr <= 0) sr = global_config().sample_rate;
        const double rc = 1.0 / (TWO_PI * fc);
        const double dt = 1.0 / sr;
        alpha_ = static_cast<float>(dt / (rc + dt));
        z1_ = 0.0f;
    }
    float tick(float x) noexcept { z1_ += alpha_ * (x - z1_); return z1_; }
    void  process(Buffer& b) noexcept { for (auto& s : b) s = tick(s); }
    void  reset()  noexcept { z1_ = 0.0f; }
private:
    float alpha_ = 0.1f, z1_ = 0.0f;
};

class HighPassFilter {
public:
    explicit HighPassFilter(double cutoff_hz = 200.0, int sample_rate = 0) {
        set_cutoff(cutoff_hz, sample_rate ? sample_rate : global_config().sample_rate);
    }
    void set_cutoff(double fc, int sr = 0) {
        if (sr <= 0) sr = global_config().sample_rate;
        const double rc = 1.0 / (TWO_PI * fc);
        const double dt = 1.0 / sr;
        alpha_ = static_cast<float>(rc / (rc + dt));
        xp_ = yp_ = 0.0f;
    }
    float tick(float x) noexcept {
        float y = alpha_ * (yp_ + x - xp_);
        xp_ = x;  yp_ = y;
        return y;
    }
    void process(Buffer& b) noexcept { for (auto& s : b) s = tick(s); }
    void reset()  noexcept { xp_ = yp_ = 0.0f; }
private:
    float alpha_ = 0.9f, xp_ = 0.0f, yp_ = 0.0f;
};

// =============================================================================
//  §15  FM SYNTHESIS  (unchanged from v1)
// =============================================================================

class FMSynth {
public:
    double carrier_freq   = 440.0;
    double modulator_freq = 440.0;
    double mod_index      = 1.0;
    double amplitude      = 1.0;

    Buffer render(double seconds, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t n = static_cast<size_t>(seconds * sample_rate);
        Buffer buf(n);
        const double c_inc = TWO_PI * carrier_freq   / sample_rate;
        const double m_inc = TWO_PI * modulator_freq / sample_rate;
        double cp = 0.0, mp = 0.0;
        for (size_t i = 0; i < n; ++i) {
            buf[i] = static_cast<float>(amplitude * std::sin(cp + mod_index * std::sin(mp)));
            cp += c_inc;  mp += m_inc;
            if (cp >= TWO_PI) cp -= TWO_PI;
            if (mp >= TWO_PI) mp -= TWO_PI;
        }
        return buf;
    }
};

// =============================================================================
//  §16  DISTORTION
// =============================================================================

namespace distortion {

/// Soft clip via hyperbolic tangent.  drive > 1 = more distortion.
inline float soft_clip(float x, float drive = 2.0f) noexcept {
    return std::tanh(x * drive) / std::tanh(drive);
}

/// Hard clip at ±threshold.
inline float hard_clip(float x, float threshold = 0.5f) noexcept {
    return std::max(-threshold, std::min(threshold, x)) / threshold;
}

/// Asymmetric waveshaper: polynomial with tunable knee.
///   knee = 0.0 → linear; knee = 1.0 → maximum curvature
inline float waveshaper(float x, float knee = 0.7f) noexcept {
    // Cubic soft saturation with asymmetric bias
    const float k   = knee * 3.0f;
    const float abs_x = std::abs(x);
    if (abs_x >= 1.0f) return (x > 0) ? 1.0f : -1.0f;
    return x * (1.0f + k * (1.0f - abs_x));
}

/// Process a buffer in place with a chosen distortion function.
inline void apply_soft_clip(Buffer& buf, float drive = 2.0f) {
    for (auto& s : buf) s = soft_clip(s, drive);
}

inline void apply_hard_clip(Buffer& buf, float threshold = 0.5f) {
    for (auto& s : buf) s = hard_clip(s, threshold);
}

inline void apply_waveshaper(Buffer& buf, float knee = 0.7f) {
    for (auto& s : buf) s = waveshaper(s, knee);
}

} // namespace distortion

// =============================================================================
//  §17  DELAY LINE
//  Feedback digital delay.  Supports tempo-synced delays (pass time_ms).
// =============================================================================

class DelayLine {
public:
    double time_ms  = 250.0;  ///< Delay time in milliseconds
    float  feedback = 0.4f;   ///< Feedback amount [0,1)
    float  wet      = 0.5f;   ///< Wet level
    float  dry      = 1.0f;   ///< Dry level

    void init(int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        sr_    = sample_rate;
        size_  = static_cast<size_t>(time_ms * 0.001 * sample_rate) + 1;
        buf_.assign(size_, 0.0f);
        head_  = 0;
    }

    float tick(float x) noexcept {
        const float delayed = buf_[head_];
        buf_[head_] = x + delayed * feedback;
        head_ = (head_ + 1) % size_;
        return x * dry + delayed * wet;
    }

    void process(Buffer& buf) {
        if (buf_.empty()) init(sr_);
        for (auto& s : buf) s = tick(s);
    }

    void reset() noexcept { std::fill(buf_.begin(), buf_.end(), 0.0f); head_ = 0; }

private:
    std::vector<float> buf_;
    size_t             head_ = 0;
    size_t             size_ = 1;
    int                sr_   = 44100;
};

// =============================================================================
//  §18  REVERB  - Schroeder structure
//  4 parallel feedback comb filters → 2 series allpass filters.
//  Classic digital reverb suitable for atmospheric tails.
// =============================================================================

class Reverb {
public:
    float room_size = 0.8f;   ///< [0,1] - longer = larger room
    float damping   = 0.5f;   ///< [0,1] - higher = duller high frequencies
    float wet       = 0.3f;
    float dry       = 1.0f;

    void init(int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        // Comb filter delay lengths (in samples) - prime-ish values
        static const int comb_ms[] = { 29, 37, 41, 43 };
        for (int i = 0; i < 4; ++i) {
            const int len = comb_ms[i] * sample_rate / 1000;
            comb_buf_[i].assign(len, 0.0f);
            comb_head_[i] = 0;
            comb_fb_[i]   = 0.0f;
        }
        static const int ap_ms[] = { 5, 13 };
        for (int i = 0; i < 2; ++i) {
            const int len = ap_ms[i] * sample_rate / 1000;
            ap_buf_[i].assign(len, 0.0f);
            ap_head_[i] = 0;
        }
    }

    float tick(float x) noexcept {
        // Comb bank
        float comb_out = 0.0f;
        for (int i = 0; i < 4; ++i) {
            auto& cb = comb_buf_[i];
            const float delayed  = cb[comb_head_[i]];
            const float dampened = comb_fb_[i] * (1.0f - damping) + delayed * damping;
            comb_fb_[i]          = dampened;
            cb[comb_head_[i]]    = x + dampened * room_size;
            comb_head_[i]        = (comb_head_[i] + 1) % cb.size();
            comb_out            += delayed;
        }
        comb_out *= 0.25f;

        // Allpass chain
        float ap_in = comb_out;
        for (int i = 0; i < 2; ++i) {
            auto& ab = ap_buf_[i];
            const float v    = ab[ap_head_[i]];
            const float out  = -ap_in + v;
            ab[ap_head_[i]]  = ap_in + v * 0.5f;
            ap_head_[i]      = (ap_head_[i] + 1) % ab.size();
            ap_in            = out;
        }

        return x * dry + ap_in * wet;
    }

    void process(Buffer& buf) {
        if (comb_buf_[0].empty()) init();
        for (auto& s : buf) s = tick(s);
    }

    void reset() noexcept {
        for (auto& b : comb_buf_) std::fill(b.begin(), b.end(), 0.0f);
        for (auto& b : ap_buf_)   std::fill(b.begin(), b.end(), 0.0f);
    }

private:
    std::vector<float> comb_buf_[4];
    int                comb_head_[4] = {};
    float              comb_fb_[4]   = {};

    std::vector<float> ap_buf_[2];
    int                ap_head_[2] = {};
};

// =============================================================================
//  §19  CHORUS
//  Modulated delay line that creates pitch/timbre widening.
//  Uses an LFO to vary delay time, detuning copies of the signal slightly.
// =============================================================================

class Chorus {
public:
    double rate_hz    = 0.5;    ///< LFO rate
    double depth_ms   = 1.5;    ///< Modulation depth in ms
    double delay_ms   = 8.0;    ///< Centre delay time in ms
    float  wet        = 0.5f;
    float  dry        = 1.0f;

    void init(int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        sr_  = sample_rate;
        max_delay_ = static_cast<size_t>((delay_ms + depth_ms * 2.0) * 0.001 * sr_) + 2;
        buf_.assign(max_delay_, 0.0f);
        head_ = 0;
    }

    float tick(float x) noexcept {
        buf_[head_] = x;

        lfo_phase_ += TWO_PI * rate_hz / sr_;
        if (lfo_phase_ >= TWO_PI) lfo_phase_ -= TWO_PI;

        const double mod    = depth_ms * 0.001 * sr_ * std::sin(lfo_phase_);
        const double delay  = delay_ms * 0.001 * sr_ + mod;
        const size_t d_int  = static_cast<size_t>(delay);
        const float  frac   = static_cast<float>(delay - d_int);

        const size_t i0 = (head_ + max_delay_ - d_int)     % max_delay_;
        const size_t i1 = (head_ + max_delay_ - d_int - 1) % max_delay_;
        const float  delayed = buf_[i0] * (1.0f - frac) + buf_[i1] * frac;

        head_ = (head_ + 1) % max_delay_;
        return x * dry + delayed * wet;
    }

    void process(Buffer& buf) {
        if (buf_.empty()) init(sr_);
        for (auto& s : buf) s = tick(s);
    }

private:
    std::vector<float> buf_;
    size_t             head_      = 0;
    size_t             max_delay_ = 1;
    double             lfo_phase_ = 0.0;
    int                sr_        = 44100;
};

// =============================================================================
//  §20  STEREO BUFFER UTILITIES
// =============================================================================

/// Pan a mono buffer into a stereo buffer using constant-power panning.
/// pan: -1.0 = full left, 0.0 = centre, 1.0 = full right
inline StereoBuffer pan_mono(const Buffer& mono, float pan, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    const float angle = (pan + 1.0f) * static_cast<float>(PI) * 0.25f; // [0, π/2]
    const float gainL = std::cos(angle);
    const float gainR = std::sin(angle);
    StereoBuffer out(mono.size());
    for (size_t i = 0; i < mono.size(); ++i) {
        out.L[i] = mono[i] * gainL;
        out.R[i] = mono[i] * gainR;
    }
    return out;
}

/// Haas stereo widening: L = original, R = original + tiny delay (1–30 ms).
inline StereoBuffer haas_widen(const Buffer& mono, double delay_ms = 12.0,
                                int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    const size_t delay_samples = static_cast<size_t>(delay_ms * 0.001 * sample_rate);
    StereoBuffer out(mono.size());
    for (size_t i = 0; i < mono.size(); ++i) {
        out.L[i] = mono[i];
        out.R[i] = (i >= delay_samples) ? mono[i - delay_samples] : 0.0f;
    }
    return out;
}

/// Mix a mono buffer into a stereo bus at given pan position.
inline void mix_panned(StereoBuffer& dst, const Buffer& src,
                        float pan = 0.0f, float gain = 1.0f) {
    const float angle = (pan + 1.0f) * static_cast<float>(PI) * 0.25f;
    const float gL = std::cos(angle) * gain;
    const float gR = std::sin(angle) * gain;
    size_t n = std::min(dst.size(), src.size());
    for (size_t i = 0; i < n; ++i) {
        dst.L[i] += src[i] * gL;
        dst.R[i] += src[i] * gR;
    }
}

/// Mix a stereo buffer into a stereo bus.
inline void mix_stereo(StereoBuffer& dst, const StereoBuffer& src, float gain = 1.0f) {
    size_t n = std::min(dst.size(), src.size());
    for (size_t i = 0; i < n; ++i) {
        dst.L[i] += src.L[i] * gain;
        dst.R[i] += src.R[i] * gain;
    }
}

// =============================================================================
//  §21  SIDECHAIN COMPRESSOR
//  Ducker: attenuates `target` buffer based on an envelope follower applied
//  to the `trigger` buffer.  Classic pumping / sidechaining effect.
// =============================================================================

class SidechainCompressor {
public:
    float attack_ms   = 1.0f;    ///< Envelope follower attack  (ms)
    float release_ms  = 100.0f;  ///< Envelope follower release (ms)
    float strength    = 0.9f;    ///< Duck depth [0=no effect, 1=full silence]
    float threshold   = 0.05f;   ///< RMS level above which ducking begins

    /// Apply sidechain ducking: modulate `target` using envelope of `trigger`.
    /// Both buffers must have the same length.
    void apply(Buffer& target, const Buffer& trigger, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const float att_coef = std::exp(-1.0f / (attack_ms  * 0.001f * sample_rate));
        const float rel_coef = std::exp(-1.0f / (release_ms * 0.001f * sample_rate));

        const size_t n = std::min(target.size(), trigger.size());
        float env = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            const float level = std::abs(trigger[i]);
            env = (level > env) ? att_coef * env + (1.0f - att_coef) * level
                                : rel_coef * env + (1.0f - rel_coef) * level;
            const float duck = (env > threshold)
                ? 1.0f - strength * std::min(1.0f, (env - threshold) / (1.0f - threshold))
                : 1.0f;
            target[i] *= duck;
        }
    }

    /// Stereo version - applies same gain reduction to both channels.
    void apply(StereoBuffer& target, const Buffer& trigger, int sample_rate = 0) const {
        Buffer L_copy = target.L;
        apply(target.L, trigger, sample_rate);
        // Compute same gain from L, apply to R
        const float att_coef = std::exp(-1.0f / (attack_ms  * 0.001f * sample_rate));
        const float rel_coef = std::exp(-1.0f / (release_ms * 0.001f * sample_rate));
        const size_t n = std::min(target.R.size(), trigger.size());
        float env = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            const float level = std::abs(trigger[i]);
            env = (level > env) ? att_coef * env + (1.0f - att_coef) * level
                                : rel_coef * env + (1.0f - rel_coef) * level;
            const float duck = (env > threshold)
                ? 1.0f - strength * std::min(1.0f, (env - threshold) / (1.0f - threshold))
                : 1.0f;
            target.R[i] *= duck;
        }
    }
};

// =============================================================================
//  §22  AUTOMATION SYSTEM
//  Parameter automation curves applied to buffer data (volume, filter, pitch).
// =============================================================================

/// An automation point: time + value.
struct AutoPoint { double time_sec; float value; };

/// Compute the linearly interpolated value at `t` from a sorted point list.
inline float auto_eval(const std::vector<AutoPoint>& pts, double t) noexcept {
    if (pts.empty()) return 0.0f;
    if (t <= pts.front().time_sec) return pts.front().value;
    if (t >= pts.back().time_sec)  return pts.back().value;

    for (size_t i = 0; i + 1 < pts.size(); ++i) {
        if (t >= pts[i].time_sec && t <= pts[i+1].time_sec) {
            const double span  = pts[i+1].time_sec - pts[i].time_sec;
            const double frac  = (span > 0) ? (t - pts[i].time_sec) / span : 0.0;
            return static_cast<float>(pts[i].value + frac * (pts[i+1].value - pts[i].value));
        }
    }
    return pts.back().value;
}

/// Apply a volume automation curve to a buffer in-place.
inline void apply_volume_automation(Buffer& buf,
                                     const std::vector<AutoPoint>& pts,
                                     int sample_rate = 0)
{
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sample_rate;
        buf[i] *= auto_eval(pts, t);
    }
}

/// Apply a volume automation curve to a stereo buffer in-place.
inline void apply_volume_automation(StereoBuffer& buf,
                                     const std::vector<AutoPoint>& pts,
                                     int sample_rate = 0)
{
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    for (size_t i = 0; i < buf.size(); ++i) {
        const double t = static_cast<double>(i) / sample_rate;
        const float  v = auto_eval(pts, t);
        buf.L[i] *= v;  buf.R[i] *= v;
    }
}

/// Apply an SVF cutoff automation - filter moves from point to point.
inline Buffer apply_filter_automation(const Buffer& src,
                                       const std::vector<AutoPoint>& cutoff_pts,
                                       double Q,
                                       SVFilter::Mode mode = SVFilter::Mode::LowPass,
                                       int sample_rate = 0)
{
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    Buffer out(src.size());
    SVFilter flt(mode, cutoff_pts.empty() ? 1000.0 : cutoff_pts.front().value, Q);
    for (size_t i = 0; i < src.size(); ++i) {
        const double t  = static_cast<double>(i) / sample_rate;
        const float  fc = auto_eval(cutoff_pts, t);
        flt.cutoff = static_cast<double>(std::max(20.0f, fc));
        flt.update_coeffs(sample_rate);
        out[i] = flt.tick(src[i], sample_rate);
    }
    return out;
}

// =============================================================================
//  §23  POLYPHONIC VOICE SYSTEM
// =============================================================================

/// One voice in a polyphonic synth patch.
struct Voice {
    bool        active    = false;
    double      frequency = 440.0;
    double      velocity  = 1.0;
    uint64_t    birth_sample = 0;   ///< When was this voice triggered (for stealing)
    float       current_amp  = 0.0f;

    Oscillator  osc;
    Oscillator  osc2;     ///< Optional second oscillator (detune layer)
    ADSR        adsr;
    ADSR::State adsr_state;
    SVFilter    filt;

    void trigger(double freq, double vel, const ADSR& env,
                 WaveShape shape1, WaveShape shape2,
                 double detune_hz, uint64_t timestamp, int /*sample_rate*/)
    {
        frequency     = freq;
        velocity      = vel;
        active        = true;
        birth_sample  = timestamp;
        current_amp   = 0.0f;
        adsr          = env;
        adsr_state    = ADSR::State{};

        osc  = Oscillator(shape1, freq, vel);
        osc2 = Oscillator(shape2, freq + detune_hz, vel * 0.6);

        filt.reset();
    }

    void release() { adsr_state.note_off_trigger(); }

    /// Returns next sample (mono).
    float tick(int sample_rate) noexcept {
        if (!active) return 0.0f;
        const float env = adsr.tick(adsr_state, sample_rate);
        if (adsr_state.phase == ADSR::State::DONE) { active = false; return 0.0f; }
        current_amp  = env;
        const float s = (osc.tick(sample_rate) + osc2.tick(sample_rate)) * 0.5f;
        return filt.tick(s * env, sample_rate);
    }
};

/// Polyphonic voice manager - allocates voices, handles stealing.
class VoiceManager {
public:
    static constexpr int MAX_VOICES = 16;

    WaveShape shape1       = WaveShape::Sawtooth;
    WaveShape shape2       = WaveShape::Square;
    double    detune_hz    = 2.5;
    ADSR      adsr         = {0.01, 0.08, 0.7, 0.15};
    SVFilter  filter_proto;   ///< Prototype filter settings copied per voice

    void note_on(double freq, double velocity = 1.0) {
        Voice* v = find_free_voice();
        if (!v) v = steal_voice();
        v->filt = filter_proto;
        v->trigger(freq, velocity, adsr, shape1, shape2, detune_hz,
                   sample_counter_, global_config().sample_rate);
    }

    void note_off(double freq) {
        for (auto& v : voices_)
            if (v.active && std::abs(v.frequency - freq) < 0.1)
                v.release();
    }

    void all_notes_off() {
        for (auto& v : voices_) v.release();
    }

    float tick(int sample_rate) noexcept {
        float sum = 0.0f;
        for (auto& v : voices_) sum += v.tick(sample_rate);
        ++sample_counter_;
        return sum;
    }

    Buffer render(double seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t n = static_cast<size_t>(seconds * sample_rate);
        Buffer out(n);
        for (size_t i = 0; i < n; ++i) out[i] = tick(sample_rate);
        return out;
    }

    int active_voices() const {
        int n = 0;
        for (const auto& v : voices_) if (v.active) ++n;
        return n;
    }

private:
    Voice    voices_[MAX_VOICES];
    uint64_t sample_counter_ = 0;

    Voice* find_free_voice() {
        for (auto& v : voices_) if (!v.active) return &v;
        return nullptr;
    }

    Voice* steal_voice() {
        // Steal the oldest voice
        Voice* oldest = &voices_[0];
        for (auto& v : voices_)
            if (v.birth_sample < oldest->birth_sample) oldest = &v;
        return oldest;
    }
};

// =============================================================================
//  §24  MUSIC THEORY HELPERS
// =============================================================================

inline double midi_to_freq(int midi_note) noexcept {
    return 440.0 * std::pow(2.0, (midi_note - 69) / 12.0);
}

namespace notes {
    constexpr double C0   =   16.35; constexpr double Cs0  =   17.32;
    constexpr double D0   =   18.35; constexpr double Ds0  =   19.45;
    constexpr double E0   =   20.60; constexpr double F0   =   21.83;
    constexpr double Fs0  =   23.12; constexpr double G0   =   24.50;
    constexpr double Gs0  =   25.96; constexpr double A0   =   27.50;
    constexpr double As0  =   29.14; constexpr double B0   =   30.87;
    constexpr double C1   =   32.70; constexpr double Cs1  =   34.65;
    constexpr double D1   =   36.71; constexpr double Ds1  =   38.89;
    constexpr double E1   =   41.20; constexpr double F1   =   43.65;
    constexpr double Fs1  =   46.25; constexpr double G1   =   49.00;
    constexpr double Gs1  =   51.91; constexpr double A1   =   55.00;
    constexpr double As1  =   58.27; constexpr double B1   =   61.74;
    constexpr double C2   =   65.41; constexpr double Cs2  =   69.30;
    constexpr double D2   =   73.42; constexpr double Ds2  =   77.78;
    constexpr double E2   =   82.41; constexpr double F2   =   87.31;
    constexpr double Fs2  =   92.50; constexpr double G2   =   98.00;
    constexpr double Gs2  =  103.83; constexpr double A2   =  110.00;
    constexpr double As2  =  116.54; constexpr double B2   =  123.47;
    constexpr double C3   =  130.81; constexpr double Cs3  =  138.59;
    constexpr double D3   =  146.83; constexpr double Ds3  =  155.56;
    constexpr double E3   =  164.81; constexpr double F3   =  174.61;
    constexpr double Fs3  =  185.00; constexpr double G3   =  196.00;
    constexpr double Gs3  =  207.65; constexpr double A3   =  220.00;
    constexpr double As3  =  233.08; constexpr double B3   =  246.94;
    constexpr double C4   =  261.63; constexpr double Cs4  =  277.18;
    constexpr double D4   =  293.66; constexpr double Ds4  =  311.13;
    constexpr double E4   =  329.63; constexpr double F4   =  349.23;
    constexpr double Fs4  =  369.99; constexpr double G4   =  392.00;
    constexpr double Gs4  =  415.30; constexpr double A4   =  440.00;
    constexpr double As4  =  466.16; constexpr double B4   =  493.88;
    constexpr double C5   =  523.25; constexpr double Cs5  =  554.37;
    constexpr double D5   =  587.33; constexpr double Ds5  =  622.25;
    constexpr double E5   =  659.26; constexpr double F5   =  698.46;
    constexpr double Fs5  =  739.99; constexpr double G5   =  783.99;
    constexpr double Gs5  =  830.61; constexpr double A5   =  880.00;
    constexpr double As5  =  932.33; constexpr double B5   =  987.77;
    constexpr double C6   = 1046.50; constexpr double Cs6  = 1108.73;
    constexpr double D6   = 1174.66; constexpr double Ds6  = 1244.51;
    constexpr double E6   = 1318.51; constexpr double F6   = 1396.91;
    constexpr double Fs6  = 1479.98; constexpr double G6   = 1567.98;
    constexpr double Gs6  = 1661.22; constexpr double A6   = 1760.00;
    constexpr double As6  = 1864.66; constexpr double B6   = 1975.53;
    constexpr double C7   = 2093.00; constexpr double Cs7  = 2217.46;
    constexpr double D7   = 2349.32; constexpr double Ds7  = 2489.02;
    constexpr double E7   = 2637.02; constexpr double F7   = 2793.83;
    constexpr double Fs7  = 2959.96; constexpr double G7   = 3135.96;
    constexpr double Gs7  = 3322.44; constexpr double A7   = 3520.00;
    constexpr double As7  = 3729.31; constexpr double B7   = 3951.07;
    constexpr double C8   = 4186.01; constexpr double Cs8  = 4434.92;
    constexpr double D8   = 4698.64; constexpr double Ds8  = 4978.03;
    constexpr double E8   = 5274.04; constexpr double F8   = 5587.65;
    constexpr double Fs8  = 5919.91; constexpr double G8   = 6271.93;
    constexpr double Gs8  = 6644.88; constexpr double A8   = 7040.00;
    constexpr double As8  = 7458.62; constexpr double B8   = 7902.13;
    constexpr double C9   = 8372.02; constexpr double Cs9  = 8869.84;
    constexpr double D9   = 9397.27; constexpr double Ds9  = 9956.06;
    constexpr double E9   = 10548.08;constexpr double F9  = 11175.30;
    constexpr double Fs9  = 11839.82;constexpr double G9  = 12543.85;
    constexpr double Gs9  = 13289.75;constexpr double A9  = 14080.00;
    constexpr double As9  = 14917.24;constexpr double B9  = 15804.26;
    constexpr double REST = 0.0;
} // namespace notes

// =============================================================================
//  §25  NOTE / INSTRUMENT / TRACK  (unchanged from v1, stereo-aware)
// =============================================================================

struct Note {
    double frequency = 440.0;
    double duration  = 0.25;
    double amplitude = 0.8;
    Note() = default;
    Note(double f, double d, double a = 0.8) : frequency(f), duration(d), amplitude(a) {}
};

struct Instrument {
    WaveShape shape      = WaveShape::Sine;
    ADSR      envelope   = {0.01, 0.05, 0.7, 0.08};
    float     gain       = 1.0f;
    double    detune     = 0.0;
    bool      use_lpf    = false;
    double    lpf_cutoff = 2000.0;
    double    duty       = 0.5;

    Buffer render_note(const Note& note, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        if (note.frequency < 1.0) return make_buffer(note.duration, sample_rate);
        Buffer buf = make_buffer(note.duration, sample_rate);
        Oscillator osc(shape, note.frequency + detune,
                       note.amplitude * static_cast<double>(gain));
        osc.duty = duty;
        for (size_t i = 0; i < buf.size(); ++i) buf[i] = osc.tick(sample_rate);
        envelope.apply(buf, note.duration, -1.0, sample_rate);
        if (use_lpf) { LowPassFilter lpf(lpf_cutoff, sample_rate); lpf.process(buf); }
        return buf;
    }
};

class Track {
public:
    Instrument instrument;
    Track() = default;
    explicit Track(const Instrument& i) : instrument(i) {}

    Track& add(double f, double d, double a = 0.8) {
        notes_.push_back({f, d, a}); return *this;
    }
    Track& rest(double d) { notes_.push_back({0.0, d}); return *this; }
    Track& add(const Note& n) { notes_.push_back(n); return *this; }

    double total_duration() const {
        double t = 0.0; for (auto& n : notes_) t += n.duration; return t;
    }

    Buffer render(int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        Buffer out = make_buffer(total_duration(), sample_rate);
        size_t off = 0;
        for (auto& note : notes_) {
            Buffer nb = instrument.render_note(note, sample_rate);
            for (size_t i = 0; i < nb.size() && off+i < out.size(); ++i)
                out[off+i] += nb[i];
            off += nb.size();
        }
        return out;
    }

private:
    std::vector<Note> notes_;
};

// Helpers
inline Buffer render_chord(const std::vector<double>& freqs, double duration,
                            const Instrument& instr, int sample_rate = 0)
{
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    Buffer out = make_buffer(duration, sample_rate);
    for (double f : freqs) {
        if (f < 1.0) continue;
        Instrument tmp = instr; tmp.gain = 1.0f;
        mix_into(out, tmp.render_note({f, duration, instr.gain}, sample_rate), instr.gain);
    }
    return out;
}

inline Buffer render_arpeggio(const std::vector<double>& freqs, double step_dur,
                               int cycles, const Instrument& instr, int sample_rate = 0)
{
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    Track tr(instr);
    const int total = static_cast<int>(freqs.size()) * cycles;
    for (int i = 0; i < total; ++i) tr.add(freqs[i % freqs.size()], step_dur);
    return tr.render(sample_rate);
}

// =============================================================================
//  §26  DRUM SEQUENCER  (unchanged from v1)
// =============================================================================

class DrumSequencer {
public:
    double bpm           = 120.0;
    int    steps_per_bar = 16;

    struct Pattern {
        std::vector<bool>          steps;
        std::function<Buffer(int)> sound;
        float                      gain = 1.0f;
    };

    void add_pattern(std::vector<bool> steps,
                     std::function<Buffer(int)> sound, float gain = 1.0f)
    { patterns_.push_back({ std::move(steps), std::move(sound), gain }); }

    void clear_patterns() { patterns_.clear(); }

    Buffer render(int num_bars = 1, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const double step_dur = 60.0 / (bpm * static_cast<double>(steps_per_bar) / 4.0);
        Buffer out = make_buffer(step_dur * steps_per_bar * num_bars, sample_rate);
        for (auto& pat : patterns_) {
            const int ns = static_cast<int>(pat.steps.size());
            for (int bar = 0; bar < num_bars; ++bar) {
                for (int s = 0; s < steps_per_bar; ++s) {
                    if (!pat.steps[s % ns]) continue;
                    const size_t off = static_cast<size_t>((bar*steps_per_bar+s)*step_dur*sample_rate);
                    Buffer snd = pat.sound(sample_rate);
                    for (size_t i = 0; i < snd.size() && off+i < out.size(); ++i)
                        out[off+i] += snd[i] * pat.gain;
                }
            }
        }
        return out;
    }

private:
    std::vector<Pattern> patterns_;
};

// =============================================================================
//  §27  MASTER BUS  (now handles both mono Buffer and StereoBuffer)
// =============================================================================

class MasterBus {
public:
    int sample_rate;
    explicit MasterBus(int sr = 0)
        : sample_rate(sr > 0 ? sr : global_config().sample_rate) {}

    void schedule(Buffer buf, double onset, float gain = 1.0f) {
        mono_slots_.push_back({ onset, gain, std::move(buf) });
    }
    void schedule(StereoBuffer buf, double onset, float gain = 1.0f) {
        stereo_slots_.push_back({ onset, gain, std::move(buf) });
    }

    void advance(double s) { cursor_ += s; }
    double cursor() const noexcept { return cursor_; }
    void set_cursor(double t) { cursor_ = t; }

    void place(Buffer buf, float gain = 1.0f) {
        schedule(std::move(buf), cursor_, gain);
    }
    void place(StereoBuffer buf, float gain = 1.0f) {
        schedule(std::move(buf), cursor_, gain);
    }

    /// Mix all scheduled content into a stereo output.
    StereoBuffer mix_stereo() const {
        double total = 0.0;
        for (auto& s : mono_slots_)
            total = std::max(total, s.onset + (double)s.buf.size() / sample_rate);
        for (auto& s : stereo_slots_)
            total = std::max(total, s.onset + (double)s.buf.size() / sample_rate);

        StereoBuffer out(static_cast<size_t>(total * sample_rate), 0.0f);

        for (auto& s : mono_slots_) {
            const size_t off = static_cast<size_t>(s.onset * sample_rate);
            for (size_t i = 0; i < s.buf.size() && off+i < out.size(); ++i) {
                const float v = s.buf[i] * s.gain;
                out.L[off+i] += v;
                out.R[off+i] += v;
            }
        }
        for (auto& s : stereo_slots_) {
            const size_t off = static_cast<size_t>(s.onset * sample_rate);
            for (size_t i = 0; i < s.buf.size() && off+i < out.size(); ++i) {
                out.L[off+i] += s.buf.L[i] * s.gain;
                out.R[off+i] += s.buf.R[i] * s.gain;
            }
        }
        return out;
    }

    /// Mix all scheduled content into a mono output.
    Buffer mix() const {
        StereoBuffer st = mix_stereo();
        return st.to_mono();
    }

    void reset() { mono_slots_.clear(); stereo_slots_.clear(); cursor_ = 0.0; }

private:
    struct MonoSlot   { double onset; float gain; Buffer       buf; };
    struct StereoSlot { double onset; float gain; StereoBuffer buf; };
    std::vector<MonoSlot>   mono_slots_;
    std::vector<StereoSlot> stereo_slots_;
    double cursor_ = 0.0;
};

// =============================================================================
//  §28  WAV FILE EXPORT  (mono and stereo)
// =============================================================================

inline void save_wav(const Buffer& buf, const std::string& path, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    const auto pcm    = to_pcm16(buf);
    const uint32_t dsz    = static_cast<uint32_t>(pcm.size() * 2u);
    const uint32_t chksz  = 36u + dsz;
    FILE* f = nullptr;
#ifdef WAUVIO_WINDOWS
    fopen_s(&f, path.c_str(), "wb");
#else
    f = std::fopen(path.c_str(), "wb");
#endif
    if (!f) throw std::runtime_error("save_wav: cannot open " + path);
    auto w4 = [&](uint32_t v){ std::fwrite(&v,4,1,f); };
    auto w2 = [&](uint16_t v){ std::fwrite(&v,2,1,f); };
    auto ws = [&](const char* s, size_t n){ std::fwrite(s,1,n,f); };
    ws("RIFF",4); w4(chksz); ws("WAVE",4);
    ws("fmt ",4); w4(16); w2(1u); w2(1u);
    w4((uint32_t)sample_rate); w4((uint32_t)(sample_rate*2)); w2(2u); w2(16u);
    ws("data",4); w4(dsz);
    std::fwrite(pcm.data(),2,pcm.size(),f);
    std::fclose(f);
}

/// Save a stereo WAV (interleaved L/R, 16-bit PCM).
inline void save_wav_stereo(const StereoBuffer& buf, const std::string& path,
                             int sample_rate = 0)
{
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    Buffer interleaved = buf.interleave();
    const auto pcm     = to_pcm16(interleaved);
    const uint32_t dsz   = static_cast<uint32_t>(pcm.size() * 2u);
    const uint32_t chksz = 36u + dsz;
    FILE* f = nullptr;
#ifdef WAUVIO_WINDOWS
    fopen_s(&f, path.c_str(), "wb");
#else
    f = std::fopen(path.c_str(), "wb");
#endif
    if (!f) throw std::runtime_error("save_wav_stereo: cannot open " + path);
    auto w4 = [&](uint32_t v){ std::fwrite(&v,4,1,f); };
    auto w2 = [&](uint16_t v){ std::fwrite(&v,2,1,f); };
    auto ws = [&](const char* s, size_t n){ std::fwrite(s,1,n,f); };
    ws("RIFF",4); w4(chksz); ws("WAVE",4);
    ws("fmt ",4); w4(16); w2(1u); w2(2u);  // 2 channels
    w4((uint32_t)sample_rate);
    w4((uint32_t)(sample_rate * 4));  // byte rate: sr * channels * bytes_per_sample
    w2(4u);   // block align: 2 channels * 2 bytes
    w2(16u);
    ws("data",4); w4(dsz);
    std::fwrite(pcm.data(),2,pcm.size(),f);
    std::fclose(f);
}

// =============================================================================
//  §29  PLAYBACK  v1.3.3
// =============================================================================

namespace detail {

// ---------------------------------------------------------------------------
//  Internal state of one playback stream
// ---------------------------------------------------------------------------
class PlaybackInstance {
public:
    // Construct with a pre-converted PCM buffer + metadata.
    // The PCM data is moved in - this instance owns it exclusively.
    PlaybackInstance(std::vector<int16_t> pcm, int sample_rate, int channels)
        : pcm_(std::move(pcm))
        , sample_rate_(sample_rate)
        , channels_(channels)
        , stopped_(false)
        , paused_(false)
        , playing_(true)
        , volume_(1.0f)
    {}

    // Non-copyable, non-movable (shared_ptr is the indirection layer).
    PlaybackInstance(const PlaybackInstance&)            = delete;
    PlaybackInstance& operator=(const PlaybackInstance&) = delete;

    // ---- Public control API ----

    void stop() {
        stopped_.store(true, std::memory_order_release);
        // Wake the worker if it is paused so it can exit cleanly.
        {
            std::lock_guard<std::mutex> lk(pause_mtx_);
            paused_.store(false, std::memory_order_release);
        }
        pause_cv_.notify_all();
    }

    void pause() {
        paused_.store(true, std::memory_order_release);
    }

    void resume() {
        {
            std::lock_guard<std::mutex> lk(pause_mtx_);
            paused_.store(false, std::memory_order_release);
        }
        pause_cv_.notify_all();
    }

    bool is_playing() const noexcept {
        return playing_.load(std::memory_order_acquire);
    }

    void set_volume(float v) noexcept {
        volume_.store(v < 0.0f ? 0.0f : (v > 2.0f ? 2.0f : v),
                      std::memory_order_relaxed);
    }

    float volume() const noexcept {
        return volume_.load(std::memory_order_relaxed);
    }

    // ---- Worker entry point ----
    // Called once from the worker thread spawned by PlaybackRuntime.

    void run() {
#ifdef WAUVIO_LINUX
        run_linux();
#elif defined(WAUVIO_WINDOWS)
        run_windows();
#endif
        playing_.store(false, std::memory_order_release);
    }

private:
    // PCM data (owned exclusively by this instance)
    std::vector<int16_t> pcm_;
    int                  sample_rate_;
    int                  channels_;

    // Atomic flags - readable from any thread without locking
    std::atomic<bool>  stopped_;
    std::atomic<bool>  paused_;
    std::atomic<bool>  playing_;
    std::atomic<float> volume_;

    // Pause mechanism
    std::mutex              pause_mtx_;
    std::condition_variable pause_cv_;

    // Block the worker while paused (or exit immediately if stopped).
    void wait_while_paused() {
        std::unique_lock<std::mutex> lk(pause_mtx_);
        pause_cv_.wait(lk, [this]{
            return !paused_.load(std::memory_order_acquire)
                || stopped_.load(std::memory_order_acquire);
        });
    }

    // Apply current volume to a chunk of samples (in-place, temporary copy).
    // We write from a local scaled buffer so the master pcm_ stays intact
    // (allows replay in future iterations if needed).
    static void scale_chunk(const int16_t* src, int16_t* dst,
                             size_t count, float vol) noexcept
    {
        for (size_t i = 0; i < count; ++i) {
            float s = static_cast<float>(src[i]) * vol;
            s = s < -32768.0f ? -32768.0f : (s > 32767.0f ? 32767.0f : s);
            dst[i] = static_cast<int16_t>(s);
        }
    }

#ifdef WAUVIO_LINUX
    // ---- Linux: stream PCM to ffmpeg/ffplay via popen ----
    // We write in small chunks so we can honour stop/pause between writes.

    static constexpr size_t CHUNK_FRAMES = 4096; // frames per write

    void run_linux() {
        const std::string sr  = std::to_string(sample_rate_);
        const std::string ach = std::to_string(channels_);
        const std::string cmd =
            "ffmpeg -hide_banner -loglevel error "
            "-f s16le -ar " + sr + " -ac " + ach + " -i pipe:0 "
            "-f alsa default 2>/dev/null || "
            "ffmpeg -hide_banner -loglevel error "
            "-f s16le -ar " + sr + " -ac " + ach + " -i pipe:0 "
            "-f pulse default 2>/dev/null || "
            "ffplay -hide_banner -loglevel error "
            "-f s16le -ar " + sr + " -ac " + ach + " -";

        FILE* pipe = popen(cmd.c_str(), "w");
        if (!pipe) { playing_.store(false, std::memory_order_release); return; }

        const size_t chunk_samples = CHUNK_FRAMES * static_cast<size_t>(channels_);
        std::vector<int16_t> chunk(chunk_samples);
        size_t pos = 0;

        while (pos < pcm_.size()) {
            // Honour stop
            if (stopped_.load(std::memory_order_acquire)) break;

            // Honour pause: block until resumed or stopped
            if (paused_.load(std::memory_order_acquire)) {
                wait_while_paused();
                if (stopped_.load(std::memory_order_acquire)) break;
            }

            const size_t remaining = pcm_.size() - pos;
            const size_t n         = remaining < chunk_samples ? remaining : chunk_samples;

            scale_chunk(pcm_.data() + pos, chunk.data(), n,
                        volume_.load(std::memory_order_relaxed));

            if (std::fwrite(chunk.data(), sizeof(int16_t), n, pipe) != n) break;
            pos += n;
        }

        pclose(pipe);
    }
#endif // WAUVIO_LINUX

#ifdef WAUVIO_WINDOWS
    // ---- Windows: stream PCM via WaveOut double-buffering ----
    // We use two alternating WAVEHDR buffers so we can check
    // stop/pause between buffer completions without a Sleep() spin.

    static constexpr DWORD CHUNK_FRAMES_WIN = 4096;

    void run_windows() {
        const DWORD  sr   = static_cast<DWORD>(sample_rate_);
        const WORD   ch   = static_cast<WORD>(channels_);
        const WORD   ba   = static_cast<WORD>(ch * 2);          // block align
        const DWORD  abps = static_cast<DWORD>(sr) * ba;        // avg bytes/sec

        WAVEFORMATEX wfx{};
        wfx.wFormatTag      = WAVE_FORMAT_PCM;
        wfx.nChannels       = ch;
        wfx.nSamplesPerSec  = sr;
        wfx.wBitsPerSample  = 16;
        wfx.nBlockAlign     = ba;
        wfx.nAvgBytesPerSec = abps;

        HWAVEOUT hwo = nullptr;
        if (waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL)
                != MMSYSERR_NOERROR)
            return;

        const size_t chunk_samples =
            static_cast<size_t>(CHUNK_FRAMES_WIN) * static_cast<size_t>(channels_);
        const DWORD  chunk_bytes   = static_cast<DWORD>(chunk_samples * 2u);

        // Two alternating PCM buffers (double-buffering)
        std::vector<int16_t> bufs[2];
        bufs[0].resize(chunk_samples);
        bufs[1].resize(chunk_samples);
        WAVEHDR hdrs[2]{};
        for (int b = 0; b < 2; ++b) {
            hdrs[b].lpData         = reinterpret_cast<LPSTR>(bufs[b].data());
            hdrs[b].dwBufferLength = chunk_bytes;
            waveOutPrepareHeader(hwo, &hdrs[b], sizeof(WAVEHDR));
        }

        size_t pos  = 0;
        int    slot = 0;

        while (pos < pcm_.size()) {
            if (stopped_.load(std::memory_order_acquire)) break;
            if (paused_.load(std::memory_order_acquire)) {
                // Wait while paused, but don't block WaveOut - let it drain
                wait_while_paused();
                if (stopped_.load(std::memory_order_acquire)) break;
            }

            WAVEHDR& hdr = hdrs[slot];

            // Wait for this slot to finish if it's still playing
            while ((hdr.dwFlags & WHDR_INQUEUE) && !stopped_.load(std::memory_order_acquire)) {
                Sleep(1);
            }
            if (stopped_.load(std::memory_order_acquire)) break;

            const size_t remaining = pcm_.size() - pos;
            const size_t n = remaining < chunk_samples ? remaining : chunk_samples;

            scale_chunk(pcm_.data() + pos, bufs[slot].data(), n,
                        volume_.load(std::memory_order_relaxed));

            // Zero-pad the last partial chunk
            if (n < chunk_samples)
                std::fill(bufs[slot].begin() + static_cast<ptrdiff_t>(n),
                          bufs[slot].end(), int16_t{0});

            hdr.dwBufferLength = static_cast<DWORD>(n * 2u);
            waveOutWrite(hwo, &hdr, sizeof(WAVEHDR));

            pos  += n;
            slot  = 1 - slot;
        }

        // Drain: wait for both buffers to complete
        for (int b = 0; b < 2; ++b) {
            while ((hdrs[b].dwFlags & WHDR_INQUEUE)) Sleep(1);
        }

        for (int b = 0; b < 2; ++b)
            waveOutUnprepareHeader(hwo, &hdrs[b], sizeof(WAVEHDR));

        waveOutReset(hwo);
        waveOutClose(hwo);
    }
#endif // WAUVIO_WINDOWS
};

// ---------------------------------------------------------------------------
//  PlaybackRuntime - singleton registry of active instances
// ---------------------------------------------------------------------------
class PlaybackRuntime {
public:
    static PlaybackRuntime& instance() {
        static PlaybackRuntime rt;
        return rt;
    }

    // Launch a new PlaybackInstance on its own thread.
    // Returns a shared_ptr so the caller can build a PlaybackHandle.
    std::shared_ptr<PlaybackInstance> launch(
            std::vector<int16_t> pcm, int sample_rate, int channels)
    {
        auto inst = std::make_shared<PlaybackInstance>(
                std::move(pcm), sample_rate, channels);

        // Register a weak_ptr in the registry before spawning the thread.
        {
            std::lock_guard<std::mutex> lk(registry_mtx_);
            // Opportunistic cleanup of dead entries
            registry_.remove_if(
                [](const std::weak_ptr<PlaybackInstance>& w){ return w.expired(); });
            registry_.emplace_back(inst);
        }

        // Spawn worker thread.  The thread holds a strong shared_ptr so the
        // instance lives until playback finishes even if the caller discards
        // their PlaybackHandle.
        std::thread([inst]() mutable {
            inst->run();
            // inst falls out of scope here - ref-count drops, memory freed
        }).detach();

        return inst;
    }

    // Request all active streams to stop (useful for clean shutdown).
    void stop_all() {
        std::lock_guard<std::mutex> lk(registry_mtx_);
        for (auto& w : registry_) {
            if (auto s = w.lock()) s->stop();
        }
        registry_.clear();
    }

    ~PlaybackRuntime() { stop_all(); }

private:
    PlaybackRuntime() = default;

    std::mutex                                    registry_mtx_;
    std::list<std::weak_ptr<PlaybackInstance>>    registry_;
};

} // namespace detail

// =============================================================================
//  PUBLIC API - PlaybackHandle
// =============================================================================

/// Lightweight, copyable handle to an in-flight playback stream.
/// All methods are thread-safe.
/// Discarding the handle does NOT stop playback (fire-and-forget).
class PlaybackHandle {
public:
    PlaybackHandle() = default;
    explicit PlaybackHandle(std::shared_ptr<detail::PlaybackInstance> inst)
        : inst_(std::move(inst)) {}

    /// Stop playback immediately and release resources.
    void stop()   { if (inst_) inst_->stop();   }

    /// Pause playback (resume() to continue).
    void pause()  { if (inst_) inst_->pause();  }

    /// Resume after pause().
    void resume() { if (inst_) inst_->resume(); }

    /// Returns true while the worker thread is still active.
    bool is_playing() const noexcept {
        return inst_ && inst_->is_playing();
    }

    /// Set output volume multiplier [0.0, 2.0].  Thread-safe.
    void set_volume(float v) { if (inst_) inst_->set_volume(v); }

    /// Block the calling thread until playback finishes (or is stopped).
    void wait() const {
        if (!inst_) return;
        while (inst_->is_playing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    /// True if this handle refers to a real stream.
    explicit operator bool() const noexcept { return inst_ != nullptr; }

private:
    std::shared_ptr<detail::PlaybackInstance> inst_;
};

// =============================================================================
//  play_async  -  non-blocking launch
// =============================================================================

/// Launch mono buffer playback asynchronously.
/// Returns immediately; audio runs on a background thread.
inline PlaybackHandle play_async(const Buffer& buf, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    auto pcm = to_pcm16(buf);
    auto inst = detail::PlaybackRuntime::instance().launch(
            std::move(pcm), sample_rate, 1);
    return PlaybackHandle(std::move(inst));
}

/// Launch stereo buffer playback asynchronously.
/// Returns immediately; audio runs on a background thread.
inline PlaybackHandle play_async(const StereoBuffer& buf, int sample_rate = 0) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    Buffer il  = buf.interleave();
    auto pcm   = to_pcm16(il);
    auto inst  = detail::PlaybackRuntime::instance().launch(
            std::move(pcm), sample_rate, 2);
    return PlaybackHandle(std::move(inst));
}

// =============================================================================
//  play  -  blocking (backward-compatible)
// =============================================================================

/// Blocking playback of a mono buffer.
/// Internally delegates to play_async() and waits for completion.
inline void play(const Buffer& buf, int sample_rate = 0) {
    play_async(buf, sample_rate).wait();
}

/// Blocking playback of a stereo buffer.
/// Internally delegates to play_async() and waits for completion.
inline void play(const StereoBuffer& buf, int sample_rate = 0) {
    play_async(buf, sample_rate).wait();
}

/// Stop every active playback stream immediately.
inline void stop_all_playback() {
    detail::PlaybackRuntime::instance().stop_all();
}

} // namespace wauvio