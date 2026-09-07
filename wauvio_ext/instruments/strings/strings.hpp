#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class SoloViolin : public BowedStringInstrument {
public:
    SoloViolin() : BowedStringInstrument("Solo Violin") {
        recipe.filter_cutoff = 7000.0; recipe.formant1_hz = 1200.0;
    }
};

class SoloViola : public BowedStringInstrument {
public:
    SoloViola() : BowedStringInstrument("Solo Viola") {
        recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 850.0; recipe.vibrato_depth_cents = 16.0;
    }
};
using Viola = SoloViola;

class SoloCello : public BowedStringInstrument {
public:
    SoloCello() : BowedStringInstrument("Solo Cello") {
        recipe.filter_cutoff = 3400.0; recipe.formant1_hz = 400.0; recipe.vibrato_rate_hz = 4.8;
        recipe.vibrato_depth_cents = 20.0;
    }
};
using Cello = SoloCello;

class DoubleBass : public BowedStringInstrument {
public:
    DoubleBass() : BowedStringInstrument("Double Bass") {
        recipe.filter_cutoff = 1600.0; recipe.formant1_hz = 180.0; recipe.vibrato_rate_hz = 4.0;
        recipe.vibrato_depth_cents = 14.0; recipe.osc_mix = 0.5;
    }
};

class UprightBass : public BowedStringInstrument {
public:
    UprightBass() : BowedStringInstrument("Upright Bass") {
        recipe.filter_cutoff = 1400.0; recipe.formant1_hz = 150.0; recipe.osc_mix = 0.55;
        recipe.art_decay_scale[Articulation::Pizzicato] = 0.3;
    }
};

class StringEnsemble : public BowedStringInstrument {
public:
    StringEnsemble() : BowedStringInstrument("String Ensemble") {
        recipe.detune_cents = 14.0; recipe.stereo_width = 0.4; recipe.filter_cutoff = 5000.0;
        recipe.envelope.attack = 0.15;
    }
};

class ChamberStrings : public BowedStringInstrument {
public:
    ChamberStrings() : BowedStringInstrument("Chamber Strings") {
        recipe.detune_cents = 9.0; recipe.stereo_width = 0.3; recipe.filter_cutoff = 5600.0;
        recipe.envelope.attack = 0.08;
    }
};

class FullStringSection : public BowedStringInstrument {
public:
    FullStringSection() : BowedStringInstrument("Full String Section") {
        recipe.detune_cents = 20.0; recipe.stereo_width = 0.5; recipe.filter_cutoff = 4800.0;
        recipe.envelope.attack = 0.2; recipe.gain = 1.0;
    }
};
using FullStringEnsemble = FullStringSection;

class SymphonicStrings : public BowedStringInstrument {
public:
    SymphonicStrings() : BowedStringInstrument("Symphonic Strings") {
        recipe.detune_cents = 26.0; recipe.stereo_width = 0.6; recipe.filter_cutoff = 5200.0;
        recipe.envelope.attack = 0.25; recipe.gain = 1.05;
    }
};

class StringQuartet : public BowedStringInstrument {
public:
    StringQuartet() : BowedStringInstrument("String Quartet") {
        recipe.detune_cents = 7.0; recipe.stereo_width = 0.28; recipe.filter_cutoff = 5800.0;
        recipe.envelope.attack = 0.05;
    }
};

class ViolaDaGamba : public BowedStringInstrument {
public:
    ViolaDaGamba() : BowedStringInstrument("Viola da Gamba") {
        recipe.filter_cutoff = 3000.0; recipe.formant1_hz = 500.0; recipe.noise_mix = 0.05;
        recipe.vibrato_depth_cents = 8.0;
    }
};

class CelloDaSpalla : public BowedStringInstrument {
public:
    CelloDaSpalla() : BowedStringInstrument("Cello da Spalla") {
        recipe.filter_cutoff = 3800.0; recipe.formant1_hz = 550.0; recipe.noise_mix = 0.045;
        recipe.vibrato_depth_cents = 10.0;
    }
};

class HardangerFiddle : public BowedStringInstrument {
public:
    HardangerFiddle() : BowedStringInstrument("Hardanger Fiddle") {
        recipe.filter_cutoff = 7500.0; recipe.formant1_hz = 1400.0; recipe.detune_cents = 18.0;
        recipe.stereo_width = 0.3; recipe.formant2_hz = 2600.0; recipe.formant2_gain_db = 3.0;
    }
};

class Nyckelharpa : public BowedStringInstrument {
public:
    Nyckelharpa() : BowedStringInstrument("Nyckelharpa") {
        recipe.filter_cutoff = 6800.0; recipe.formant1_hz = 1200.0; recipe.detune_cents = 22.0;
        recipe.stereo_width = 0.32; recipe.noise_mix = 0.04;
    }
};

class Zhonghu : public BowedStringInstrument {
public:
    Zhonghu() : BowedStringInstrument("Zhonghu") {
        recipe.filter_cutoff = 3800.0; recipe.formant1_hz = 750.0; recipe.vibrato_depth_cents = 24.0;
        recipe.osc_mix = 0.5;
    }
};

class BaroqueViolin : public BowedStringInstrument {
public:
    BaroqueViolin() : BowedStringInstrument("Baroque Violin") {
        recipe.filter_cutoff = 5800.0; recipe.formant1_hz = 1000.0; recipe.noise_mix = 0.05;
        recipe.vibrato_depth_cents = 6.0;
    }
};

class BaroqueCello : public BowedStringInstrument {
public:
    BaroqueCello() : BowedStringInstrument("Baroque Cello") {
        recipe.filter_cutoff = 2900.0; recipe.formant1_hz = 380.0; recipe.noise_mix = 0.05;
        recipe.vibrato_depth_cents = 6.0;
    }
};

class ViolinHarmonics : public BowedStringInstrument {
public:
    ViolinHarmonics() : BowedStringInstrument("Violin Harmonics") {
        recipe.filter_cutoff = 9000.0; recipe.formant1_hz = 1600.0; recipe.gain = 0.6;
        recipe.art_pitch_shift[Articulation::Sustain] = 12.0;
    }
};

class CelloHarmonics : public BowedStringInstrument {
public:
    CelloHarmonics() : BowedStringInstrument("Cello Harmonics") {
        recipe.filter_cutoff = 6000.0; recipe.formant1_hz = 900.0; recipe.gain = 0.6;
        recipe.art_pitch_shift[Articulation::Sustain] = 12.0;
    }
};

class TremoloStrings : public BowedStringInstrument {
public:
    TremoloStrings() : BowedStringInstrument("Tremolo Strings") {
        recipe.detune_cents = 16.0; recipe.stereo_width = 0.4;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Tremolo;
        return BowedStringInstrument::render_note(n, sample_rate);
    }
};

class PizzicatoStrings : public BowedStringInstrument {
public:
    PizzicatoStrings() : BowedStringInstrument("Pizzicato Strings") {
        recipe.detune_cents = 10.0; recipe.stereo_width = 0.3;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Pizzicato;
        return BowedStringInstrument::render_note(n, sample_rate);
    }
};

class SpiccatoStrings : public BowedStringInstrument {
public:
    SpiccatoStrings() : BowedStringInstrument("Spiccato Strings") {
        recipe.detune_cents = 12.0; recipe.stereo_width = 0.32;
    }
    StereoBuffer render_note(const audio::Note& note, int sample_rate = 0) const override {
        audio::Note n = note;
        if (n.articulation == Articulation::Sustain) n.articulation = Articulation::Spiccato;
        return BowedStringInstrument::render_note(n, sample_rate);
    }
};

}
}
