#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class Flute : public WoodwindInstrument {
public:
    Flute() : WoodwindInstrument("Flute") {
        recipe.filter_cutoff = 5000.0; recipe.noise_mix = 0.09; recipe.osc_mix = 0.15;
    }
};

class Piccolo : public WoodwindInstrument {
public:
    Piccolo() : WoodwindInstrument("Piccolo") {
        recipe.filter_cutoff = 8000.0; recipe.noise_mix = 0.1; recipe.osc_mix = 0.1;
    }
};

class AltoFlute : public WoodwindInstrument {
public:
    AltoFlute() : WoodwindInstrument("Alto Flute") {
        recipe.filter_cutoff = 3600.0; recipe.noise_mix = 0.08;
    }
};

class BassFlute : public WoodwindInstrument {
public:
    BassFlute() : WoodwindInstrument("Bass Flute") {
        recipe.filter_cutoff = 2400.0; recipe.noise_mix = 0.1;
    }
};

class ContrabassFlute : public WoodwindInstrument {
public:
    ContrabassFlute() : WoodwindInstrument("Contrabass Flute") {
        recipe.filter_cutoff = 1400.0; recipe.noise_mix = 0.12;
    }
};

class Oboe : public WoodwindInstrument {
public:
    Oboe() : WoodwindInstrument("Oboe") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.35; recipe.noise_mix = 0.04;
        recipe.formant1_hz = 1400.0; recipe.formant1_gain_db = 5.0; recipe.filter_cutoff = 5600.0;
    }
};

class BassOboe : public WoodwindInstrument {
public:
    BassOboe() : WoodwindInstrument("Bass Oboe") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.38; recipe.noise_mix = 0.045;
        recipe.formant1_hz = 800.0; recipe.formant1_gain_db = 4.5; recipe.filter_cutoff = 3200.0;
    }
};

class EnglishHorn : public WoodwindInstrument {
public:
    EnglishHorn() : WoodwindInstrument("English Horn") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.35; recipe.noise_mix = 0.045;
        recipe.formant1_hz = 1000.0; recipe.formant1_gain_db = 5.0; recipe.filter_cutoff = 4200.0;
    }
};

class Heckelphone : public WoodwindInstrument {
public:
    Heckelphone() : WoodwindInstrument("Heckelphone") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.4; recipe.noise_mix = 0.05;
        recipe.formant1_hz = 650.0; recipe.formant1_gain_db = 4.5; recipe.filter_cutoff = 2800.0;
    }
};

class Clarinet : public WoodwindInstrument {
public:
    Clarinet() : WoodwindInstrument("Clarinet") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.25; recipe.noise_mix = 0.03;
        recipe.filter_cutoff = 4200.0; recipe.formant1_hz = 900.0; recipe.formant1_gain_db = 3.0;
    }
};

class EbClarinet : public WoodwindInstrument {
public:
    EbClarinet() : WoodwindInstrument("E-flat Clarinet") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.22; recipe.noise_mix = 0.03;
        recipe.filter_cutoff = 5800.0; recipe.formant1_hz = 1200.0; recipe.formant1_gain_db = 3.0;
    }
};

class BassClarinet : public WoodwindInstrument {
public:
    BassClarinet() : WoodwindInstrument("Bass Clarinet") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.3; recipe.noise_mix = 0.035;
        recipe.filter_cutoff = 2400.0;
    }
};

class ContrabassClarinet : public WoodwindInstrument {
public:
    ContrabassClarinet() : WoodwindInstrument("Contrabass Clarinet") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.34; recipe.noise_mix = 0.04;
        recipe.filter_cutoff = 1400.0;
    }
};

class Bassoon : public WoodwindInstrument {
public:
    Bassoon() : WoodwindInstrument("Bassoon") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.4; recipe.noise_mix = 0.05;
        recipe.filter_cutoff = 2600.0; recipe.formant1_hz = 500.0; recipe.formant1_gain_db = 4.0;
    }
};

class Contrabassoon : public WoodwindInstrument {
public:
    Contrabassoon() : WoodwindInstrument("Contrabassoon") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.45; recipe.noise_mix = 0.06;
        recipe.filter_cutoff = 1600.0;
    }
};

class SopranoRecorder : public WoodwindInstrument {
public:
    SopranoRecorder() : WoodwindInstrument("Soprano Recorder") {
        recipe.filter_cutoff = 4600.0; recipe.noise_mix = 0.12; recipe.osc_mix = 0.1;
    }
};
using Recorder = SopranoRecorder;

class AltoRecorder : public WoodwindInstrument {
public:
    AltoRecorder() : WoodwindInstrument("Alto Recorder") {
        recipe.filter_cutoff = 3800.0; recipe.noise_mix = 0.11; recipe.osc_mix = 0.1;
    }
};

class TenorRecorder : public WoodwindInstrument {
public:
    TenorRecorder() : WoodwindInstrument("Tenor Recorder") {
        recipe.filter_cutoff = 3000.0; recipe.noise_mix = 0.1; recipe.osc_mix = 0.1;
    }
};

class BassRecorder : public WoodwindInstrument {
public:
    BassRecorder() : WoodwindInstrument("Bass Recorder") {
        recipe.filter_cutoff = 2000.0; recipe.noise_mix = 0.1; recipe.osc_mix = 0.12;
    }
};

class ContrabassRecorder : public WoodwindInstrument {
public:
    ContrabassRecorder() : WoodwindInstrument("Contrabass Recorder") {
        recipe.filter_cutoff = 1200.0; recipe.noise_mix = 0.11; recipe.osc_mix = 0.14;
    }
};

class PanFlute : public WoodwindInstrument {
public:
    PanFlute() : WoodwindInstrument("Pan Flute") {
        recipe.filter_cutoff = 4200.0; recipe.noise_mix = 0.14; recipe.osc_mix = 0.1;
        recipe.vibrato_depth_cents = 6.0;
    }
};

class Ocarina : public WoodwindInstrument {
public:
    Ocarina() : WoodwindInstrument("Ocarina") {
        recipe.filter_cutoff = 3600.0; recipe.noise_mix = 0.1; recipe.osc_mix = 0.05;
        recipe.osc1_shape = WaveShape::Sine;
    }
};

class Harmonica : public WoodwindInstrument {
public:
    Harmonica() : WoodwindInstrument("Harmonica") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.3; recipe.noise_mix = 0.06;
        recipe.detune_cents = 12.0; recipe.filter_cutoff = 4800.0;
    }
};

class IrishWhistle : public WoodwindInstrument {
public:
    IrishWhistle() : WoodwindInstrument("Irish Whistle") {
        recipe.filter_cutoff = 6200.0; recipe.noise_mix = 0.13; recipe.osc_mix = 0.08;
    }
};
using TinWhistle = IrishWhistle;

class LowWhistle : public WoodwindInstrument {
public:
    LowWhistle() : WoodwindInstrument("Low Whistle") {
        recipe.filter_cutoff = 3400.0; recipe.noise_mix = 0.12; recipe.osc_mix = 0.09;
    }
};

}
}
