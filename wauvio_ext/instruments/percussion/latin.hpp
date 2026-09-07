#pragma once
#include "../../core/model_instruments.hpp"

namespace wauvio {
namespace instruments {
using namespace wauvio::audio;

class Surdo : public PercussionInstrument {
public:
    Surdo() : PercussionInstrument("Surdo") {
        recipe.fixed_pitch_hz = 75.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 700.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.5, 0.0, 0.15};
    }
};

class Repinique : public PercussionInstrument {
public:
    Repinique() : PercussionInstrument("Repinique") {
        recipe.fixed_pitch_hz = 260.0; recipe.noise_mix = 0.15; recipe.filter_cutoff = 2600.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.22, 0.0, 0.06};
    }
};

class Timbales : public PercussionInstrument {
public:
    Timbales() : PercussionInstrument("Timbales") {
        recipe.fixed_pitch_hz = 420.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 3400.0;
        recipe.envelope = DAHDSR{0.0, 0.0008, 0.0, 0.2, 0.0, 0.05};
    }
};

class Agogo : public PercussionInstrument {
public:
    Agogo() : PercussionInstrument("Agogo") {
        recipe.fixed_pitch_hz = 900.0; recipe.noise_mix = 0.05; recipe.filter_cutoff = 5000.0;
        recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.25, 0.0, 0.08};
    }
};

class Claves : public PercussionInstrument {
public:
    Claves() : PercussionInstrument("Claves") {
        recipe.fixed_pitch_hz = 2500.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 4500.0;
        recipe.envelope = DAHDSR{0.0, 0.0005, 0.0, 0.06, 0.0, 0.02};
    }
};

class Cuica : public PercussionInstrument {
public:
    Cuica() : PercussionInstrument("Cuica") {
        recipe.fixed_pitch_hz = 250.0; recipe.noise_mix = 0.3; recipe.filter_cutoff = 1800.0;
        recipe.envelope = DAHDSR{0.0, 0.01, 0.0, 0.3, 0.4, 0.1};
        recipe.pitch_instability = 40.0;
    }
};

class Pandeiro : public PercussionInstrument {
public:
    Pandeiro() : PercussionInstrument("Pandeiro") {
        recipe.fixed_pitch_hz = 3600.0; recipe.noise_mix = 0.6; recipe.pink_noise = false;
        recipe.filter_cutoff = 7500.0; recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.18, 0.0, 0.06};
    }
};

class Berimbau : public PercussionInstrument {
public:
    Berimbau() : PercussionInstrument("Berimbau") {
        recipe.fixed_pitch_hz = 150.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 2200.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.5, 0.1, 0.2}; recipe.pitch_instability = 12.0;
    }
};

class Bongo : public PercussionInstrument {
public:
    Bongo() : PercussionInstrument("Bongo") {
        recipe.fixed_pitch_hz = 340.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 2400.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.18, 0.0, 0.05};
    }
};
using Bongos = Bongo;

class Conga : public PercussionInstrument {
public:
    Conga() : PercussionInstrument("Conga") {
        recipe.fixed_pitch_hz = 220.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 1800.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.25, 0.0, 0.06};
    }
};
using Congas = Conga;

class BataDrums : public PercussionInstrument {
public:
    BataDrums() : PercussionInstrument("Bata Drums") {
        recipe.fixed_pitch_hz = 260.0; recipe.noise_mix = 0.12; recipe.filter_cutoff = 2200.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.22, 0.0, 0.06};
    }
};

class FrameDrum : public PercussionInstrument {
public:
    FrameDrum() : PercussionInstrument("Frame Drum") {
        recipe.fixed_pitch_hz = 130.0; recipe.noise_mix = 0.2; recipe.filter_cutoff = 1600.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.4, 0.0, 0.12};
    }
};

class Darbuka : public PercussionInstrument {
public:
    Darbuka() : PercussionInstrument("Darbuka") {
        recipe.fixed_pitch_hz = 220.0; recipe.noise_mix = 0.25; recipe.filter_cutoff = 3200.0;
        recipe.envelope = DAHDSR{0.0, 0.0006, 0.0, 0.2, 0.0, 0.05};
    }
};
using Doumbek = Darbuka;

class Riq : public PercussionInstrument {
public:
    Riq() : PercussionInstrument("Riq") {
        recipe.fixed_pitch_hz = 3800.0; recipe.noise_mix = 0.65; recipe.pink_noise = false;
        recipe.filter_cutoff = 8500.0; recipe.envelope = DAHDSR{0.0, 0.0008, 0.0, 0.22, 0.0, 0.06};
    }
};

class Bendir : public PercussionInstrument {
public:
    Bendir() : PercussionInstrument("Bendir") {
        recipe.fixed_pitch_hz = 110.0; recipe.noise_mix = 0.25; recipe.filter_cutoff = 1400.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.5, 0.0, 0.18};
    }
};

class Tabla : public PercussionInstrument {
public:
    Tabla() : PercussionInstrument("Tabla") {
        recipe.fixed_pitch_hz = 180.0; recipe.noise_mix = 0.08; recipe.filter_cutoff = 2000.0;
        recipe.envelope = DAHDSR{0.0, 0.0008, 0.0, 0.3, 0.0, 0.08};
        recipe.pitch_attack_cents = -300.0; recipe.pitch_attack_time = 0.04;
    }
};

class Udu : public PercussionInstrument {
public:
    Udu() : PercussionInstrument("Udu") {
        recipe.fixed_pitch_hz = 130.0; recipe.noise_mix = 0.2; recipe.filter_cutoff = 900.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.4, 0.0, 0.15};
    }
};

class Cajon : public PercussionInstrument {
public:
    Cajon() : PercussionInstrument("Cajon") {
        recipe.fixed_pitch_hz = 100.0; recipe.noise_mix = 0.35; recipe.filter_cutoff = 3000.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.2, 0.0, 0.06};
    }
};

class Djembe : public PercussionInstrument {
public:
    Djembe() : PercussionInstrument("Djembe") {
        recipe.fixed_pitch_hz = 180.0; recipe.noise_mix = 0.2; recipe.filter_cutoff = 2200.0;
        recipe.envelope = DAHDSR{0.0, 0.001, 0.0, 0.3, 0.0, 0.08};
    }
};

class Taiko : public PercussionInstrument {
public:
    Taiko() : PercussionInstrument("Taiko") {
        recipe.fixed_pitch_hz = 65.0; recipe.noise_mix = 0.1; recipe.filter_cutoff = 800.0;
        recipe.envelope = DAHDSR{0.0, 0.002, 0.0, 0.6, 0.0, 0.2}; recipe.stereo_width = 0.25;
    }
};

}
}
