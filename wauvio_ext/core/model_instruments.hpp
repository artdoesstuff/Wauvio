#pragma once

#include "../samples/sampling.hpp"
#include <map>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <utility>

namespace wauvio {
namespace audio {

struct TimbreRecipe {

    WaveShape osc1_shape   = WaveShape::BL_Sawtooth;
    WaveShape osc2_shape   = WaveShape::Sine;
    double    osc2_ratio   = 2.0;
    double    osc_mix      = 0.35;
    double    detune_cents = 4.0;

    bool      use_fm       = false;
    double    fm_ratio     = 2.0;
    double    fm_index     = 2.0;

    double noise_mix   = 0.0;
    double noise_attack_burst = 0.0;
    bool   pink_noise  = true;

    DAHDSR envelope { 0.0, 0.015, 0.0, 0.12, 0.72, 0.18 };
    bool   percussive = false;

    double filter_cutoff     = 5000.0;
    double filter_q          = 0.8;
    bool   filter_env        = true;
    double filter_env_amount = 0.0;

    double vibrato_rate_hz     = 5.2;
    double vibrato_depth_cents = 0.0;
    double vibrato_delay_sec   = 0.18;
    double pitch_attack_cents  = 0.0;
    double pitch_attack_time   = 0.035;
    double fixed_pitch_hz      = -1.0;
    double pitch_instability   = 0.0;

    double formant1_hz = 0.0, formant1_gain_db = 0.0, formant1_q = 3.0;
    double formant2_hz = 0.0, formant2_gain_db = 0.0, formant2_q = 3.0;

    double stereo_width = 0.18;
    double gain          = 0.9;

    std::map<Articulation, double> art_decay_scale;
    std::map<Articulation, double> art_noise_boost;
    std::map<Articulation, double> art_pitch_shift;
    std::map<Articulation, double> art_filter_scale;
};

inline StereoBuffer render_timbre(const TimbreRecipe& r, const Note& note, int sample_rate) {
    if (sample_rate <= 0) sample_rate = global_config().sample_rate;
    const double duration = std::max(0.02, note.duration);
    const size_t n = static_cast<size_t>(duration * sample_rate);
    if (n == 0) return StereoBuffer();

    Articulation art = note.articulation;
    double decay_scale  = 1.0, noise_boost = 0.0, pitch_shift_semi = 0.0, filter_scale = 1.0;
    auto get = [](const std::map<Articulation,double>& m, Articulation a, double def) {
        auto it = m.find(a); return it != m.end() ? it->second : def;
    };
    decay_scale      = get(r.art_decay_scale, art, 1.0);
    noise_boost      = get(r.art_noise_boost, art, 0.0);
    pitch_shift_semi = get(r.art_pitch_shift, art, 0.0);
    filter_scale     = get(r.art_filter_scale, art, 1.0);

    bool percussive = r.percussive;
    if (art == Articulation::Pizzicato) { percussive = true; decay_scale = std::min(decay_scale, 0.28); noise_boost += 0.12; }
    if (art == Articulation::Staccato)  { decay_scale = std::min(decay_scale, 0.4); }
    if (art == Articulation::Muted || art == Articulation::PalmMute) { filter_scale *= 0.45; decay_scale = std::min(decay_scale, 0.55); }
    if (art == Articulation::Harmonic)  { pitch_shift_semi += 12.0; filter_scale *= 1.6; }
    if (art == Articulation::Hard)      { filter_scale *= 1.3; }
    if (art == Articulation::Soft)      { filter_scale *= 0.7; }
    double art_extra_noise = 0.0;
    bool art_click_only = false;
    if (art == Articulation::Spiccato)  { percussive = true; decay_scale = std::min(decay_scale, 0.18); art_extra_noise += 0.1; }
    if (art == Articulation::SulPonticello) { filter_scale *= 2.1; art_extra_noise += 0.08; }
    if (art == Articulation::ColLegno)  { percussive = true; decay_scale = std::min(decay_scale, 0.08); art_extra_noise += 0.55; art_click_only = true; }
    if (art == Articulation::ShortStab) { percussive = true; decay_scale = std::min(decay_scale, 0.15); }
    if (art == Articulation::Whisper)   { art_extra_noise += 0.7; filter_scale *= 0.8; }
    if (art == Articulation::Breath)    { art_extra_noise += 0.85; }
    if (art == Articulation::Humming)   { filter_scale *= 0.6; }

    double base_freq = (r.fixed_pitch_hz >= 0.0)
        ? r.fixed_pitch_hz
        : midi_to_freq(note.midi_note) * std::pow(2.0, pitch_shift_semi / 12.0)
                                        * std::pow(2.0, note.pitch_bend_semitones / 12.0);

    const double velocity = dynamics_to_velocity(note.dynamics) * std::max(0.0, std::min(1.0, note.expression));

    const bool has_glide = (note.glide_from_midi >= 0 && r.fixed_pitch_hz < 0.0);
    const double glide_start_cents = has_glide
        ? 1200.0 * std::log2(midi_to_freq(note.glide_from_midi) / std::max(1.0, base_freq))
        : 0.0;
    const double glide_time = std::max(0.001, note.glide_time);

    Oscillator osc1(r.osc1_shape, base_freq, 1.0);
    Oscillator osc2(r.osc2_shape, base_freq * r.osc2_ratio * std::pow(2.0, r.detune_cents / 1200.0), 1.0);
    osc1.duty = 0.5; osc2.duty = 0.5;

    NoiseGenerator noise(static_cast<uint32_t>(note.midi_note * 7919u + 17u));
    NoiseGenerator drift_noise(static_cast<uint32_t>(note.midi_note * 104729u + 3u));
    double drift = 0.0;

    SVFilter filt(SVFilter::Mode::LowPass, std::max(80.0, r.filter_cutoff * filter_scale), r.filter_q);

    Buffer mono(n);

    const double fm_c_ratio = 1.0;
    double fm_cp = 0.0, fm_mp = 0.0;
    const double fm_c_inc = TWO_PI * base_freq * fm_c_ratio / sample_rate;
    const double fm_m_inc = TWO_PI * base_freq * r.fm_ratio / sample_rate;

    for (size_t i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / sample_rate;

        double pitch_mult = 1.0;
        if (has_glide && t < glide_time) {
            double f = 1.0 - (t / glide_time);
            pitch_mult *= std::pow(2.0, (glide_start_cents * f) / 1200.0);
        }
        if (r.pitch_attack_cents != 0.0 && t < r.pitch_attack_time) {
            double f = 1.0 - (t / r.pitch_attack_time);
            pitch_mult *= std::pow(2.0, (r.pitch_attack_cents * f) / 1200.0);
        }
        if (r.vibrato_depth_cents > 0.0 && t > r.vibrato_delay_sec) {
            double vt = (t - r.vibrato_delay_sec);
            double lfo = std::sin(TWO_PI * r.vibrato_rate_hz * vt);
            pitch_mult *= std::pow(2.0, (lfo * r.vibrato_depth_cents) / 1200.0);
        }
        if (r.pitch_instability > 0.0) {
            double target = drift_noise.tick_pink();
            drift += (target - drift) * 0.0004;
            pitch_mult *= std::pow(2.0, (drift * r.pitch_instability) / 1200.0);
        }
        osc1.frequency = base_freq * pitch_mult;
        osc2.frequency = base_freq * r.osc2_ratio * std::pow(2.0, r.detune_cents / 1200.0) * pitch_mult;

        float sample;
        if (art_click_only) {
            sample = 0.0f;
        } else if (r.use_fm) {
            float fm_val = static_cast<float>(std::sin(fm_cp + r.fm_index * std::sin(fm_mp)));
            fm_cp += fm_c_inc * pitch_mult; fm_mp += fm_m_inc * pitch_mult;
            if (fm_cp >= TWO_PI) fm_cp -= TWO_PI;
            if (fm_mp >= TWO_PI) fm_mp -= TWO_PI;
            float osc1v = osc1.tick(sample_rate);
            sample = osc1v * static_cast<float>(1.0 - r.osc_mix) + fm_val * static_cast<float>(r.osc_mix);
        } else {
            float osc1v = osc1.tick(sample_rate);
            float osc2v = osc2.tick(sample_rate);
            sample = osc1v * static_cast<float>(1.0 - r.osc_mix) + osc2v * static_cast<float>(r.osc_mix);
        }

        double n_amt = r.noise_mix + noise_boost + art_extra_noise;
        if (r.noise_attack_burst > 0.0 && t < 0.02) n_amt += r.noise_attack_burst * (1.0 - t / 0.02);
        if (n_amt > 0.0) {
            float nz = r.pink_noise ? noise.tick_pink() : noise.tick_white();
            sample = static_cast<float>(sample * (1.0 - std::min(1.0, n_amt)) + nz * std::min(1.0, n_amt));
        }

        if (r.filter_env_amount != 0.0) {
            double sweep = r.filter_env_amount * (1.0 - t / duration);
            filt.cutoff = std::max(60.0, r.filter_cutoff * filter_scale + sweep);
        }
        sample = filt.tick(sample, sample_rate);

        mono[i] = sample * static_cast<float>(velocity);
    }

    DAHDSR env = r.envelope;
    env.decay   *= decay_scale;
    env.release *= decay_scale;
    if (percussive) {
        env.attack  = std::min(env.attack, 0.006);
        env.hold    = 0.0;
        env.decay   = duration * 0.72 * decay_scale;
        env.sustain = 0.04;
        env.release = duration * 0.28 * decay_scale;
    }
    env.apply(mono, duration, -1.0, sample_rate);

    if (r.formant1_gain_db != 0.0) {
        BiquadEQ f1(BiquadEQ::Type::Peak, r.formant1_hz, r.formant1_gain_db, r.formant1_q);
        f1.process(mono, sample_rate);
    }
    if (r.formant2_gain_db != 0.0) {
        BiquadEQ f2(BiquadEQ::Type::Peak, r.formant2_hz, r.formant2_gain_db, r.formant2_q);
        f2.process(mono, sample_rate);
    }

    apply_gain(mono, static_cast<float>(r.gain));
    clamp_buffer(mono);

    StereoBuffer out = pan_mono(mono, 0.0f, sample_rate);
    if (r.stereo_width > 0.0) {
        StereoBuffer widened = haas_widen(mono, r.stereo_width * 18.0, sample_rate);
        for (size_t i = 0; i < out.size(); ++i) {
            out.R[i] = out.R[i] * static_cast<float>(1.0 - r.stereo_width) +
                       widened.R[i] * static_cast<float>(r.stereo_width);
        }
    }
    return out;
}

class ModeledInstrument : public SampledInstrument {
public:
    TimbreRecipe recipe;

    explicit ModeledInstrument(std::string instrument_name = "") {
        name = std::move(instrument_name);
        fallback_model = [this](const Note& note, int sample_rate) {
            return render_timbre(this->recipe, note, sample_rate);
        };
    }
};

class KeyboardInstrument : public ModeledInstrument {
public:
    explicit KeyboardInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Soft, Articulation::Hard };
        recipe.osc1_shape = WaveShape::Triangle;
        recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.25; recipe.detune_cents = 3.0;
        recipe.noise_attack_burst = 0.05;
        recipe.envelope = DAHDSR{0.0, 0.004, 0.0, 0.9, 0.0, 0.35};
        recipe.filter_cutoff = 6500.0; recipe.filter_q = 0.7;
        recipe.stereo_width = 0.15;
    }
};

class BowedStringInstrument : public ModeledInstrument {
public:
    explicit BowedStringInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Pizzicato,
                           Articulation::Staccato, Articulation::Tremolo, Articulation::Harmonic,
                           Articulation::Spiccato, Articulation::SulPonticello, Articulation::ColLegno };
        recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.osc2_shape = WaveShape::BL_Sawtooth;
        recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.4; recipe.detune_cents = 6.0;
        recipe.noise_mix = 0.03; recipe.noise_attack_burst = 0.05; recipe.pink_noise = true;
        recipe.envelope = DAHDSR{0.0, 0.06, 0.0, 0.15, 0.82, 0.25};
        recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 18.0; recipe.vibrato_delay_sec = 0.2;
        recipe.filter_cutoff = 5200.0; recipe.filter_q = 0.9;
        recipe.formant1_hz = 800.0; recipe.formant1_gain_db = 3.0; recipe.formant1_q = 2.0;
        recipe.stereo_width = 0.22;
        recipe.art_decay_scale[Articulation::Pizzicato] = 0.22;
        recipe.art_decay_scale[Articulation::Staccato]  = 0.35;
    }
};

class PluckedStringInstrument : public ModeledInstrument {
public:
    explicit PluckedStringInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::PalmMute,
                           Articulation::Harmonic, Articulation::Muted, Articulation::Staccato };
        recipe.percussive = true;
        recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.osc2_shape = WaveShape::Triangle;
        recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.2; recipe.detune_cents = 2.0;
        recipe.noise_attack_burst = 0.22;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.6, 0.02, 0.25};
        recipe.filter_cutoff = 4200.0; recipe.filter_q = 0.7; recipe.filter_env_amount = -1800.0;
        recipe.stereo_width = 0.2;
        recipe.art_filter_scale[Articulation::PalmMute] = 0.35;
        recipe.art_filter_scale[Articulation::Muted]     = 0.3;
    }
};

class WoodwindInstrument : public ModeledInstrument {
public:
    explicit WoodwindInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Staccato,
                           Articulation::FlutterTongue, Articulation::Legato };
        recipe.osc1_shape = WaveShape::Sine;
        recipe.osc2_shape = WaveShape::Triangle;
        recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.3; recipe.detune_cents = 1.5;
        recipe.noise_mix = 0.05; recipe.noise_attack_burst = 0.08; recipe.pink_noise = false;
        recipe.envelope = DAHDSR{0.0, 0.045, 0.0, 0.08, 0.85, 0.16};
        recipe.vibrato_rate_hz = 5.0; recipe.vibrato_depth_cents = 12.0; recipe.vibrato_delay_sec = 0.25;
        recipe.filter_cutoff = 4500.0; recipe.filter_q = 0.75;
        recipe.stereo_width = 0.14;
    }
};

class BrassInstrument : public ModeledInstrument {
public:
    explicit BrassInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Staccato,
                           Articulation::Fall, Articulation::Doit, Articulation::Muted, Articulation::Marcato,
                           Articulation::ShortStab };
        recipe.osc1_shape = WaveShape::BL_Square;
        recipe.osc2_shape = WaveShape::BL_Sawtooth;
        recipe.osc2_ratio = 1.0; recipe.osc_mix = 0.35; recipe.detune_cents = 5.0;
        recipe.noise_mix = 0.015; recipe.noise_attack_burst = 0.1;
        recipe.envelope = DAHDSR{0.0, 0.03, 0.0, 0.1, 0.85, 0.14};
        recipe.pitch_attack_cents = 35.0; recipe.pitch_attack_time = 0.045;
        recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 8.0; recipe.vibrato_delay_sec = 0.3;
        recipe.filter_cutoff = 5800.0; recipe.filter_q = 1.1;
        recipe.formant1_hz = 1200.0; recipe.formant1_gain_db = 4.0; recipe.formant1_q = 2.5;
        recipe.stereo_width = 0.16;
        recipe.art_filter_scale[Articulation::Muted] = 0.4;
    }
};

class SaxophoneInstrument : public WoodwindInstrument {
public:
    explicit SaxophoneInstrument(std::string n) : WoodwindInstrument(std::move(n)) {
        recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.osc_mix = 0.4; recipe.noise_mix = 0.09; recipe.noise_attack_burst = 0.14;
        recipe.filter_cutoff = 5200.0; recipe.filter_q = 1.0;
        recipe.formant1_hz = 1100.0; recipe.formant1_gain_db = 5.0; recipe.formant1_q = 2.2;
        recipe.formant2_hz = 2600.0; recipe.formant2_gain_db = 3.0; recipe.formant2_q = 2.5;
    }
};

class MalletInstrument : public ModeledInstrument {
public:
    explicit MalletInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Tremolo, Articulation::Muted };
        recipe.percussive = true;
        recipe.use_fm = true; recipe.fm_ratio = 3.01; recipe.fm_index = 1.4; recipe.osc_mix = 0.85;
        recipe.osc1_shape = WaveShape::Sine;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 1.4, 0.0, 0.4};
        recipe.filter_cutoff = 9000.0; recipe.filter_q = 0.6;
        recipe.stereo_width = 0.25;
        recipe.art_decay_scale[Articulation::Muted] = 0.2;
    }
};

class WorldInstrument : public ModeledInstrument {
public:
    explicit WorldInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Staccato, Articulation::Tremolo };
        recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.osc2_shape = WaveShape::Sine;
        recipe.osc_mix = 0.3; recipe.detune_cents = 4.0;
        recipe.noise_mix = 0.04;
        recipe.envelope = DAHDSR{0.0, 0.02, 0.0, 0.2, 0.65, 0.2};
        recipe.filter_cutoff = 5000.0; recipe.filter_q = 0.85;
        recipe.stereo_width = 0.2;
    }
};

class PercussionInstrument : public ModeledInstrument {
public:
    explicit PercussionInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Rimshot, Articulation::Accent };
        recipe.percussive = true;
        recipe.osc1_shape = WaveShape::Sine;
        recipe.noise_mix = 0.5; recipe.pink_noise = false;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.12, 0.0, 0.06};
        recipe.filter_cutoff = 3000.0; recipe.filter_q = 0.7;
        recipe.stereo_width = 0.1;
    }

    PlayedNote hit(Dynamics dyn = Dynamics::mf, double duration = 0.4,
                   Articulation art = Articulation::Sustain, int sample_rate = 0) const
    {
        return play(60, duration, dyn, art, sample_rate);
    }
};

class VocalInstrument : public ModeledInstrument {
public:
    explicit VocalInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Legato, Articulation::Whisper,
                           Articulation::Breath, Articulation::Humming, Articulation::Staccato };
        recipe.osc1_shape = WaveShape::Sine;
        recipe.osc2_shape = WaveShape::Triangle;
        recipe.osc_mix = 0.25; recipe.detune_cents = 6.0;
        recipe.noise_mix = 0.025;
        recipe.envelope = DAHDSR{0.0, 0.09, 0.0, 0.12, 0.8, 0.3};
        recipe.vibrato_rate_hz = 5.4; recipe.vibrato_depth_cents = 14.0; recipe.vibrato_delay_sec = 0.35;
        recipe.filter_cutoff = 3800.0; recipe.filter_q = 0.8;
        recipe.formant1_hz = 700.0;  recipe.formant1_gain_db = 6.0; recipe.formant1_q = 3.0;
        recipe.formant2_hz = 1200.0; recipe.formant2_gain_db = 4.0; recipe.formant2_q = 3.0;
        recipe.stereo_width = 0.3;
    }
};

class SynthInstrument : public ModeledInstrument {
public:
    explicit SynthInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Staccato };
        recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.osc2_shape = WaveShape::BL_Square;
        recipe.osc_mix = 0.45; recipe.detune_cents = 7.0;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 0.15, 0.7, 0.2};
        recipe.filter_cutoff = 4000.0; recipe.filter_q = 1.0; recipe.filter_env_amount = 1500.0;
        recipe.stereo_width = 0.28;
    }
};

class GuitarInstrument : public PluckedStringInstrument {
public:
    explicit GuitarInstrument(std::string n) : PluckedStringInstrument(std::move(n)) {}
};

class ExperimentalInstrument : public ModeledInstrument {
public:
    explicit ExperimentalInstrument(std::string n) : ModeledInstrument(std::move(n)) {
        articulations = { Articulation::Sustain, Articulation::Tremolo, Articulation::Whisper };
        recipe.osc1_shape = WaveShape::Sine;
        recipe.osc2_shape = WaveShape::Triangle;
        recipe.osc_mix = 0.3;
        recipe.noise_mix = 0.06;
        recipe.envelope = DAHDSR{0.0, 0.3, 0.0, 0.3, 0.6, 0.8};
        recipe.pitch_instability = 18.0;
        recipe.vibrato_rate_hz = 4.0; recipe.vibrato_depth_cents = 10.0; recipe.vibrato_delay_sec = 0.1;
        recipe.filter_cutoff = 3500.0; recipe.stereo_width = 0.35;
    }
};

class DrumKit {
public:
    std::string name;

    explicit DrumKit(std::string kit_name) : name(std::move(kit_name)) {}
    virtual ~DrumKit() = default;

    void map(int midi_note, std::shared_ptr<Instrument> instr) {
        mapping_[midi_note] = std::move(instr);
    }

    bool has(int midi_note) const { return mapping_.find(midi_note) != mapping_.end(); }

    PlayedNote play(int midi_note, Dynamics dyn = Dynamics::mf, double duration = 0.4,
                     int sample_rate = 0) const
    {
        auto it = mapping_.find(midi_note);
        if (it == mapping_.end() || !it->second)
            return PlayedNote{ StereoBuffer(), sample_rate > 0 ? sample_rate : global_config().sample_rate };
        return it->second->play(midi_note, duration, dyn, Articulation::Sustain, sample_rate);
    }

    StereoBuffer render_pattern(const std::vector<std::pair<double,int>>& hits,
                                 double total_duration, Dynamics dyn = Dynamics::mf,
                                 int sample_rate = 0) const
    {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        StereoBuffer out = make_stereo(total_duration, sample_rate);
        for (auto& h : hits) {
            PlayedNote pn = play(h.second, dyn, 0.4, sample_rate);
            const size_t off = static_cast<size_t>(h.first * sample_rate);
            for (size_t i = 0; i < pn.audio.size() && off + i < out.size(); ++i) {
                out.L[off + i] += pn.audio.L[i];
                out.R[off + i] += pn.audio.R[i];
            }
        }
        return out;
    }

protected:
    std::unordered_map<int, std::shared_ptr<Instrument>> mapping_;
};

}
}
