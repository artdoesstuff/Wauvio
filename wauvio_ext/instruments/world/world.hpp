#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class Erhu : public BowedStringInstrument {
public:
    Erhu() : BowedStringInstrument("Erhu") {
        recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 1100.0; recipe.vibrato_depth_cents = 28.0;
        recipe.osc_mix = 0.5;
    }
};

class Guzheng : public GuitarInstrument {
public:
    Guzheng() : GuitarInstrument("Guzheng") {
        recipe.filter_cutoff = 6000.0; recipe.envelope.decay = 1.3; recipe.noise_attack_burst = 0.18;
    }
};

class Sitar : public GuitarInstrument {
public:
    Sitar() : GuitarInstrument("Sitar") {
        recipe.filter_cutoff = 6500.0; recipe.detune_cents = 16.0; recipe.osc_mix = 0.4;
        recipe.envelope.decay = 1.3; recipe.formant1_hz = 1500.0; recipe.formant1_gain_db = 4.0;
    }
};

class Koto : public GuitarInstrument {
public:
    Koto() : GuitarInstrument("Koto") {
        recipe.filter_cutoff = 5600.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.envelope.decay = 1.2; recipe.noise_attack_burst = 0.15;
    }
};

class Shamisen : public GuitarInstrument {
public:
    Shamisen() : GuitarInstrument("Shamisen") {
        recipe.filter_cutoff = 4400.0; recipe.noise_attack_burst = 0.32; recipe.envelope.decay = 0.3;
    }
};

class Biwa : public GuitarInstrument {
public:
    Biwa() : GuitarInstrument("Biwa") {
        recipe.filter_cutoff = 3800.0; recipe.noise_attack_burst = 0.35; recipe.envelope.decay = 0.4;
        recipe.detune_cents = 14.0;
    }
};

class Pipa : public GuitarInstrument {
public:
    Pipa() : GuitarInstrument("Pipa") {
        recipe.filter_cutoff = 6800.0; recipe.noise_attack_burst = 0.28; recipe.envelope.decay = 0.45;
    }
};

class Sanxian : public GuitarInstrument {
public:
    Sanxian() : GuitarInstrument("Sanxian") {
        recipe.filter_cutoff = 5200.0; recipe.noise_attack_burst = 0.3; recipe.envelope.decay = 0.35;
    }
};

class Guqin : public GuitarInstrument {
public:
    Guqin() : GuitarInstrument("Guqin") {
        recipe.filter_cutoff = 2600.0; recipe.envelope.decay = 2.0; recipe.noise_attack_burst = 0.1;
        recipe.gain = 0.7;
    }
};

class Gayageum : public GuitarInstrument {
public:
    Gayageum() : GuitarInstrument("Gayageum") {
        recipe.filter_cutoff = 5400.0; recipe.envelope.decay = 1.4; recipe.noise_attack_burst = 0.16;
        recipe.vibrato_depth_cents = 10.0;
    }
};

class Bansuri : public WoodwindInstrument {
public:
    Bansuri() : WoodwindInstrument("Bansuri") {
        recipe.filter_cutoff = 4400.0; recipe.noise_mix = 0.13; recipe.vibrato_depth_cents = 20.0;
    }
};

class Duduk : public WoodwindInstrument {
public:
    Duduk() : WoodwindInstrument("Duduk") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.4; recipe.noise_mix = 0.05;
        recipe.formant1_hz = 800.0; recipe.formant1_gain_db = 6.0; recipe.filter_cutoff = 3000.0;
        recipe.vibrato_depth_cents = 22.0;
    }
};

class Ney : public WoodwindInstrument {
public:
    Ney() : WoodwindInstrument("Ney") {
        recipe.filter_cutoff = 3600.0; recipe.noise_mix = 0.22; recipe.osc_mix = 0.12;
        recipe.vibrato_depth_cents = 14.0;
    }
};

class Shakuhachi : public WoodwindInstrument {
public:
    Shakuhachi() : WoodwindInstrument("Shakuhachi") {
        recipe.filter_cutoff = 3200.0; recipe.noise_mix = 0.25; recipe.osc_mix = 0.1;
        recipe.pitch_instability = 6.0;
    }
};

class Xiao : public WoodwindInstrument {
public:
    Xiao() : WoodwindInstrument("Xiao") {
        recipe.filter_cutoff = 3000.0; recipe.noise_mix = 0.22; recipe.osc_mix = 0.1;
    }
};

class Hulusi : public WoodwindInstrument {
public:
    Hulusi() : WoodwindInstrument("Hulusi") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.3; recipe.noise_mix = 0.05;
        recipe.filter_cutoff = 3400.0; recipe.formant1_hz = 700.0; recipe.formant1_gain_db = 3.5;
    }
};

class Sheng : public WoodwindInstrument {
public:
    Sheng() : WoodwindInstrument("Sheng") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc2_shape = WaveShape::BL_Sawtooth;
        recipe.osc_mix = 0.45; recipe.detune_cents = 8.0; recipe.filter_cutoff = 4600.0;
    }
};

class Suona : public WoodwindInstrument {
public:
    Suona() : WoodwindInstrument("Suona") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.5; recipe.noise_mix = 0.06;
        recipe.formant1_hz = 1600.0; recipe.formant1_gain_db = 6.0; recipe.filter_cutoff = 6200.0;
    }
};

class Dizi : public WoodwindInstrument {
public:
    Dizi() : WoodwindInstrument("Dizi") {
        recipe.filter_cutoff = 5200.0; recipe.noise_mix = 0.14; recipe.osc_mix = 0.12;
        recipe.formant1_hz = 2000.0; recipe.formant1_gain_db = 2.0;
    }
};

class Zurna : public WoodwindInstrument {
public:
    Zurna() : WoodwindInstrument("Zurna") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.55; recipe.noise_mix = 0.07;
        recipe.formant1_hz = 1700.0; recipe.formant1_gain_db = 6.5; recipe.filter_cutoff = 6600.0;
    }
};

class Raita : public WoodwindInstrument {
public:
    Raita() : WoodwindInstrument("Raita") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.5; recipe.noise_mix = 0.06;
        recipe.formant1_hz = 1500.0; recipe.formant1_gain_db = 6.0; recipe.filter_cutoff = 6000.0;
    }
};

class Shehnai : public WoodwindInstrument {
public:
    Shehnai() : WoodwindInstrument("Shehnai") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.48; recipe.noise_mix = 0.06;
        recipe.formant1_hz = 1300.0; recipe.formant1_gain_db = 5.5; recipe.filter_cutoff = 5600.0;
        recipe.vibrato_depth_cents = 18.0;
    }
};

class NativeAmericanFlute : public WoodwindInstrument {
public:
    NativeAmericanFlute() : WoodwindInstrument("Native American Flute") {
        recipe.filter_cutoff = 3400.0; recipe.noise_mix = 0.16; recipe.osc_mix = 0.08;
        recipe.vibrato_depth_cents = 24.0;
    }
};

class Mbira : public MalletInstrument {
public:
    Mbira() : MalletInstrument("Mbira") {
        recipe.fm_ratio = 2.5; recipe.fm_index = 0.6; recipe.filter_cutoff = 4400.0;
        recipe.envelope.decay = 1.0;
    }
};

class Balafon : public MalletInstrument {
public:
    Balafon() : MalletInstrument("Balafon") {
        recipe.fm_ratio = 3.2; recipe.fm_index = 1.0; recipe.filter_cutoff = 4000.0;
        recipe.envelope.decay = 0.8;
    }
};

class Kalimba : public MalletInstrument {
public:
    Kalimba() : MalletInstrument("Kalimba") {
        recipe.fm_ratio = 2.0; recipe.fm_index = 0.7; recipe.filter_cutoff = 5200.0;
        recipe.envelope.decay = 1.4; recipe.noise_attack_burst = 0.12;
    }
};

class SteelPan : public MalletInstrument {
public:
    SteelPan() : MalletInstrument("Steel Pan") {
        recipe.fm_ratio = 2.9; recipe.fm_index = 1.4; recipe.filter_cutoff = 5000.0;
        recipe.envelope.decay = 1.3; recipe.stereo_width = 0.3;
    }
};

class SteelTongueDrum : public MalletInstrument {
public:
    SteelTongueDrum() : MalletInstrument("Steel Tongue Drum") {
        recipe.fm_ratio = 2.7; recipe.fm_index = 1.0; recipe.filter_cutoff = 3200.0;
        recipe.envelope.decay = 1.8; recipe.stereo_width = 0.2;
    }
};
using TongueDrum = SteelTongueDrum;

class Handpan : public MalletInstrument {
public:
    Handpan() : MalletInstrument("Handpan") {
        recipe.fm_ratio = 2.35; recipe.fm_index = 0.9; recipe.filter_cutoff = 3600.0;
        recipe.envelope.decay = 2.4; recipe.stereo_width = 0.3;
    }
};
using HangDrum = Handpan;

class Didgeridoo : public WorldInstrument {
public:
    Didgeridoo() : WorldInstrument("Didgeridoo") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.5; recipe.noise_mix = 0.15;
        recipe.filter_cutoff = 900.0; recipe.envelope = DAHDSR{0.0, 0.05, 0.0, 0.05, 0.9, 0.2};
        recipe.fixed_pitch_hz = 73.4;
    }
};

class Bagpipes : public WorldInstrument {
public:
    Bagpipes() : WorldInstrument("Bagpipes") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc2_shape = WaveShape::BL_Sawtooth;
        recipe.osc_mix = 0.4; recipe.detune_cents = 3.0; recipe.noise_mix = 0.08;
        recipe.filter_cutoff = 4600.0; recipe.envelope = DAHDSR{0.0, 0.03, 0.0, 0.02, 0.95, 0.15};
    }
};

class GreatHighlandBagpipe : public WorldInstrument {
public:
    GreatHighlandBagpipe() : WorldInstrument("Great Highland Bagpipe") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc2_shape = WaveShape::BL_Sawtooth;
        recipe.osc_mix = 0.48; recipe.detune_cents = 5.0; recipe.noise_mix = 0.1;
        recipe.filter_cutoff = 5200.0; recipe.envelope = DAHDSR{0.0, 0.02, 0.0, 0.02, 0.95, 0.12};
        recipe.gain = 1.0;
    }
};

class UilleannPipes : public WorldInstrument {
public:
    UilleannPipes() : WorldInstrument("Uilleann Pipes") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc2_shape = WaveShape::BL_Square;
        recipe.osc_mix = 0.4; recipe.detune_cents = 4.0; recipe.noise_mix = 0.06;
        recipe.formant1_hz = 1300.0; recipe.formant1_gain_db = 4.0; recipe.filter_cutoff = 4800.0;
        recipe.envelope = DAHDSR{0.0, 0.025, 0.0, 0.02, 0.93, 0.1}; recipe.vibrato_depth_cents = 10.0;
    }
};

class HurdyGurdy : public WorldInstrument {
public:
    HurdyGurdy() : WorldInstrument("Hurdy-Gurdy") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc2_shape = WaveShape::BL_Square;
        recipe.osc_mix = 0.35; recipe.detune_cents = 10.0; recipe.noise_mix = 0.18;
        recipe.pitch_instability = 12.0; recipe.filter_cutoff = 4200.0;
        recipe.envelope = DAHDSR{0.0, 0.03, 0.0, 0.02, 0.9, 0.15};
    }
};

class Concertina : public KeyboardInstrument {
public:
    Concertina() : KeyboardInstrument("Concertina") {
        recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.3;
        recipe.envelope = DAHDSR{0.0, 0.03, 0.0, 0.02, 0.9, 0.15}; recipe.filter_cutoff = 4800.0;
    }
};

class Bandoneon : public KeyboardInstrument {
public:
    Bandoneon() : KeyboardInstrument("Bandoneon") {
        recipe.osc1_shape = WaveShape::BL_Sawtooth; recipe.osc_mix = 0.45; recipe.detune_cents = 11.0;
        recipe.envelope = DAHDSR{0.0, 0.05, 0.0, 0.02, 0.88, 0.25}; recipe.filter_cutoff = 3400.0;
    }
};

}
}
