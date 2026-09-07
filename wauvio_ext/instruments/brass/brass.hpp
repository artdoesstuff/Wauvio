#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class PiccoloTrumpet : public BrassInstrument {
public:
    PiccoloTrumpet() : BrassInstrument("Piccolo Trumpet") {
        recipe.filter_cutoff = 8200.0; recipe.formant1_hz = 1800.0; recipe.pitch_attack_cents = 50.0;
    }
};

class Trumpet : public BrassInstrument {
public:
    Trumpet() : BrassInstrument("Trumpet") {
        recipe.filter_cutoff = 6800.0; recipe.formant1_hz = 1400.0; recipe.pitch_attack_cents = 45.0;
    }
};

class Cornet : public BrassInstrument {
public:
    Cornet() : BrassInstrument("Cornet") {
        recipe.filter_cutoff = 5800.0; recipe.formant1_hz = 1200.0; recipe.pitch_attack_cents = 35.0;
    }
};

class Flugelhorn : public BrassInstrument {
public:
    Flugelhorn() : BrassInstrument("Flugelhorn") {
        recipe.filter_cutoff = 4200.0; recipe.formant1_hz = 900.0; recipe.osc_mix = 0.2;
    }
};

class BassTrumpet : public BrassInstrument {
public:
    BassTrumpet() : BrassInstrument("Bass Trumpet") {
        recipe.filter_cutoff = 3800.0; recipe.formant1_hz = 750.0; recipe.osc_mix = 0.3;
        recipe.pitch_attack_cents = 30.0;
    }
};

class Trombone : public BrassInstrument {
public:
    Trombone() : BrassInstrument("Trombone") {
        recipe.filter_cutoff = 4600.0; recipe.formant1_hz = 700.0; recipe.pitch_attack_time = 0.06;
    }
};

class BassTrombone : public BrassInstrument {
public:
    BassTrombone() : BrassInstrument("Bass Trombone") {
        recipe.filter_cutoff = 3200.0; recipe.formant1_hz = 500.0; recipe.osc_mix = 0.4;
    }
};

class FrenchHorn : public BrassInstrument {
public:
    FrenchHorn() : BrassInstrument("French Horn") {
        recipe.filter_cutoff = 3600.0; recipe.formant1_hz = 600.0; recipe.osc1_shape = WaveShape::BL_Triangle;
        recipe.pitch_attack_cents = 20.0;
    }
};

class FrenchHornEnsemble : public BrassInstrument {
public:
    FrenchHornEnsemble() : BrassInstrument("French Horn Ensemble") {
        recipe.filter_cutoff = 3600.0; recipe.formant1_hz = 600.0; recipe.osc1_shape = WaveShape::BL_Triangle;
        recipe.pitch_attack_cents = 20.0; recipe.detune_cents = 16.0; recipe.stereo_width = 0.35;
    }
};

class Euphonium : public BrassInstrument {
public:
    Euphonium() : BrassInstrument("Euphonium") {
        recipe.filter_cutoff = 2800.0; recipe.formant1_hz = 450.0; recipe.osc_mix = 0.35;
    }
};

class BaritoneHorn : public BrassInstrument {
public:
    BaritoneHorn() : BrassInstrument("Baritone Horn") {
        recipe.filter_cutoff = 2600.0; recipe.formant1_hz = 400.0; recipe.osc_mix = 0.4;
    }
};

class Tuba : public BrassInstrument {
public:
    Tuba() : BrassInstrument("Tuba") {
        recipe.filter_cutoff = 1800.0; recipe.formant1_hz = 250.0; recipe.osc_mix = 0.5;
        recipe.pitch_attack_time = 0.08;
    }
};

class Sousaphone : public BrassInstrument {
public:
    Sousaphone() : BrassInstrument("Sousaphone") {
        recipe.filter_cutoff = 1600.0; recipe.formant1_hz = 220.0; recipe.osc_mix = 0.55;
        recipe.pitch_attack_time = 0.09; recipe.stereo_width = 0.2;
    }
};

class WagnerTuba : public BrassInstrument {
public:
    WagnerTuba() : BrassInstrument("Wagner Tuba") {
        recipe.filter_cutoff = 2200.0; recipe.formant1_hz = 350.0; recipe.osc_mix = 0.45;
        recipe.osc1_shape = WaveShape::BL_Triangle;
    }
};

class BassCornet : public BrassInstrument {
public:
    BassCornet() : BrassInstrument("Bass Cornet") {
        recipe.filter_cutoff = 2900.0; recipe.formant1_hz = 480.0; recipe.osc_mix = 0.38;
        recipe.pitch_attack_cents = 25.0;
    }
};

class BrassSection : public BrassInstrument {
public:
    BrassSection() : BrassInstrument("Brass Section") {
        recipe.filter_cutoff = 5200.0; recipe.formant1_hz = 1100.0; recipe.detune_cents = 18.0;
        recipe.stereo_width = 0.4; recipe.pitch_attack_cents = 30.0;
    }
};

class LowBrassSection : public BrassInstrument {
public:
    LowBrassSection() : BrassInstrument("Low Brass Section") {
        recipe.filter_cutoff = 2400.0; recipe.formant1_hz = 400.0; recipe.detune_cents = 18.0;
        recipe.osc_mix = 0.45; recipe.stereo_width = 0.4;
    }
};

class NaturalHorn : public BrassInstrument {
public:
    NaturalHorn() : BrassInstrument("Natural Horn") {
        recipe.filter_cutoff = 3200.0; recipe.formant1_hz = 580.0; recipe.osc1_shape = WaveShape::BL_Triangle;
        recipe.noise_mix = 0.03; recipe.pitch_attack_cents = 15.0;
    }
};

class Alphorn : public BrassInstrument {
public:
    Alphorn() : BrassInstrument("Alphorn") {
        recipe.filter_cutoff = 1400.0; recipe.formant1_hz = 220.0; recipe.osc_mix = 0.5;
        recipe.pitch_attack_time = 0.1; recipe.envelope.attack = 0.05; recipe.stereo_width = 0.3;
    }
};

class BaroqueTrumpet : public BrassInstrument {
public:
    BaroqueTrumpet() : BrassInstrument("Baroque Trumpet") {
        recipe.filter_cutoff = 6200.0; recipe.formant1_hz = 1500.0; recipe.noise_mix = 0.025;
        recipe.pitch_attack_cents = 25.0;
    }
};

class BaroqueTrombone : public BrassInstrument {
public:
    BaroqueTrombone() : BrassInstrument("Baroque Trombone") {
        recipe.filter_cutoff = 3600.0; recipe.formant1_hz = 620.0; recipe.noise_mix = 0.02;
        recipe.pitch_attack_time = 0.07;
    }
};

class Serpent : public BrassInstrument {
public:
    Serpent() : BrassInstrument("Serpent") {
        recipe.filter_cutoff = 1600.0; recipe.formant1_hz = 260.0; recipe.osc_mix = 0.5;
        recipe.noise_mix = 0.04; recipe.pitch_attack_time = 0.08;
    }
};

class Ophicleide : public BrassInstrument {
public:
    Ophicleide() : BrassInstrument("Ophicleide") {
        recipe.filter_cutoff = 2000.0; recipe.formant1_hz = 320.0; recipe.osc_mix = 0.48;
        recipe.noise_mix = 0.035; recipe.pitch_attack_cents = 15.0;
    }
};

}
}
