#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class AcousticGrandPiano : public KeyboardInstrument {
public:
    AcousticGrandPiano() : KeyboardInstrument("Acoustic Grand Piano") {
        recipe.filter_cutoff = 7200.0; recipe.osc_mix = 0.22; recipe.envelope.decay = 1.1;
        recipe.envelope.release = 0.4; recipe.stereo_width = 0.18;
    }
};

class UprightPiano : public KeyboardInstrument {
public:
    UprightPiano() : KeyboardInstrument("Upright Piano") {
        recipe.filter_cutoff = 5800.0; recipe.osc_mix = 0.3; recipe.envelope.decay = 0.85;
        recipe.noise_attack_burst = 0.08;
    }
};

class ElectricPiano : public KeyboardInstrument {
public:
    ElectricPiano() : KeyboardInstrument("Electric Piano") {
        recipe.use_fm = true; recipe.fm_ratio = 1.0; recipe.fm_index = 1.2; recipe.osc_mix = 0.55;
        recipe.filter_cutoff = 4200.0; recipe.envelope.decay = 1.3; recipe.stereo_width = 0.22;
    }
};

class RhodesMarkI : public KeyboardInstrument {
public:
    RhodesMarkI() : KeyboardInstrument("Rhodes Mark I") {
        recipe.use_fm = true; recipe.fm_ratio = 1.0; recipe.fm_index = 0.8; recipe.osc_mix = 0.6;
        recipe.filter_cutoff = 3600.0; recipe.envelope.decay = 1.6;
    }
};
using Rhodes = RhodesMarkI;

class RhodesMarkII : public KeyboardInstrument {
public:
    RhodesMarkII() : KeyboardInstrument("Rhodes Mark II") {
        recipe.use_fm = true; recipe.fm_ratio = 1.0; recipe.fm_index = 1.1; recipe.osc_mix = 0.65;
        recipe.filter_cutoff = 4400.0; recipe.envelope.decay = 1.3;
    }
};

class Wurlitzer200A : public KeyboardInstrument {
public:
    Wurlitzer200A() : KeyboardInstrument("Wurlitzer 200A") {
        recipe.use_fm = true; recipe.fm_ratio = 2.0; recipe.fm_index = 1.6; recipe.osc_mix = 0.5;
        recipe.filter_cutoff = 4600.0; recipe.envelope.decay = 1.1;
    }
};
using Wurlitzer = Wurlitzer200A;

class ClavinetD6 : public KeyboardInstrument {
public:
    ClavinetD6() : KeyboardInstrument("Clavinet D6") {
        recipe.percussive = true; recipe.osc1_shape = WaveShape::BL_Square;
        recipe.filter_cutoff = 3200.0; recipe.noise_attack_burst = 0.15;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.35, 0.0, 0.1};
    }
};
using Clavinet = ClavinetD6;

class Harpsichord : public KeyboardInstrument {
public:
    Harpsichord() : KeyboardInstrument("Harpsichord") {
        recipe.percussive = true; recipe.osc1_shape = WaveShape::BL_Sawtooth;
        recipe.filter_cutoff = 8500.0; recipe.noise_attack_burst = 0.2;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.7, 0.0, 0.12};
    }
};

class Celesta : public KeyboardInstrument {
public:
    Celesta() : KeyboardInstrument("Celesta") {
        recipe.use_fm = true; recipe.fm_ratio = 4.0; recipe.fm_index = 1.0; recipe.osc_mix = 0.7;
        recipe.filter_cutoff = 9500.0; recipe.envelope.decay = 1.4; recipe.stereo_width = 0.3;
    }
};

class ToyPiano : public KeyboardInstrument {
public:
    ToyPiano() : KeyboardInstrument("Toy Piano") {
        recipe.use_fm = true; recipe.fm_ratio = 3.5; recipe.fm_index = 1.3; recipe.osc_mix = 0.65;
        recipe.filter_cutoff = 6000.0; recipe.envelope.decay = 0.5;
    }
};

class PipeOrgan : public KeyboardInstrument {
public:
    PipeOrgan() : KeyboardInstrument("Pipe Organ") {
        recipe.percussive = false; recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.5; recipe.noise_attack_burst = 0.0;
        recipe.envelope = DAHDSR{0.0, 0.05, 0.0, 0.02, 0.95, 0.3};
        recipe.filter_cutoff = 5000.0; recipe.stereo_width = 0.35;
    }
};

class ChurchOrgan : public KeyboardInstrument {
public:
    ChurchOrgan() : KeyboardInstrument("Church Organ") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 3.0; recipe.osc_mix = 0.45;
        recipe.envelope = DAHDSR{0.0, 0.08, 0.0, 0.02, 0.95, 0.6};
        recipe.filter_cutoff = 4200.0; recipe.stereo_width = 0.4;
    }
};

class HammondOrgan : public KeyboardInstrument {
public:
    HammondOrgan() : KeyboardInstrument("Hammond Organ") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 3.0; recipe.osc_mix = 0.4; recipe.detune_cents = 8.0;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 0.02, 0.95, 0.1};
        recipe.filter_cutoff = 6000.0; recipe.stereo_width = 0.25;
    }
};

class SpinetPiano : public KeyboardInstrument {
public:
    SpinetPiano() : KeyboardInstrument("Spinet Piano") {
        recipe.filter_cutoff = 5200.0; recipe.osc_mix = 0.28; recipe.envelope.decay = 0.65;
    }
};

class CloseMicGrandPiano : public KeyboardInstrument {
public:
    CloseMicGrandPiano() : KeyboardInstrument("Close-Mic Grand Piano") {
        recipe.filter_cutoff = 8200.0; recipe.osc_mix = 0.2; recipe.envelope.decay = 1.1;
        recipe.stereo_width = 0.08; recipe.noise_attack_burst = 0.1;
    }
};

class RoomMicGrandPiano : public KeyboardInstrument {
public:
    RoomMicGrandPiano() : KeyboardInstrument("Room-Mic Grand Piano") {
        recipe.filter_cutoff = 6000.0; recipe.osc_mix = 0.22; recipe.envelope.decay = 1.4;
        recipe.stereo_width = 0.45;
    }
};

class HonkyTonkPiano : public KeyboardInstrument {
public:
    HonkyTonkPiano() : KeyboardInstrument("Honky-Tonk Piano") {
        recipe.filter_cutoff = 5500.0; recipe.detune_cents = 22.0; recipe.osc_mix = 0.35;
        recipe.envelope.decay = 0.9;
    }
};

class FeltPiano : public KeyboardInstrument {
public:
    FeltPiano() : KeyboardInstrument("Felt Piano") {
        recipe.filter_cutoff = 2600.0; recipe.osc_mix = 0.15; recipe.envelope.decay = 1.3;
        recipe.noise_attack_burst = 0.02;
    }
};

class TackPiano : public KeyboardInstrument {
public:
    TackPiano() : KeyboardInstrument("Tack Piano") {
        recipe.filter_cutoff = 9000.0; recipe.osc1_shape = WaveShape::BL_Square;
        recipe.noise_attack_burst = 0.25; recipe.envelope.decay = 0.4;
    }
};

class PreparedPiano : public KeyboardInstrument {
public:
    PreparedPiano() : KeyboardInstrument("Prepared Piano") {
        recipe.percussive = true; recipe.filter_cutoff = 3400.0; recipe.osc_mix = 0.5;
        recipe.noise_mix = 0.12; recipe.noise_attack_burst = 0.3;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.3, 0.0, 0.1};
        recipe.pitch_instability = 6.0;
    }
};

class Mellotron : public KeyboardInstrument {
public:
    Mellotron() : KeyboardInstrument("Mellotron") {
        recipe.filter_cutoff = 3800.0; recipe.osc_mix = 0.4; recipe.detune_cents = 14.0;
        recipe.noise_mix = 0.03; recipe.envelope = DAHDSR{0.0, 0.06, 0.0, 0.1, 0.85, 0.3};
        recipe.pitch_instability = 4.0; recipe.stereo_width = 0.3;
    }
};

class TheaterOrgan : public KeyboardInstrument {
public:
    TheaterOrgan() : KeyboardInstrument("Theater Organ") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.5; recipe.detune_cents = 10.0;
        recipe.envelope = DAHDSR{0.0, 0.03, 0.0, 0.02, 0.95, 0.4};
        recipe.filter_cutoff = 5500.0; recipe.vibrato_rate_hz = 6.0; recipe.vibrato_depth_cents = 12.0;
        recipe.stereo_width = 0.4;
    }
};

class Accordion : public KeyboardInstrument {
public:
    Accordion() : KeyboardInstrument("Accordion") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc2_shape = WaveShape::BL_Sawtooth;
        recipe.osc_mix = 0.4; recipe.detune_cents = 9.0; recipe.noise_attack_burst = 0.02;
        recipe.envelope = DAHDSR{0.0, 0.04, 0.0, 0.02, 0.9, 0.2}; recipe.filter_cutoff = 4200.0;
    }
};

class AccordionBass : public KeyboardInstrument {
public:
    AccordionBass() : KeyboardInstrument("Accordion Bass") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.3; recipe.detune_cents = 6.0;
        recipe.envelope = DAHDSR{0.0, 0.03, 0.0, 0.02, 0.9, 0.15}; recipe.filter_cutoff = 1800.0;
    }
};

class ChamberOrgan : public KeyboardInstrument {
public:
    ChamberOrgan() : KeyboardInstrument("Chamber Organ") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 2.0; recipe.osc_mix = 0.4;
        recipe.envelope = DAHDSR{0.0, 0.04, 0.0, 0.02, 0.95, 0.2};
        recipe.filter_cutoff = 4000.0; recipe.stereo_width = 0.25;
    }
};

class Harmonium : public KeyboardInstrument {
public:
    Harmonium() : KeyboardInstrument("Harmonium") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.4; recipe.detune_cents = 10.0;
        recipe.noise_mix = 0.04;
        recipe.envelope = DAHDSR{0.0, 0.05, 0.0, 0.02, 0.9, 0.2}; recipe.filter_cutoff = 3000.0;
    }
};

class ReedOrgan : public KeyboardInstrument {
public:
    ReedOrgan() : KeyboardInstrument("Reed Organ") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.42; recipe.detune_cents = 12.0;
        recipe.noise_mix = 0.03;
        recipe.envelope = DAHDSR{0.0, 0.04, 0.0, 0.02, 0.92, 0.18}; recipe.filter_cutoff = 3400.0;
    }
};

class CPElectricPiano : public KeyboardInstrument {
public:
    CPElectricPiano() : KeyboardInstrument("CP Electric Piano") {
        recipe.use_fm = true; recipe.fm_ratio = 1.0; recipe.fm_index = 1.4; recipe.osc_mix = 0.7;
        recipe.filter_cutoff = 5000.0; recipe.envelope.decay = 1.4; recipe.stereo_width = 0.2;
    }
};

class SoftPedalPiano : public KeyboardInstrument {
public:
    SoftPedalPiano() : KeyboardInstrument("Soft-Pedal Piano") {
        recipe.filter_cutoff = 3400.0; recipe.osc_mix = 0.15; recipe.envelope.decay = 1.0;
        recipe.gain = 0.6; recipe.noise_attack_burst = 0.03;
    }
};

}
}
