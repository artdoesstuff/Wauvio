#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class AcousticGuitar : public GuitarInstrument {
public:
    AcousticGuitar() : GuitarInstrument("Acoustic Guitar") {
        recipe.filter_cutoff = 4500.0; recipe.envelope.decay = 0.9;
    }
};

class NylonGuitar : public GuitarInstrument {
public:
    NylonGuitar() : GuitarInstrument("Nylon Guitar") {
        recipe.filter_cutoff = 3400.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.noise_attack_burst = 0.14; recipe.envelope.decay = 1.0;
    }
};

class SteelStringGuitar : public GuitarInstrument {
public:
    SteelStringGuitar() : GuitarInstrument("Steel-String Guitar") {
        recipe.filter_cutoff = 6200.0; recipe.noise_attack_burst = 0.28; recipe.envelope.decay = 0.8;
    }
};
using SteelGuitar = SteelStringGuitar;

class ClassicalGuitar : public GuitarInstrument {
public:
    ClassicalGuitar() : GuitarInstrument("Classical Guitar") {
        recipe.filter_cutoff = 3200.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.envelope.decay = 1.1; recipe.noise_attack_burst = 0.1;
    }
};

class TwelveStringGuitar : public GuitarInstrument {
public:
    TwelveStringGuitar() : GuitarInstrument("12-String Guitar") {
        recipe.filter_cutoff = 5800.0; recipe.detune_cents = 12.0; recipe.stereo_width = 0.35;
        recipe.envelope.decay = 1.0;
    }
};

class FingerpickedAcousticGuitar : public GuitarInstrument {
public:
    FingerpickedAcousticGuitar() : GuitarInstrument("Fingerpicked Acoustic Guitar") {
        recipe.filter_cutoff = 4200.0; recipe.noise_attack_burst = 0.08; recipe.envelope.decay = 1.0;
        recipe.gain = 0.85;
    }
};

class StrummedAcousticGuitar : public GuitarInstrument {
public:
    StrummedAcousticGuitar() : GuitarInstrument("Strummed Acoustic Guitar") {
        recipe.filter_cutoff = 5000.0; recipe.detune_cents = 8.0; recipe.noise_attack_burst = 0.22;
        recipe.envelope.decay = 0.75; recipe.stereo_width = 0.3;
    }
};

class AcousticGuitarHarmonics : public GuitarInstrument {
public:
    AcousticGuitarHarmonics() : GuitarInstrument("Acoustic Guitar Harmonics") {
        recipe.filter_cutoff = 9000.0; recipe.gain = 0.55; recipe.envelope.decay = 1.4;
        recipe.art_pitch_shift[Articulation::Sustain] = 12.0;
    }
};

class ElectricGuitar : public GuitarInstrument {
public:
    ElectricGuitar() : GuitarInstrument("Electric Guitar") {
        recipe.filter_cutoff = 5200.0; recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.35;
        recipe.envelope.decay = 0.6;
    }
};

class CleanElectricGuitar : public GuitarInstrument {
public:
    CleanElectricGuitar() : GuitarInstrument("Clean Electric Guitar") {
        recipe.filter_cutoff = 4000.0; recipe.envelope.decay = 0.85; recipe.noise_attack_burst = 0.1;
    }
};

class OverdrivenElectricGuitar : public GuitarInstrument {
public:
    OverdrivenElectricGuitar() : GuitarInstrument("Overdriven Electric Guitar") {
        recipe.filter_cutoff = 4800.0; recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.45;
        recipe.envelope.decay = 0.7;
        distortion(1.6f);
    }
};

class DistortedElectricGuitar : public GuitarInstrument {
public:
    DistortedElectricGuitar() : GuitarInstrument("Distorted Electric Guitar") {
        recipe.filter_cutoff = 5600.0; recipe.osc1_shape = WaveShape::BL_Square; recipe.osc_mix = 0.55;
        recipe.envelope.decay = 0.5;
        distortion(3.2f);
    }
};

class PalmMutedElectricGuitar : public GuitarInstrument {
public:
    PalmMutedElectricGuitar() : GuitarInstrument("Palm-Muted Electric Guitar") {
        recipe.filter_cutoff = 1900.0; recipe.envelope.decay = 0.2; recipe.noise_attack_burst = 0.28;
        distortion(1.4f);
    }
};
using MutedElectricGuitar = PalmMutedElectricGuitar;

class ElectricGuitarPowerChords : public GuitarInstrument {
public:
    ElectricGuitarPowerChords() : GuitarInstrument("Electric Guitar Power Chords") {
        recipe.filter_cutoff = 2600.0; recipe.osc_mix = 0.5; recipe.envelope.decay = 0.35;
        distortion(3.4f);
    }
};

class ElectricGuitarHarmonics : public GuitarInstrument {
public:
    ElectricGuitarHarmonics() : GuitarInstrument("Electric Guitar Harmonics") {
        recipe.filter_cutoff = 8500.0; recipe.gain = 0.5; recipe.envelope.decay = 1.6;
        recipe.art_pitch_shift[Articulation::Sustain] = 12.0;
        distortion(1.2f);
    }
};

class Mandolin : public GuitarInstrument {
public:
    Mandolin() : GuitarInstrument("Mandolin") {
        recipe.filter_cutoff = 7200.0; recipe.detune_cents = 10.0; recipe.envelope.decay = 0.35;
        recipe.noise_attack_burst = 0.2;
    }
};

class Banjo : public GuitarInstrument {
public:
    Banjo() : GuitarInstrument("Banjo") {
        recipe.filter_cutoff = 7800.0; recipe.noise_attack_burst = 0.3; recipe.envelope.decay = 0.4;
    }
};

class Ukulele : public GuitarInstrument {
public:
    Ukulele() : GuitarInstrument("Ukulele") {
        recipe.filter_cutoff = 5000.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.envelope.decay = 0.5;
    }
};

class Harp : public GuitarInstrument {
public:
    Harp() : GuitarInstrument("Harp") {
        recipe.filter_cutoff = 8000.0; recipe.osc1_shape = WaveShape::Sine; recipe.osc_mix = 0.15;
        recipe.envelope.decay = 1.8; recipe.stereo_width = 0.3;
    }
};

class Lute : public GuitarInstrument {
public:
    Lute() : GuitarInstrument("Lute") {
        recipe.filter_cutoff = 4000.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.envelope.decay = 0.9; recipe.noise_attack_burst = 0.12;
    }
};

class Theorbo : public GuitarInstrument {
public:
    Theorbo() : GuitarInstrument("Theorbo") {
        recipe.filter_cutoff = 2200.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.envelope.decay = 1.6; recipe.noise_attack_burst = 0.1;
    }
};

class Oud : public GuitarInstrument {
public:
    Oud() : GuitarInstrument("Oud") {
        recipe.filter_cutoff = 3600.0; recipe.detune_cents = 14.0; recipe.envelope.decay = 1.1;
        recipe.noise_attack_burst = 0.18; recipe.formant1_hz = 900.0; recipe.formant1_gain_db = 3.0;
    }
};

class Bouzouki : public GuitarInstrument {
public:
    Bouzouki() : GuitarInstrument("Bouzouki") {
        recipe.filter_cutoff = 6800.0; recipe.detune_cents = 18.0; recipe.envelope.decay = 0.8;
        recipe.noise_attack_burst = 0.22;
    }
};

class Balalaika : public GuitarInstrument {
public:
    Balalaika() : GuitarInstrument("Balalaika") {
        recipe.filter_cutoff = 6400.0; recipe.envelope.decay = 0.3; recipe.noise_attack_burst = 0.24;
    }
};

class Charango : public GuitarInstrument {
public:
    Charango() : GuitarInstrument("Charango") {
        recipe.filter_cutoff = 7400.0; recipe.detune_cents = 16.0; recipe.envelope.decay = 0.45;
        recipe.noise_attack_burst = 0.2;
    }
};

class Cuatro : public GuitarInstrument {
public:
    Cuatro() : GuitarInstrument("Cuatro") {
        recipe.filter_cutoff = 6600.0; recipe.envelope.decay = 0.5; recipe.noise_attack_burst = 0.18;
    }
};

class Autoharp : public GuitarInstrument {
public:
    Autoharp() : GuitarInstrument("Autoharp") {
        recipe.filter_cutoff = 5600.0; recipe.detune_cents = 20.0; recipe.stereo_width = 0.3;
        recipe.envelope.decay = 1.2;
    }
};

class Zither : public GuitarInstrument {
public:
    Zither() : GuitarInstrument("Zither") {
        recipe.filter_cutoff = 6000.0; recipe.detune_cents = 12.0; recipe.envelope.decay = 1.0;
        recipe.noise_attack_burst = 0.15;
    }
};

class Lyre : public GuitarInstrument {
public:
    Lyre() : GuitarInstrument("Lyre") {
        recipe.filter_cutoff = 4800.0; recipe.osc1_shape = WaveShape::Sine; recipe.osc_mix = 0.2;
        recipe.envelope.decay = 1.3; recipe.stereo_width = 0.25;
    }
};

class AppalachianDulcimer : public GuitarInstrument {
public:
    AppalachianDulcimer() : GuitarInstrument("Appalachian Dulcimer") {
        recipe.filter_cutoff = 5500.0; recipe.envelope.decay = 1.0; recipe.noise_attack_burst = 0.15;
    }
};
using Dulcimer = AppalachianDulcimer;

class HammeredDulcimer : public MalletInstrument {
public:
    HammeredDulcimer() : MalletInstrument("Hammered Dulcimer") {
        recipe.fm_ratio = 2.3; recipe.fm_index = 1.0; recipe.filter_cutoff = 6200.0;
        recipe.envelope.decay = 1.1;
    }
};

class BassGuitar : public GuitarInstrument {
public:
    BassGuitar() : GuitarInstrument("Bass Guitar") {
        recipe.filter_cutoff = 1800.0; recipe.osc_mix = 0.15; recipe.envelope.decay = 0.7;
    }
};
using FingerBass = BassGuitar;

class PickBass : public GuitarInstrument {
public:
    PickBass() : GuitarInstrument("Pick Bass") {
        recipe.filter_cutoff = 2400.0; recipe.osc_mix = 0.2; recipe.envelope.decay = 0.5;
        recipe.noise_attack_burst = 0.12;
    }
};

class SlapBass : public GuitarInstrument {
public:
    SlapBass() : GuitarInstrument("Slap Bass") {
        recipe.filter_cutoff = 3200.0; recipe.osc_mix = 0.3; recipe.envelope.decay = 0.3;
        recipe.noise_attack_burst = 0.3;
    }
};

class MutedBass : public GuitarInstrument {
public:
    MutedBass() : GuitarInstrument("Muted Bass") {
        recipe.filter_cutoff = 900.0; recipe.osc_mix = 0.15; recipe.envelope.decay = 0.18;
    }
};

class FretlessBass : public GuitarInstrument {
public:
    FretlessBass() : GuitarInstrument("Fretless Bass") {
        recipe.filter_cutoff = 1500.0; recipe.vibrato_depth_cents = 8.0; recipe.envelope.decay = 0.9;
    }
};

class UprightBassPluck : public GuitarInstrument {
public:
    UprightBassPluck() : GuitarInstrument("Upright Bass Pluck") {
        recipe.filter_cutoff = 1300.0; recipe.osc_mix = 0.5; recipe.envelope.decay = 0.55;
        recipe.noise_attack_burst = 0.1;
    }
};

class ResonatorGuitar : public GuitarInstrument {
public:
    ResonatorGuitar() : GuitarInstrument("Resonator Guitar") {
        recipe.filter_cutoff = 7200.0; recipe.formant1_hz = 2200.0; recipe.formant1_gain_db = 4.0;
        recipe.noise_attack_burst = 0.26; recipe.envelope.decay = 0.7;
    }
};

class Sarod : public GuitarInstrument {
public:
    Sarod() : GuitarInstrument("Sarod") {
        recipe.filter_cutoff = 5600.0; recipe.detune_cents = 14.0; recipe.envelope.decay = 1.5;
        recipe.noise_attack_burst = 0.16; recipe.gain = 0.85;
    }
};

class Santur : public MalletInstrument {
public:
    Santur() : MalletInstrument("Santur") {
        recipe.fm_ratio = 2.2; recipe.fm_index = 1.0; recipe.filter_cutoff = 6000.0;
        recipe.envelope.decay = 1.0;
    }
};

class Kantele : public GuitarInstrument {
public:
    Kantele() : GuitarInstrument("Kantele") {
        recipe.filter_cutoff = 6600.0; recipe.osc1_shape = WaveShape::Triangle;
        recipe.envelope.decay = 1.2; recipe.noise_attack_burst = 0.1;
    }
};

}
}
