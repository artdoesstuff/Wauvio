#pragma once
#include "../../core/model_instruments.hpp"
#include "orchestral.hpp"
#include "latin.hpp"
#include "tuned.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class JazzKit : public DrumKit {
public:
    JazzKit() : DrumKit("Jazz Kit") {
        map(36, std::make_shared<Kick>());
        map(38, std::make_shared<Snare>());
        map(40, std::make_shared<Rimshot>());
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(49, std::make_shared<CrashCymbal>());
        map(51, std::make_shared<RideCymbal>());
        map(53, std::make_shared<RideBell>());
        map(45, std::make_shared<Tom>());
        map(41, std::make_shared<FloorTom>());
    }
};

class RockKit : public DrumKit {
public:
    RockKit() : DrumKit("Rock Kit") {
        map(36, std::make_shared<Kick>());
        map(38, std::make_shared<Snare>());
        map(40, std::make_shared<Rimshot>());
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(49, std::make_shared<CrashCymbal>());
        map(51, std::make_shared<RideCymbal>());
        map(45, std::make_shared<Tom>());
        map(47, std::make_shared<Tom>());
        map(41, std::make_shared<FloorTom>());
        map(43, std::make_shared<FloorTom>());
        map(52, std::make_shared<ChinaCymbal>());
        map(55, std::make_shared<SplashCymbal>());
    }
};

class StudioKit : public DrumKit {
public:
    StudioKit() : DrumKit("Studio Kit") {
        map(36, std::make_shared<Kick>());
        map(38, std::make_shared<Snare>());
        map(40, std::make_shared<ConcertSnare>());
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(49, std::make_shared<CrashCymbal>());
        map(51, std::make_shared<RideCymbal>());
        map(45, std::make_shared<Tom>());
        map(41, std::make_shared<FloorTom>());
        map(56, std::make_shared<Cowbell>());
        map(54, std::make_shared<Tambourine>());
    }
};

class VintageKit : public DrumKit {
public:
    VintageKit() : DrumKit("Vintage Kit") {
        auto kick = std::make_shared<Kick>();
        kick->recipe.filter_cutoff = 180.0; kick->recipe.noise_mix = 0.12;
        map(36, kick);
        auto snare = std::make_shared<Snare>();
        snare->recipe.filter_cutoff = 2600.0;
        map(38, snare);
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(49, std::make_shared<CrashCymbal>());
        map(51, std::make_shared<RideCymbal>());
        map(45, std::make_shared<Tom>());
        map(41, std::make_shared<FloorTom>());
    }
};

class ElectronicKit : public DrumKit {
public:
    ElectronicKit() : DrumKit("Electronic Kit") {
        auto kick = std::make_shared<Kick>();
        kick->recipe.filter_cutoff = 320.0; kick->recipe.pitch_attack_cents = -2400.0;
        map(36, kick);
        auto snare = std::make_shared<Snare>();
        snare->recipe.filter_cutoff = 5200.0; snare->recipe.pink_noise = false;
        map(38, snare);
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(49, std::make_shared<CrashCymbal>());
        map(39, std::make_shared<Claves>());
        map(37, std::make_shared<Rimshot>());
    }
};

class IndustrialKit : public DrumKit {
public:
    IndustrialKit() : DrumKit("Industrial Kit") {
        map(36, std::make_shared<ConcertBassDrum>());
        map(38, std::make_shared<BrakeDrum>());
        map(42, std::make_shared<Anvil>());
        map(46, std::make_shared<Ratchet>());
        map(49, std::make_shared<ThunderSheet>());
        map(56, std::make_shared<Whip>());
    }
};

class OrchestralPercussionKit : public DrumKit {
public:
    OrchestralPercussionKit() : DrumKit("Orchestral Percussion Kit") {
        map(35, std::make_shared<ConcertBassDrum>());
        map(38, std::make_shared<ConcertSnare>());
        map(49, std::make_shared<SuspendedCymbal>());
        map(52, std::make_shared<TamTam>());
        map(56, std::make_shared<Cowbell>());
        map(59, std::make_shared<RideCymbal>());
        map(60, std::make_shared<Bongo>());
        map(76, std::make_shared<Woodblock>());
        map(80, std::make_shared<Triangle>());
    }
};

class LoFiKit : public DrumKit {
public:
    LoFiKit() : DrumKit("Lo-Fi Kit") {
        auto kick = std::make_shared<Kick>();
        kick->recipe.filter_cutoff = 140.0; kick->recipe.pitch_instability = 6.0;
        map(36, kick);
        auto snare = std::make_shared<Snare>();
        snare->recipe.filter_cutoff = 2200.0; snare->recipe.pitch_instability = 8.0;
        map(38, snare);
        auto hat = std::make_shared<ClosedHiHat>();
        hat->recipe.filter_cutoff = 6000.0;
        map(42, hat);
        map(46, std::make_shared<OpenHiHat>());
    }
};

class Kit808 : public DrumKit {
public:
    Kit808() : DrumKit("808 Kit") {
        auto kick = std::make_shared<Kick>();
        kick->recipe.fixed_pitch_hz = 45.0; kick->recipe.envelope.decay = 0.6;
        kick->recipe.pitch_attack_cents = -1800.0;
        map(36, kick);
        auto snare = std::make_shared<Snare>();
        snare->recipe.filter_cutoff = 4000.0;
        map(38, snare);
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(39, std::make_shared<Claves>());
        map(56, std::make_shared<Cowbell>());
    }
};

class Kit909 : public DrumKit {
public:
    Kit909() : DrumKit("909 Kit") {
        auto kick = std::make_shared<Kick>();
        kick->recipe.fixed_pitch_hz = 60.0; kick->recipe.envelope.decay = 0.32;
        kick->recipe.pitch_attack_cents = -1400.0;
        map(36, kick);
        auto snare = std::make_shared<Snare>();
        snare->recipe.filter_cutoff = 4800.0; snare->recipe.noise_mix = 0.7;
        map(38, snare);
        map(42, std::make_shared<ClosedHiHat>());
        map(46, std::make_shared<OpenHiHat>());
        map(49, std::make_shared<CrashCymbal>());
        map(51, std::make_shared<RideCymbal>());
    }
};

}
}
