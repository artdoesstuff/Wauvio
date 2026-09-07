#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class SynthLead : public SynthInstrument {
public:
    SynthLead() : SynthInstrument("Synth Lead") {
        recipe.filter_cutoff = 5500.0; recipe.filter_env_amount = 2500.0; recipe.osc_mix = 0.5;
    }
};

class SynthPad : public SynthInstrument {
public:
    SynthPad() : SynthInstrument("Synth Pad") {
        recipe.detune_cents = 15.0; recipe.stereo_width = 0.5;
        recipe.envelope = DAHDSR{0.0, 0.6, 0.0, 0.4, 0.75, 1.2};
        recipe.filter_cutoff = 2800.0;
    }
};

class SynthBass : public SynthInstrument {
public:
    SynthBass() : SynthInstrument("Synth Bass") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.3; recipe.filter_cutoff = 1400.0;
        recipe.filter_env_amount = 900.0; recipe.envelope.decay = 0.12;
    }
};

class Pluck : public SynthInstrument {
public:
    Pluck() : SynthInstrument("Pluck") {
        recipe.percussive = true; recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.filter_cutoff = 6000.0; recipe.filter_env_amount = -3500.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.35, 0.0, 0.15};
    }
};
using SynthPluck = Pluck;

class Bell : public SynthInstrument {
public:
    Bell() : SynthInstrument("Bell") {
        recipe.use_fm = true; recipe.fm_ratio = 3.5; recipe.fm_index = 1.5; recipe.osc_mix = 0.8;
        recipe.envelope.decay = 1.8; recipe.filter_cutoff = 9000.0;
    }
};

class FMBell : public SynthInstrument {
public:
    FMBell() : SynthInstrument("FM Bell") {
        recipe.use_fm = true; recipe.fm_ratio = 7.0; recipe.fm_index = 3.0; recipe.osc_mix = 0.9;
        recipe.envelope.decay = 2.2; recipe.filter_cutoff = 10000.0;
    }
};

class OrganSynth : public SynthInstrument {
public:
    OrganSynth() : SynthInstrument("Organ Synth") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine; recipe.osc2_ratio = 3.0;
        recipe.osc_mix = 0.4; recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 0.02, 0.95, 0.1};
        recipe.filter_cutoff = 5500.0;
    }
};

class StringSynth : public SynthInstrument {
public:
    StringSynth() : SynthInstrument("String Synth") {
        recipe.detune_cents = 18.0; recipe.stereo_width = 0.4;
        recipe.envelope = DAHDSR{0.0, 0.25, 0.0, 0.2, 0.8, 0.4}; recipe.filter_cutoff = 4000.0;
    }
};

class BrassSynth : public SynthInstrument {
public:
    BrassSynth() : SynthInstrument("Brass Synth") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.4;
        recipe.pitch_attack_cents = 30.0; recipe.filter_cutoff = 5000.0;
        recipe.envelope = DAHDSR{0.0, 0.04, 0.0, 0.1, 0.85, 0.15};
    }
};

class SoftPiano : public SynthInstrument {
public:
    SoftPiano() : SynthInstrument("Soft Piano") {
        recipe.osc1_shape = WaveShape::Triangle; recipe.osc2_shape = WaveShape::Sine; recipe.osc_mix = 0.2;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 1.0, 0.0, 0.5}; recipe.percussive = true;
        recipe.filter_cutoff = 4000.0;
    }
};

class ChiptuneLead : public SynthInstrument {
public:
    ChiptuneLead() : SynthInstrument("Chiptune Lead") {
        recipe.osc1_shape = WaveShape::Pulse; recipe.osc2_shape = WaveShape::Pulse; recipe.osc_mix = 0.0;
        recipe.filter_cutoff = 9000.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.05, 0.7, 0.05};
    }
};

class ChiptuneBass : public SynthInstrument {
public:
    ChiptuneBass() : SynthInstrument("Chiptune Bass") {
        recipe.osc1_shape = WaveShape::Square; recipe.osc_mix = 0.0; recipe.filter_cutoff = 2200.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.05, 0.8, 0.04};
    }
};

class ArpeggioSynth : public SynthInstrument {
public:
    ArpeggioSynth() : SynthInstrument("Arpeggio Synth") {
        recipe.percussive = true; recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.filter_cutoff = 6500.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.12, 0.0, 0.05};
    }
};

class AtmosphericSynth : public SynthInstrument {
public:
    AtmosphericSynth() : SynthInstrument("Atmospheric Synth") {
        recipe.detune_cents = 25.0; recipe.stereo_width = 0.6; recipe.noise_mix = 0.05;
        recipe.envelope = DAHDSR{0.0, 1.5, 0.0, 1.0, 0.7, 2.5}; recipe.filter_cutoff = 2200.0;
    }
};

class Drone : public SynthInstrument {
public:
    Drone() : SynthInstrument("Drone") {
        recipe.detune_cents = 8.0; recipe.stereo_width = 0.4;
        recipe.envelope = DAHDSR{0.0, 2.0, 0.0, 0.5, 0.9, 3.0}; recipe.filter_cutoff = 1800.0;
    }
};

}
}
