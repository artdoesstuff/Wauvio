#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class Kick : public PercussionInstrument {
public:
    Kick() : PercussionInstrument("Kick") {
        recipe.fixed_pitch_hz = 58.0; recipe.noise_mix = 0.08; recipe.filter_cutoff = 250.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.28, 0.0, 0.05};
        recipe.pitch_attack_cents = -1200.0; recipe.pitch_attack_time = 0.06;
    }
};
using KickDrum = Kick;

class ConcertBassDrum : public PercussionInstrument {
public:
    ConcertBassDrum() : PercussionInstrument("Concert Bass Drum") {
        recipe.fixed_pitch_hz = 42.0; recipe.noise_mix = 0.12; recipe.filter_cutoff = 180.0;
        recipe.envelope = DAHDSR{0.0, 0.003, 0.0, 0.9, 0.0, 0.3};
        recipe.stereo_width = 0.2;
    }
};

class Snare : public PercussionInstrument {
public:
    Snare() : PercussionInstrument("Snare") {
        recipe.fixed_pitch_hz = 180.0; recipe.noise_mix = 0.65; recipe.pink_noise = false;
        recipe.filter_cutoff = 3500.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.12, 0.0, 0.05};
    }
};

class ConcertSnare : public PercussionInstrument {
public:
    ConcertSnare() : PercussionInstrument("Concert Snare") {
        recipe.fixed_pitch_hz = 220.0; recipe.noise_mix = 0.55; recipe.pink_noise = false;
        recipe.filter_cutoff = 4200.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.3, 0.0, 0.15};
        recipe.stereo_width = 0.2;
    }
};

class MarchingSnare : public PercussionInstrument {
public:
    MarchingSnare() : PercussionInstrument("Marching Snare") {
        recipe.fixed_pitch_hz = 320.0; recipe.noise_mix = 0.5; recipe.pink_noise = false;
        recipe.filter_cutoff = 5200.0; recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.1, 0.0, 0.04};
    }
};

class FieldDrum : public PercussionInstrument {
public:
    FieldDrum() : PercussionInstrument("Field Drum") {
        recipe.fixed_pitch_hz = 200.0; recipe.noise_mix = 0.4; recipe.filter_cutoff = 3000.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.4, 0.0, 0.15};
    }
};

class TenorDrum : public PercussionInstrument {
public:
    TenorDrum() : PercussionInstrument("Tenor Drum") {
        recipe.fixed_pitch_hz = 260.0; recipe.noise_mix = 0.25; recipe.filter_cutoff = 2600.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.3, 0.0, 0.1};
    }
};

class Rimshot : public PercussionInstrument {
public:
    Rimshot() : PercussionInstrument("Rimshot") {
        recipe.fixed_pitch_hz = 900.0; recipe.noise_mix = 0.4; recipe.filter_cutoff = 5500.0;
        recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.05, 0.0, 0.02};
    }
};

class Tom : public PercussionInstrument {
public:
    Tom() : PercussionInstrument("Tom") {
        recipe.fixed_pitch_hz = 150.0; recipe.noise_mix = 0.15; recipe.filter_cutoff = 1600.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.35, 0.0, 0.08};
    }
};
using RackTom = Tom;

class FloorTom : public PercussionInstrument {
public:
    FloorTom() : PercussionInstrument("Floor Tom") {
        recipe.fixed_pitch_hz = 90.0; recipe.noise_mix = 0.15; recipe.filter_cutoff = 900.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.45, 0.0, 0.1};
    }
};

class ClosedHiHat : public PercussionInstrument {
public:
    ClosedHiHat() : PercussionInstrument("Closed Hi-Hat") {
        recipe.fixed_pitch_hz = 3000.0; recipe.noise_mix = 0.85; recipe.pink_noise = false;
        recipe.filter_cutoff = 9000.0; recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.04, 0.0, 0.02};
    }
};
using HiHat = ClosedHiHat;

class OpenHiHat : public PercussionInstrument {
public:
    OpenHiHat() : PercussionInstrument("Open Hi-Hat") {
        recipe.fixed_pitch_hz = 3200.0; recipe.noise_mix = 0.85; recipe.pink_noise = false;
        recipe.filter_cutoff = 9000.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.35, 0.0, 0.1};
    }
};

class SuspendedCymbal : public PercussionInstrument {
public:
    SuspendedCymbal() : PercussionInstrument("Suspended Cymbal") {
        recipe.fixed_pitch_hz = 2200.0; recipe.noise_mix = 0.75; recipe.pink_noise = false;
        recipe.filter_cutoff = 8500.0; recipe.envelope = DAHDSR{0.0, 0.003, 0.0, 2.2, 0.0, 0.8};
        recipe.stereo_width = 0.3;
    }
};

class CymbalRoll : public PercussionInstrument {
public:
    CymbalRoll() : PercussionInstrument("Cymbal Roll") {
        recipe.fixed_pitch_hz = 2200.0; recipe.noise_mix = 0.8; recipe.pink_noise = false;
        recipe.filter_cutoff = 8500.0; recipe.envelope = DAHDSR{0.0, 0.05, 0.0, 0.2, 0.7, 0.5};
        recipe.stereo_width = 0.35;
    }
};

class CrashCymbal : public PercussionInstrument {
public:
    CrashCymbal() : PercussionInstrument("Crash Cymbal") {
        recipe.fixed_pitch_hz = 2400.0; recipe.noise_mix = 0.8; recipe.pink_noise = false;
        recipe.filter_cutoff = 8000.0; recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 1.6, 0.0, 0.5};
        recipe.stereo_width = 0.3;
    }
};

class RideCymbal : public PercussionInstrument {
public:
    RideCymbal() : PercussionInstrument("Ride Cymbal") {
        recipe.fixed_pitch_hz = 2000.0; recipe.noise_mix = 0.55; recipe.pink_noise = false;
        recipe.filter_cutoff = 7000.0; recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 1.1, 0.0, 0.3};
    }
};

class RideBell : public PercussionInstrument {
public:
    RideBell() : PercussionInstrument("Ride Bell") {
        recipe.fixed_pitch_hz = 2600.0; recipe.noise_mix = 0.25; recipe.filter_cutoff = 6500.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.6, 0.0, 0.15};
    }
};

class ChinaCymbal : public PercussionInstrument {
public:
    ChinaCymbal() : PercussionInstrument("China Cymbal") {
        recipe.fixed_pitch_hz = 2800.0; recipe.noise_mix = 0.85; recipe.pink_noise = false;
        recipe.filter_cutoff = 9500.0; recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 1.0, 0.0, 0.4};
    }
};

class SplashCymbal : public PercussionInstrument {
public:
    SplashCymbal() : PercussionInstrument("Splash Cymbal") {
        recipe.fixed_pitch_hz = 3400.0; recipe.noise_mix = 0.8; recipe.pink_noise = false;
        recipe.filter_cutoff = 9500.0; recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.5, 0.0, 0.15};
    }
};

class Gong : public PercussionInstrument {
public:
    Gong() : PercussionInstrument("Gong") {
        recipe.fixed_pitch_hz = 110.0; recipe.noise_mix = 0.45; recipe.pink_noise = false;
        recipe.filter_cutoff = 5000.0; recipe.envelope = DAHDSR{0.0, 0.02, 0.0, 4.0, 0.0, 2.0};
        recipe.pitch_instability = 8.0; recipe.stereo_width = 0.4;
    }
};
using TamTam = Gong;

class BrakeDrum : public PercussionInstrument {
public:
    BrakeDrum() : PercussionInstrument("Brake Drum") {
        recipe.fixed_pitch_hz = 1200.0; recipe.noise_mix = 0.3; recipe.filter_cutoff = 6000.0;
        recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 1.0, 0.0, 0.4};
    }
};

class Anvil : public PercussionInstrument {
public:
    Anvil() : PercussionInstrument("Anvil") {
        recipe.fixed_pitch_hz = 1800.0; recipe.noise_mix = 0.15; recipe.filter_cutoff = 7000.0;
        recipe.envelope = DAHDSR{0.0, 0.0003, 0.0, 0.6, 0.0, 0.2};
    }
};

class Ratchet : public PercussionInstrument {
public:
    Ratchet() : PercussionInstrument("Ratchet") {
        recipe.noise_mix = 1.0; recipe.pink_noise = false; recipe.filter_cutoff = 5000.0;
        recipe.envelope = DAHDSR{0.0, 0.005, 0.0, 0.5, 0.3, 0.1};
    }
};

class Whip : public PercussionInstrument {
public:
    Whip() : PercussionInstrument("Whip") {
        recipe.noise_mix = 1.0; recipe.pink_noise = false; recipe.filter_cutoff = 8000.0;
        recipe.envelope = DAHDSR{0.0, 0.0002, 0.0, 0.04, 0.0, 0.01};
    }
};

class TempleBlocks : public PercussionInstrument {
public:
    TempleBlocks() : PercussionInstrument("Temple Blocks") {
        recipe.fixed_pitch_hz = 1100.0; recipe.noise_mix = 0.08; recipe.filter_cutoff = 3800.0;
        recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.1, 0.0, 0.03};
    }
};

class Woodblock : public PercussionInstrument {
public:
    Woodblock() : PercussionInstrument("Woodblock") {
        recipe.fixed_pitch_hz = 1400.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 4000.0;
        recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.05, 0.0, 0.02};
    }
};

class Castanets : public PercussionInstrument {
public:
    Castanets() : PercussionInstrument("Castanets") {
        recipe.fixed_pitch_hz = 2200.0; recipe.noise_mix = 0.3; recipe.filter_cutoff = 5500.0;
        recipe.envelope = DAHDSR{0.0, 0.0003, 0.0, 0.04, 0.0, 0.02};
    }
};

class FingerCymbals : public PercussionInstrument {
public:
    FingerCymbals() : PercussionInstrument("Finger Cymbals") {
        recipe.fixed_pitch_hz = 6500.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 11000.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 1.5, 0.0, 0.4};
    }
};

class BellTree : public PercussionInstrument {
public:
    BellTree() : PercussionInstrument("Bell Tree") {
        recipe.fixed_pitch_hz = 4200.0; recipe.noise_mix = 0.05; recipe.filter_cutoff = 9500.0;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 1.2, 0.0, 0.4}; recipe.stereo_width = 0.4;
    }
};

class WindChimes : public PercussionInstrument {
public:
    WindChimes() : PercussionInstrument("Wind Chimes") {
        recipe.fixed_pitch_hz = 3800.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 9000.0;
        recipe.envelope = DAHDSR{0.0, 0.02, 0.0, 1.8, 0.2, 0.6};
        recipe.pitch_instability = 20.0; recipe.stereo_width = 0.5;
    }
};
using MarkTree = WindChimes;

class RainStick : public PercussionInstrument {
public:
    RainStick() : PercussionInstrument("Rain Stick") {
        recipe.noise_mix = 1.0; recipe.pink_noise = true; recipe.filter_cutoff = 5000.0;
        recipe.envelope = DAHDSR{0.0, 0.3, 0.0, 1.5, 0.3, 1.0}; recipe.stereo_width = 0.4;
    }
};

class ThunderSheet : public PercussionInstrument {
public:
    ThunderSheet() : PercussionInstrument("Thunder Sheet") {
        recipe.fixed_pitch_hz = 60.0; recipe.noise_mix = 0.6; recipe.pink_noise = true;
        recipe.filter_cutoff = 1500.0; recipe.envelope = DAHDSR{0.0, 0.1, 0.0, 2.5, 0.2, 1.5};
        recipe.pitch_instability = 30.0; recipe.stereo_width = 0.5;
    }
};

class Tambourine : public PercussionInstrument {
public:
    Tambourine() : PercussionInstrument("Tambourine") {
        recipe.fixed_pitch_hz = 4500.0; recipe.noise_mix = 0.75; recipe.pink_noise = false;
        recipe.filter_cutoff = 8000.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.2, 0.0, 0.06};
    }
};

class Shaker : public PercussionInstrument {
public:
    Shaker() : PercussionInstrument("Shaker") {
        recipe.noise_mix = 1.0; recipe.pink_noise = false; recipe.filter_cutoff = 7000.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.1, 0.0, 0.04};
    }
};

class Maracas : public PercussionInstrument {
public:
    Maracas() : PercussionInstrument("Maracas") {
        recipe.noise_mix = 1.0; recipe.pink_noise = false; recipe.filter_cutoff = 6500.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.08, 0.0, 0.03};
    }
};

class Cowbell : public PercussionInstrument {
public:
    Cowbell() : PercussionInstrument("Cowbell") {
        recipe.fixed_pitch_hz = 560.0; recipe.noise_mix = 0.05; recipe.filter_cutoff = 3000.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.2, 0.0, 0.05};
    }
};

class Triangle : public PercussionInstrument {
public:
    Triangle() : PercussionInstrument("Triangle") {
        recipe.fixed_pitch_hz = 5200.0; recipe.noise_mix = 0.05; recipe.filter_cutoff = 10000.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 1.2, 0.0, 0.3};
    }
};

class Cabasa : public PercussionInstrument {
public:
    Cabasa() : PercussionInstrument("Cabasa") {
        recipe.noise_mix = 1.0; recipe.pink_noise = false; recipe.filter_cutoff = 6000.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.15, 0.0, 0.05};
    }
};

class Guiro : public PercussionInstrument {
public:
    Guiro() : PercussionInstrument("Guiro") {
        recipe.noise_mix = 0.9; recipe.pink_noise = false; recipe.filter_cutoff = 4500.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.3, 0.0, 0.06};
    }
};

}
}
