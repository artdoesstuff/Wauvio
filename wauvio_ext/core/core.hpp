#pragma once

#include "../../wauvio.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <stdexcept>

namespace wauvio {
namespace audio {

enum class Dynamics { ppp, pp, p, mp, mf, f, ff, fff };

inline double dynamics_to_velocity(Dynamics d) noexcept {
    switch (d) {
        case Dynamics::ppp: return 0.12;
        case Dynamics::pp:  return 0.24;
        case Dynamics::p:   return 0.38;
        case Dynamics::mp:  return 0.52;
        case Dynamics::mf:  return 0.66;
        case Dynamics::f:   return 0.80;
        case Dynamics::ff:  return 0.92;
        case Dynamics::fff: return 1.00;
    }
    return 0.66;
}

inline Dynamics velocity_to_dynamics(double v) noexcept {
    if (v < 0.18) return Dynamics::ppp;
    if (v < 0.31) return Dynamics::pp;
    if (v < 0.45) return Dynamics::p;
    if (v < 0.59) return Dynamics::mp;
    if (v < 0.73) return Dynamics::mf;
    if (v < 0.86) return Dynamics::f;
    if (v < 0.96) return Dynamics::ff;
    return Dynamics::fff;
}

enum class Articulation {
    Sustain,
    Legato,
    Staccato,
    Marcato,
    Accent,
    Soft,
    Hard,
    Pizzicato,
    Tremolo,
    PalmMute,
    Harmonic,
    Muted,
    Fall,
    Doit,
    FlutterTongue,
    Rimshot,
    Spiccato,
    SulPonticello,
    ColLegno,
    ShortStab,
    Whisper,
    Breath,
    Humming
};

inline const char* to_string(Articulation a) noexcept {
    switch (a) {
        case Articulation::Sustain:       return "Sustain";
        case Articulation::Legato:        return "Legato";
        case Articulation::Staccato:      return "Staccato";
        case Articulation::Marcato:       return "Marcato";
        case Articulation::Accent:        return "Accent";
        case Articulation::Soft:          return "Soft";
        case Articulation::Hard:          return "Hard";
        case Articulation::Pizzicato:     return "Pizzicato";
        case Articulation::Tremolo:       return "Tremolo";
        case Articulation::PalmMute:      return "PalmMute";
        case Articulation::Harmonic:      return "Harmonic";
        case Articulation::Muted:         return "Muted";
        case Articulation::Fall:          return "Fall";
        case Articulation::Doit:          return "Doit";
        case Articulation::FlutterTongue: return "FlutterTongue";
        case Articulation::Rimshot:       return "Rimshot";
        case Articulation::Spiccato:      return "Spiccato";
        case Articulation::SulPonticello: return "SulPonticello";
        case Articulation::ColLegno:      return "ColLegno";
        case Articulation::ShortStab:     return "ShortStab";
        case Articulation::Whisper:       return "Whisper";
        case Articulation::Breath:        return "Breath";
        case Articulation::Humming:       return "Humming";
    }
    return "Sustain";
}

struct TimeSignature {
    int beats_per_bar = 4;
    int beat_unit     = 4;
    TimeSignature() = default;
    TimeSignature(int bpb, int bu) : beats_per_bar(bpb), beat_unit(bu) {}
};

struct Tempo {
    double bpm = 120.0;
    Tempo() = default;
    explicit Tempo(double b) : bpm(b) {}

    double beat_seconds() const noexcept { return 60.0 / bpm; }

    double beats_to_seconds(double beats) const noexcept { return beats * beat_seconds(); }
};

constexpr int REST_NOTE = -1;

struct Note {
    int          midi_note   = 60;
    double       duration    = 0.5;
    Dynamics     dynamics    = Dynamics::mf;
    Articulation articulation = Articulation::Sustain;
    double       pan         = 0.0;

    double       expression          = 1.0;
    double       pitch_bend_semitones = 0.0;
    int          glide_from_midi     = -1;
    double       glide_time          = 0.08;

    Note() = default;
    Note(int midi, double dur, Dynamics dyn = Dynamics::mf,
         Articulation art = Articulation::Sustain, double p = 0.0)
        : midi_note(midi), duration(dur), dynamics(dyn), articulation(art), pan(p) {}

    bool is_rest() const noexcept { return midi_note < 0; }
};

inline Note rest(double duration) { return Note(REST_NOTE, duration); }

struct Chord {
    std::vector<int> midi_notes;
    double           duration    = 0.5;
    Dynamics         dynamics    = Dynamics::mf;
    Articulation     articulation = Articulation::Sustain;

    Chord() = default;
    Chord(std::vector<int> notes, double dur, Dynamics dyn = Dynamics::mf,
          Articulation art = Articulation::Sustain)
        : midi_notes(std::move(notes)), duration(dur), dynamics(dyn), articulation(art) {}
};

using Melody = std::vector<Note>;

namespace scale {

inline std::vector<int> build(int root, const std::vector<int>& steps, int octaves = 1) {
    std::vector<int> out;
    int note = root;
    for (int o = 0; o < octaves; ++o) {
        for (size_t i = 0; i < steps.size(); ++i) {
            out.push_back(note);
            note += steps[i];
        }
    }
    out.push_back(note);
    return out;
}

inline std::vector<int> major(int root, int octaves = 1)          { return build(root, {2,2,1,2,2,2,1}, octaves); }
inline std::vector<int> natural_minor(int root, int octaves = 1)  { return build(root, {2,1,2,2,1,2,2}, octaves); }
inline std::vector<int> harmonic_minor(int root, int octaves = 1) { return build(root, {2,1,2,2,1,3,1}, octaves); }
inline std::vector<int> major_pentatonic(int root, int octaves=1) { return build(root, {2,2,3,2,3}, octaves); }
inline std::vector<int> minor_pentatonic(int root, int octaves=1) { return build(root, {3,2,2,3,2}, octaves); }
inline std::vector<int> blues(int root, int octaves = 1)          { return build(root, {3,2,1,1,3,2}, octaves); }
inline std::vector<int> chromatic(int root, int octaves = 1)      { return build(root, {1,1,1,1,1,1,1,1,1,1,1,1}, octaves); }
inline std::vector<int> dorian(int root, int octaves = 1)         { return build(root, {2,1,2,2,2,1,2}, octaves); }
inline std::vector<int> mixolydian(int root, int octaves = 1)     { return build(root, {2,2,1,2,2,1,2}, octaves); }

}

struct PlayedNote {
    StereoBuffer audio;
    int          sample_rate = 44100;

    Buffer to_mono() const { return audio.to_mono(); }
    double duration_seconds() const {
        return sample_rate > 0 ? static_cast<double>(audio.size()) / sample_rate : 0.0;
    }
};

class Instrument {
public:
    std::string name;
    float       gain = 1.0f;
    float       pan  = 0.0f;

    bool     reverb_on     = false;  Reverb           reverb_fx;
    bool     chorus_on     = false;  Chorus           chorus_fx;
    bool     delay_on      = false;  DelayLine        delay_fx;
    bool     eq_on         = false;  ParametricEQ     eq_fx;
    bool     distortion_on = false;  float            distortion_drive = 2.0f;

    virtual ~Instrument() = default;

    virtual StereoBuffer render_note(const Note& note, int sample_rate = 0) const = 0;

    virtual std::vector<Articulation> supported_articulations() const {
        return { Articulation::Sustain };
    }

    bool supports(Articulation a) const {
        auto v = supported_articulations();
        return std::find(v.begin(), v.end(), a) != v.end();
    }

    PlayedNote play(int midi_note, double duration,
                     Dynamics dyn = Dynamics::mf,
                     Articulation art = Articulation::Sustain,
                     int sample_rate = 0) const
    {
        return play(Note(midi_note, duration, dyn, art), sample_rate);
    }

    PlayedNote play(const Note& note, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        StereoBuffer buf = render_note(note, sample_rate);
        buf = apply_fx_chain(std::move(buf), sample_rate);
        apply_gain(buf, gain);
        if (pan != 0.0f) {
            Buffer m = buf.to_mono();
            buf = pan_mono(m, pan, sample_rate);
        }
        return PlayedNote{ std::move(buf), sample_rate };
    }

    PlayedNote play_chord(const Chord& chord, int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        StereoBuffer out = make_stereo(chord.duration, sample_rate);
        for (int m : chord.midi_notes) {
            Note n(m, chord.duration, chord.dynamics, chord.articulation);
            StereoBuffer nb = render_note(n, sample_rate);
            mix_into(out, nb, 1.0f / std::max(1.0f, static_cast<float>(chord.midi_notes.size()) * 0.6f + 0.4f));
        }
        out = apply_fx_chain(std::move(out), sample_rate);
        apply_gain(out, gain);
        return PlayedNote{ std::move(out), sample_rate };
    }

    Instrument& reverb(float room_size = 0.6f, float wet = 0.3f, float damping = 0.5f) {
        reverb_fx.room_size = room_size; reverb_fx.wet = wet; reverb_fx.damping = damping;
        reverb_on = true; return *this;
    }
    Instrument& chorus(double rate_hz = 0.5, double depth_ms = 1.5, float wet = 0.4f) {
        chorus_fx.rate_hz = rate_hz; chorus_fx.depth_ms = depth_ms; chorus_fx.wet = wet;
        chorus_on = true; return *this;
    }
    Instrument& delay(double time_ms = 250.0, float feedback = 0.35f, float wet = 0.3f) {
        delay_fx.time_ms = time_ms; delay_fx.feedback = feedback; delay_fx.wet = wet;
        delay_on = true; return *this;
    }
    Instrument& eq(ParametricEQ e) { eq_fx = e; eq_on = true; return *this; }
    Instrument& distortion(float drive = 2.0f) { distortion_drive = drive; distortion_on = true; return *this; }

protected:
    StereoBuffer apply_fx_chain(StereoBuffer buf, int sample_rate) const {
        if (distortion_on) {
            for (auto& s : buf.L) s = distortion::soft_clip(s, distortion_drive);
            for (auto& s : buf.R) s = distortion::soft_clip(s, distortion_drive);
        }
        if (eq_on)     { ParametricEQ e = eq_fx; e.process(buf, sample_rate); }
        if (chorus_on) { Chorus c = chorus_fx; c.process(buf.L); Chorus c2 = chorus_fx; c2.process(buf.R); }
        if (delay_on)  { DelayLine d1 = delay_fx; d1.process(buf.L); DelayLine d2 = delay_fx; d2.process(buf.R); }
        if (reverb_on) { Reverb r1 = reverb_fx; r1.process(buf.L); Reverb r2 = reverb_fx; r2.process(buf.R); }
        return buf;
    }
};

using InstrumentPtr = std::shared_ptr<Instrument>;

class Track {
public:
    Track() = default;
    explicit Track(const Instrument& instr) : instrument_(&instr) {}

    Track& add(const Note& n)      { entries_.push_back({n, false}); return *this; }
    Track& add(const Melody& mel)  { for (auto& n : mel) entries_.push_back({n, false}); return *this; }
    Track& add_chord(const Chord& c) { entries_.push_back({Note(REST_NOTE, c.duration), true, c}); return *this; }
    Track& rest(double duration)   { entries_.push_back({audio::rest(duration), false}); return *this; }

    double total_duration() const {
        double t = 0.0;
        for (auto& e : entries_) t += e.note.duration;
        return t;
    }

    StereoBuffer render(int sample_rate = 0) const {
        if (!instrument_) throw std::runtime_error("Track::render: no instrument attached");
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        StereoBuffer out = make_stereo(total_duration(), sample_rate);
        size_t off = 0;
        for (auto& e : entries_) {
            StereoBuffer nb = e.is_chord
                ? instrument_->play_chord(e.chord, sample_rate).audio
                : instrument_->play(e.note, sample_rate).audio;
            for (size_t i = 0; i < nb.size() && off + i < out.size(); ++i) {
                out.L[off + i] += nb.L[i];
                out.R[off + i] += nb.R[i];
            }
            off += static_cast<size_t>(e.note.duration * sample_rate);
        }
        return out;
    }

    const Instrument* instrument() const { return instrument_; }

private:
    struct Entry { Note note; bool is_chord = false; Chord chord = Chord({}, 0.0); };
    const Instrument*  instrument_ = nullptr;
    std::vector<Entry> entries_;
};

class Arrangement {
public:
    Tempo          tempo;
    TimeSignature  time_signature;

    Track& add(const Instrument& instr, const Melody& melody,
               double start_time = 0.0, float track_gain = 1.0f)
    {
        tracks_.push_back(std::make_unique<Track>(instr));
        tracks_.back()->add(melody);
        placements_.push_back({start_time, track_gain});
        return *tracks_.back();
    }

    void add_track(const Track& t, double start_time = 0.0, float track_gain = 1.0f) {
        tracks_.push_back(std::make_unique<Track>(t));
        placements_.push_back({start_time, track_gain});
    }

    Arrangement& limiter(float threshold = 0.9f) { limiter_on_ = true; limiter_threshold_ = threshold; return *this; }
    Arrangement& master_reverb(float room = 0.5f, float wet = 0.2f) {
        master_reverb_on_ = true; master_reverb_.room_size = room; master_reverb_.wet = wet; return *this;
    }

    StereoBuffer render(int sample_rate = 0) const {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        MasterBus bus(sample_rate);
        for (size_t i = 0; i < tracks_.size(); ++i) {
            StereoBuffer rendered = tracks_[i]->render(sample_rate);
            bus.schedule(std::move(rendered), placements_[i].start_time, placements_[i].gain);
        }
        StereoBuffer out = bus.mix_stereo();
        if (master_reverb_on_) {
            Reverb rL = master_reverb_, rR = master_reverb_;
            rL.process(out.L); rR.process(out.R);
        }
        if (limiter_on_) {
            for (auto& s : out.L) s = distortion::soft_clip(s, 1.0f / std::max(0.05f, limiter_threshold_));
            for (auto& s : out.R) s = distortion::soft_clip(s, 1.0f / std::max(0.05f, limiter_threshold_));
        }
        normalize(out, 0.95f);
        return out;
    }

private:
    struct Placement { double start_time; float gain; };
    std::vector<std::unique_ptr<Track>> tracks_;
    std::vector<Placement>              placements_;
    bool   limiter_on_ = false;
    float  limiter_threshold_ = 0.9f;
    bool   master_reverb_on_ = false;
    Reverb master_reverb_;
};

}
}
