#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class Crotales : public MalletInstrument {
public:
    Crotales() : MalletInstrument("Crotales") {
        recipe.fm_ratio = 4.2; recipe.fm_index = 1.8; recipe.filter_cutoff = 12000.0;
        recipe.envelope.decay = 3.0;
    }
};

class Glockenspiel : public MalletInstrument {
public:
    Glockenspiel() : MalletInstrument("Glockenspiel") {
        recipe.fm_ratio = 3.99; recipe.fm_index = 1.6; recipe.filter_cutoff = 11000.0;
        recipe.envelope.decay = 1.6;
    }
};

class SoftGlockenspiel : public MalletInstrument {
public:
    SoftGlockenspiel() : MalletInstrument("Soft Glockenspiel") {
        recipe.fm_ratio = 3.99; recipe.fm_index = 0.9; recipe.filter_cutoff = 8000.0;
        recipe.envelope.decay = 1.8; recipe.gain = 0.7;
    }
};

class HardGlockenspiel : public MalletInstrument {
public:
    HardGlockenspiel() : MalletInstrument("Hard Glockenspiel") {
        recipe.fm_ratio = 3.99; recipe.fm_index = 2.4; recipe.filter_cutoff = 13000.0;
        recipe.envelope.decay = 1.3; recipe.noise_attack_burst = 0.1;
    }
};

class Vibraphone : public MalletInstrument {
public:
    Vibraphone() : MalletInstrument("Vibraphone") {
        recipe.fm_ratio = 4.0; recipe.fm_index = 0.9; recipe.filter_cutoff = 6500.0;
        recipe.vibrato_rate_hz = 4.5; recipe.vibrato_depth_cents = 10.0; recipe.vibrato_delay_sec = 0.0;
        recipe.envelope.decay = 2.2;
    }
};
using VibraphoneWithMotor = Vibraphone;

class VibraphoneWithoutMotor : public MalletInstrument {
public:
    VibraphoneWithoutMotor() : MalletInstrument("Vibraphone without Motor") {
        recipe.fm_ratio = 4.0; recipe.fm_index = 0.9; recipe.filter_cutoff = 6500.0;
        recipe.envelope.decay = 2.6;
    }
};

class Marimba : public MalletInstrument {
public:
    Marimba() : MalletInstrument("Marimba") {
        recipe.fm_ratio = 3.4; recipe.fm_index = 1.1; recipe.filter_cutoff = 3800.0;
        recipe.envelope.decay = 0.9; recipe.noise_attack_burst = 0.05;
    }
};

class BassMarimba : public MalletInstrument {
public:
    BassMarimba() : MalletInstrument("Bass Marimba") {
        recipe.fm_ratio = 3.1; recipe.fm_index = 1.0; recipe.filter_cutoff = 1800.0;
        recipe.envelope.decay = 1.3; recipe.noise_attack_burst = 0.06;
    }
};

class Xylophone : public MalletInstrument {
public:
    Xylophone() : MalletInstrument("Xylophone") {
        recipe.fm_ratio = 3.9; recipe.fm_index = 1.8; recipe.filter_cutoff = 9000.0;
        recipe.envelope.decay = 0.55; recipe.noise_attack_burst = 0.08;
    }
};

class TubularBells : public MalletInstrument {
public:
    TubularBells() : MalletInstrument("Tubular Bells") {
        recipe.fm_ratio = 2.4; recipe.fm_index = 2.2; recipe.filter_cutoff = 7000.0;
        recipe.envelope.decay = 3.2; recipe.stereo_width = 0.35;
    }
};
using OrchestralBells = TubularBells;

class Timpani : public MalletInstrument {
public:
    Timpani() : MalletInstrument("Timpani") {
        recipe.fm_ratio = 1.5; recipe.fm_index = 0.6; recipe.filter_cutoff = 1200.0;
        recipe.envelope.decay = 1.4; recipe.noise_attack_burst = 0.08; recipe.stereo_width = 0.2;
    }
};

class Handbells : public MalletInstrument {
public:
    Handbells() : MalletInstrument("Handbells") {
        recipe.fm_ratio = 3.0; recipe.fm_index = 1.4; recipe.filter_cutoff = 7500.0;
        recipe.envelope.decay = 2.0; recipe.stereo_width = 0.25;
    }
};

class GlassHarmonica : public MalletInstrument {
public:
    GlassHarmonica() : MalletInstrument("Glass Harmonica") {
        recipe.percussive = false; recipe.fm_ratio = 1.0; recipe.fm_index = 0.2;
        recipe.filter_cutoff = 5000.0; recipe.envelope = DAHDSR{0.0, 0.2, 0.0, 0.2, 0.85, 0.6};
        recipe.stereo_width = 0.3;
    }
};

class GlassMarimba : public MalletInstrument {
public:
    GlassMarimba() : MalletInstrument("Glass Marimba") {
        recipe.fm_ratio = 5.2; recipe.fm_index = 1.5; recipe.filter_cutoff = 10000.0;
        recipe.envelope.decay = 1.5; recipe.noise_attack_burst = 0.04;
    }
};

class MusicBox : public MalletInstrument {
public:
    MusicBox() : MalletInstrument("Music Box") {
        recipe.fm_ratio = 5.0; recipe.fm_index = 1.0; recipe.filter_cutoff = 10000.0;
        recipe.envelope.decay = 1.1; recipe.noise_attack_burst = 0.18;
    }
};

class Waterphone : public MalletInstrument {
public:
    Waterphone() : MalletInstrument("Waterphone") {
        recipe.use_fm = true; recipe.fm_ratio = 3.73; recipe.fm_index = 3.2; recipe.osc_mix = 0.75;
        recipe.noise_mix = 0.1; recipe.pink_noise = true;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 3.5, 0.15, 2.5};
        recipe.filter_cutoff = 6500.0; recipe.filter_q = 1.4;
        recipe.pitch_instability = 65.0;
        recipe.formant1_hz = 1800.0; recipe.formant1_gain_db = 5.0; recipe.formant1_q = 4.0;
        recipe.formant2_hz = 3200.0; recipe.formant2_gain_db = 3.5; recipe.formant2_q = 5.0;
        recipe.stereo_width = 0.4; recipe.gain = 0.75;
    }
};

}
}
