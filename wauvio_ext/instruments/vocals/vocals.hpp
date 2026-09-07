#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class Choir : public VocalInstrument {
public:
    Choir() : VocalInstrument("Choir") {
        recipe.detune_cents = 16.0; recipe.stereo_width = 0.45; recipe.envelope.attack = 0.15;
    }
};
using VocalEnsemble = Choir;

class MaleChoir : public VocalInstrument {
public:
    MaleChoir() : VocalInstrument("Male Choir") {
        recipe.formant1_hz = 550.0; recipe.detune_cents = 12.0; recipe.filter_cutoff = 3000.0;
    }
};

class FemaleChoir : public VocalInstrument {
public:
    FemaleChoir() : VocalInstrument("Female Choir") {
        recipe.formant1_hz = 900.0; recipe.detune_cents = 12.0; recipe.filter_cutoff = 4600.0;
    }
};

class ChildrensChoir : public VocalInstrument {
public:
    ChildrensChoir() : VocalInstrument("Children's Choir") {
        recipe.formant1_hz = 1200.0; recipe.detune_cents = 10.0; recipe.filter_cutoff = 5200.0;
    }
};

class SATBChoir : public VocalInstrument {
public:
    SATBChoir() : VocalInstrument("SATB Choir") {
        recipe.formant1_hz = 700.0; recipe.detune_cents = 22.0; recipe.stereo_width = 0.55;
        recipe.envelope.attack = 0.12;
    }
};

class GregorianChoir : public VocalInstrument {
public:
    GregorianChoir() : VocalInstrument("Gregorian Choir") {
        recipe.formant1_hz = 500.0; recipe.detune_cents = 8.0; recipe.stereo_width = 0.6;
        recipe.envelope = DAHDSR{0.0, 0.3, 0.0, 0.3, 0.85, 1.0}; recipe.filter_cutoff = 2600.0;
    }
};

class VocalPad : public VocalInstrument {
public:
    VocalPad() : VocalInstrument("Vocal Pad") {
        recipe.detune_cents = 20.0; recipe.stereo_width = 0.55; recipe.envelope.attack = 0.6;
        recipe.envelope.release = 1.0;
    }
};

class SoloVocal : public VocalInstrument {
public:
    SoloVocal() : VocalInstrument("Solo Vocal") {
        recipe.formant1_hz = 750.0; recipe.vibrato_depth_cents = 18.0;
    }
};

class SoloMale : public VocalInstrument {
public:
    SoloMale() : VocalInstrument("Solo Male") {
        recipe.formant1_hz = 550.0; recipe.vibrato_depth_cents = 14.0; recipe.filter_cutoff = 3000.0;
    }
};

class SoloFemale : public VocalInstrument {
public:
    SoloFemale() : VocalInstrument("Solo Female") {
        recipe.formant1_hz = 950.0; recipe.vibrato_depth_cents = 18.0; recipe.filter_cutoff = 4600.0;
    }
};

class Soprano : public VocalInstrument {
public:
    Soprano() : VocalInstrument("Soprano") {
        recipe.formant1_hz = 1100.0; recipe.vibrato_depth_cents = 22.0; recipe.filter_cutoff = 5200.0;
    }
};

class Alto : public VocalInstrument {
public:
    Alto() : VocalInstrument("Alto") {
        recipe.formant1_hz = 800.0; recipe.vibrato_depth_cents = 18.0; recipe.filter_cutoff = 4000.0;
    }
};

class Tenor : public VocalInstrument {
public:
    Tenor() : VocalInstrument("Tenor") {
        recipe.formant1_hz = 650.0; recipe.vibrato_depth_cents = 16.0; recipe.filter_cutoff = 3400.0;
    }
};

class Baritone : public VocalInstrument {
public:
    Baritone() : VocalInstrument("Baritone") {
        recipe.formant1_hz = 500.0; recipe.vibrato_depth_cents = 14.0; recipe.filter_cutoff = 2600.0;
    }
};

class BassVoice : public VocalInstrument {
public:
    BassVoice() : VocalInstrument("Bass") {
        recipe.formant1_hz = 380.0; recipe.vibrato_depth_cents = 12.0; recipe.filter_cutoff = 2000.0;
    }
};

class VocalAah : public VocalInstrument {
public:
    VocalAah() : VocalInstrument("Vocal Aah") {
        recipe.formant1_hz = 800.0; recipe.formant1_gain_db = 7.0;
        recipe.formant2_hz = 1150.0; recipe.formant2_gain_db = 4.0;
    }
};
using VocalAh = VocalAah;

class VocalOoh : public VocalInstrument {
public:
    VocalOoh() : VocalInstrument("Vocal Ooh") {
        recipe.formant1_hz = 350.0; recipe.formant1_gain_db = 7.0;
        recipe.formant2_hz = 800.0; recipe.formant2_gain_db = 3.0; recipe.filter_cutoff = 2400.0;
    }
};
using VocalOo = VocalOoh;

class VocalEeh : public VocalInstrument {
public:
    VocalEeh() : VocalInstrument("Vocal Eeh") {
        recipe.formant1_hz = 500.0; recipe.formant1_gain_db = 6.0;
        recipe.formant2_hz = 2200.0; recipe.formant2_gain_db = 4.0; recipe.filter_cutoff = 3800.0;
    }
};

class VocalMmm : public VocalInstrument {
public:
    VocalMmm() : VocalInstrument("Vocal Mmm") {
        recipe.formant1_hz = 250.0; recipe.formant1_gain_db = 6.0; recipe.filter_cutoff = 1600.0;
        recipe.noise_mix = 0.0;
    }
};
using VocalMm = VocalMmm;

class VocalHumming : public VocalInstrument {
public:
    VocalHumming() : VocalInstrument("Vocal Humming") {
        recipe.formant1_hz = 300.0; recipe.formant1_gain_db = 5.0; recipe.filter_cutoff = 1400.0;
        recipe.noise_mix = 0.0; recipe.gain = 0.6;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Humming;
        return VocalInstrument::render_note(n, sample_rate);
    }
};

class VocalStaccato : public VocalInstrument {
public:
    VocalStaccato() : VocalInstrument("Vocal Staccato") {
        recipe.formant1_hz = 800.0; recipe.formant1_gain_db = 6.0;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Staccato;
        return VocalInstrument::render_note(n, sample_rate);
    }
};

class Whisper : public VocalInstrument {
public:
    Whisper() : VocalInstrument("Whisper") {
        recipe.noise_mix = 0.5; recipe.filter_cutoff = 3200.0; recipe.gain = 0.4;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Whisper;
        return VocalInstrument::render_note(n, sample_rate);
    }
};

class Breath : public VocalInstrument {
public:
    Breath() : VocalInstrument("Breath") {
        recipe.noise_mix = 0.7; recipe.filter_cutoff = 2600.0; recipe.gain = 0.35;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Breath;
        return VocalInstrument::render_note(n, sample_rate);
    }
};

}
}
