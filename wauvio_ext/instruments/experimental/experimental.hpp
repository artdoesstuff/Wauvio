#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class MusicalSaw : public ExperimentalInstrument {
public:
    MusicalSaw() : ExperimentalInstrument("Musical Saw") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Triangle;
        recipe.osc_mix = 0.15; recipe.vibrato_rate_hz = 4.5; recipe.vibrato_depth_cents = 35.0;
        recipe.vibrato_delay_sec = 0.0; recipe.pitch_instability = 10.0;
        recipe.envelope = DAHDSR{0.0, 0.4, 0.0, 0.3, 0.85, 0.6};
        recipe.filter_cutoff = 4000.0; recipe.stereo_width = 0.3;
    }
};

class Theremin : public ExperimentalInstrument {
public:
    Theremin() : ExperimentalInstrument("Theremin") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc_mix = 0.0;
        recipe.vibrato_rate_hz = 5.5; recipe.vibrato_depth_cents = 20.0; recipe.vibrato_delay_sec = 0.0;
        recipe.pitch_instability = 14.0; recipe.pitch_attack_cents = 200.0; recipe.pitch_attack_time = 0.15;
        recipe.envelope = DAHDSR{0.0, 0.2, 0.0, 0.2, 0.9, 0.5};
        recipe.filter_cutoff = 3500.0; recipe.stereo_width = 0.4;
    }
};

class OndesMartenot : public ExperimentalInstrument {
public:
    OndesMartenot() : ExperimentalInstrument("Ondes Martenot") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Triangle;
        recipe.osc_mix = 0.25; recipe.vibrato_rate_hz = 5.0; recipe.vibrato_depth_cents = 24.0;
        recipe.pitch_instability = 6.0; recipe.envelope = DAHDSR{0.0, 0.15, 0.0, 0.2, 0.88, 0.5};
        recipe.filter_cutoff = 4200.0; recipe.stereo_width = 0.35;
    }
};

class AeolianHarp : public ExperimentalInstrument {
public:
    AeolianHarp() : ExperimentalInstrument("Aeolian Harp") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc2_ratio = 3.0; recipe.osc_mix = 0.3; recipe.noise_mix = 0.35;
        recipe.envelope = DAHDSR{0.0, 1.0, 0.0, 1.0, 0.6, 2.0};
        recipe.pitch_instability = 15.0; recipe.filter_cutoff = 6000.0; recipe.stereo_width = 0.6;
        recipe.gain = 0.5;
    }
};

class SingingGlass : public ExperimentalInstrument {
public:
    SingingGlass() : ExperimentalInstrument("Singing Glass") {
        recipe.osc1_shape = WaveShape::Sine; recipe.osc_mix = 0.05;
        recipe.filter_cutoff = 6500.0; recipe.filter_q = 3.5;
        recipe.envelope = DAHDSR{0.0, 0.5, 0.0, 0.3, 0.9, 1.2};
        recipe.pitch_instability = 4.0; recipe.stereo_width = 0.3;
    }
};

class BowedCymbal : public ExperimentalInstrument {
public:
    BowedCymbal() : ExperimentalInstrument("Bowed Cymbal") {
        recipe.fixed_pitch_hz = 1800.0; recipe.noise_mix = 0.55; recipe.pink_noise = false;
        recipe.envelope = DAHDSR{0.0, 0.6, 0.0, 0.5, 0.75, 1.5};
        recipe.pitch_instability = 25.0; recipe.filter_cutoff = 7500.0; recipe.stereo_width = 0.4;
    }
};

class BowedGong : public ExperimentalInstrument {
public:
    BowedGong() : ExperimentalInstrument("Bowed Gong") {
        recipe.fixed_pitch_hz = 400.0; recipe.noise_mix = 0.4; recipe.pink_noise = false;
        recipe.envelope = DAHDSR{0.0, 0.8, 0.0, 0.6, 0.8, 2.0};
        recipe.pitch_instability = 30.0; recipe.filter_cutoff = 5000.0; recipe.stereo_width = 0.45;
    }
};

class GlassInstruments : public ExperimentalInstrument {
public:
    GlassInstruments() : ExperimentalInstrument("Glass Instruments") {
        recipe.use_fm = true; recipe.fm_ratio = 4.4; recipe.fm_index = 1.2; recipe.osc_mix = 0.6;
        recipe.filter_cutoff = 8500.0; recipe.filter_q = 2.5;
        recipe.envelope = DAHDSR{0.0, 0.02, 0.0, 1.5, 0.1, 0.8};
        recipe.pitch_instability = 5.0;
    }
};

class ResonantMetal : public ExperimentalInstrument {
public:
    ResonantMetal() : ExperimentalInstrument("Resonant Metal") {
        recipe.use_fm = true; recipe.fm_ratio = 3.14; recipe.fm_index = 2.6; recipe.osc_mix = 0.7;
        recipe.filter_cutoff = 5500.0; recipe.filter_q = 2.0;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 2.0, 0.05, 1.2};
        recipe.pitch_instability = 20.0; recipe.noise_mix = 0.1;
    }
};

class FoundPercussion : public ExperimentalInstrument {
public:
    FoundPercussion() : ExperimentalInstrument("Found Percussion") {
        recipe.percussive = true; recipe.noise_mix = 0.6; recipe.pink_noise = false;
        recipe.filter_cutoff = 3500.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.3, 0.0, 0.1};
        recipe.pitch_instability = 30.0;
    }
};

class TapePiano : public ExperimentalInstrument {
public:
    TapePiano() : ExperimentalInstrument("Tape Piano") {
        recipe.osc1_shape = WaveShape::Triangle; recipe.osc2_shape = WaveShape::Sine;
        recipe.osc_mix = 0.25; recipe.filter_cutoff = 2600.0; recipe.noise_mix = 0.04;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 1.0, 0.0, 0.4};
        recipe.pitch_instability = 9.0; recipe.percussive = true;
    }
};

class TapeChoir : public ExperimentalInstrument {
public:
    TapeChoir() : ExperimentalInstrument("Tape Choir") {
        recipe.formant1_hz = 700.0; recipe.formant1_gain_db = 5.0; recipe.detune_cents = 14.0;
        recipe.filter_cutoff = 2400.0; recipe.noise_mix = 0.05;
        recipe.envelope = DAHDSR{0.0, 0.3, 0.0, 0.3, 0.8, 0.8};
        recipe.pitch_instability = 10.0; recipe.stereo_width = 0.4;
    }
};

class TapeStrings : public ExperimentalInstrument {
public:
    TapeStrings() : ExperimentalInstrument("Tape Strings") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.detune_cents = 18.0;
        recipe.filter_cutoff = 2800.0; recipe.noise_mix = 0.04;
        recipe.envelope = DAHDSR{0.0, 0.25, 0.0, 0.2, 0.8, 0.5};
        recipe.pitch_instability = 9.0; recipe.stereo_width = 0.4;
    }
};

class GranularVocal : public ExperimentalInstrument {
public:
    GranularVocal() : ExperimentalInstrument("Granular Vocal") {
        recipe.formant1_hz = 600.0; recipe.formant1_gain_db = 5.0;
        recipe.noise_mix = 0.2; recipe.filter_cutoff = 3200.0;
        recipe.envelope = DAHDSR{0.0, 0.4, 0.0, 0.4, 0.65, 1.0};
        recipe.pitch_instability = 35.0; recipe.stereo_width = 0.5; recipe.gain = 0.6;
    }
};

}
}
