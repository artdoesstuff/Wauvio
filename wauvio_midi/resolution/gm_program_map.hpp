#pragma once

#include "../../wauvio_ext.hpp"

#include <cctype>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace wauvio {
namespace midi {

using InstrumentPtr = std::shared_ptr<audio::Instrument>;

template <typename T>
inline InstrumentPtr make_instrument() { return std::make_shared<T>(); }

inline InstrumentPtr make_gm_instrument(int program) {
    using namespace wauvio::instruments;
    switch (program & 0x7F) {
        case 0:  return make_instrument<AcousticGrandPiano>();
        case 1:  return make_instrument<AcousticGrandPiano>();
        case 2:  return make_instrument<AcousticGrandPiano>();
        case 3:  return make_instrument<HonkyTonkPiano>();
        case 4:  return make_instrument<ElectricPiano>();
        case 5:  return make_instrument<CPElectricPiano>();
        case 6:  return make_instrument<Harpsichord>();
        case 7:  return make_instrument<ClavinetD6>();
        case 8:  return make_instrument<Celesta>();
        case 9:  return make_instrument<Glockenspiel>();
        case 10: return make_instrument<MusicBox>();
        case 11: return make_instrument<Vibraphone>();
        case 12: return make_instrument<Marimba>();
        case 13: return make_instrument<Xylophone>();
        case 14: return make_instrument<TubularBells>();
        case 15: return make_instrument<HammeredDulcimer>();
        case 16: return make_instrument<HammondOrgan>();
        case 17: return make_instrument<HammondOrgan>();
        case 18: return make_instrument<HammondOrgan>();
        case 19: return make_instrument<ChurchOrgan>();
        case 20: return make_instrument<ReedOrgan>();
        case 21: return make_instrument<Accordion>();
        case 22: return make_instrument<Harmonica>();
        case 23: return make_instrument<Bandoneon>();
        case 24: return make_instrument<NylonGuitar>();
        case 25: return make_instrument<SteelStringGuitar>();
        case 26: return make_instrument<CleanElectricGuitar>();
        case 27: return make_instrument<CleanElectricGuitar>();
        case 28: return make_instrument<PalmMutedElectricGuitar>();
        case 29: return make_instrument<OverdrivenElectricGuitar>();
        case 30: return make_instrument<DistortedElectricGuitar>();
        case 31: return make_instrument<ElectricGuitarHarmonics>();
        case 32: return make_instrument<UprightBassPluck>();
        case 33: return make_instrument<BassGuitar>();
        case 34: return make_instrument<PickBass>();
        case 35: return make_instrument<FretlessBass>();
        case 36: return make_instrument<SlapBass>();
        case 37: return make_instrument<SlapBass>();
        case 38: return make_instrument<SynthBass>();
        case 39: return make_instrument<SynthBass>();
        case 40: return make_instrument<SoloViolin>();
        case 41: return make_instrument<SoloViola>();
        case 42: return make_instrument<SoloCello>();
        case 43: return make_instrument<DoubleBass>();
        case 44: return make_instrument<TremoloStrings>();
        case 45: return make_instrument<PizzicatoStrings>();
        case 46: return make_instrument<Harp>();
        case 47: return make_instrument<Timpani>();
        case 48: return make_instrument<StringEnsemble>();
        case 49: return make_instrument<ChamberStrings>();
        case 50: return make_instrument<StringSynth>();
        case 51: return make_instrument<StringSynth>();
        case 52: return make_instrument<VocalAah>();
        case 53: return make_instrument<VocalOoh>();
        case 54: return make_instrument<VocalPad>();
        case 55: return make_instrument<BrassSection>();
        case 56: return make_instrument<Trumpet>();
        case 57: return make_instrument<Trombone>();
        case 58: return make_instrument<Tuba>();
        case 59: return make_instrument<Trumpet>();
        case 60: return make_instrument<FrenchHorn>();
        case 61: return make_instrument<BrassSection>();
        case 62: return make_instrument<BrassSynth>();
        case 63: return make_instrument<BrassSynth>();
        case 64: return make_instrument<SopranoSaxophone>();
        case 65: return make_instrument<AltoSaxophone>();
        case 66: return make_instrument<TenorSaxophone>();
        case 67: return make_instrument<BaritoneSaxophone>();
        case 68: return make_instrument<Oboe>();
        case 69: return make_instrument<EnglishHorn>();
        case 70: return make_instrument<Bassoon>();
        case 71: return make_instrument<Clarinet>();
        case 72: return make_instrument<Piccolo>();
        case 73: return make_instrument<Flute>();
        case 74: return make_instrument<SopranoRecorder>();
        case 75: return make_instrument<PanFlute>();
        case 76: return make_instrument<Ocarina>();
        case 77: return make_instrument<Shakuhachi>();
        case 78: return make_instrument<IrishWhistle>();
        case 79: return make_instrument<Ocarina>();
        case 80: return make_instrument<ChiptuneLead>();
        case 81: return make_instrument<SynthLead>();
        case 82: return make_instrument<OrganSynth>();
        case 83: return make_instrument<ArpeggioSynth>();
        case 84: return make_instrument<SynthLead>();
        case 85: return make_instrument<VocalPad>();
        case 86: return make_instrument<SynthLead>();
        case 87: return make_instrument<SynthBass>();
        case 88: return make_instrument<AtmosphericSynth>();
        case 89: return make_instrument<SynthPad>();
        case 90: return make_instrument<SynthPad>();
        case 91: return make_instrument<VocalPad>();
        case 92: return make_instrument<StringSynth>();
        case 93: return make_instrument<ResonantMetal>();
        case 94: return make_instrument<AtmosphericSynth>();
        case 95: return make_instrument<Drone>();
        case 96: return make_instrument<AeolianHarp>();
        case 97: return make_instrument<Drone>();
        case 98: return make_instrument<GlassInstruments>();
        case 99: return make_instrument<AtmosphericSynth>();
        case 100: return make_instrument<Bell>();
        case 101: return make_instrument<GranularVocal>();
        case 102: return make_instrument<TapeStrings>();
        case 103: return make_instrument<ResonantMetal>();
        case 104: return make_instrument<Sitar>();
        case 105: return make_instrument<Banjo>();
        case 106: return make_instrument<Shamisen>();
        case 107: return make_instrument<Koto>();
        case 108: return make_instrument<Kalimba>();
        case 109: return make_instrument<Bagpipes>();
        case 110: return make_instrument<HardangerFiddle>();
        case 111: return make_instrument<Shehnai>();
        case 112: return make_instrument<Glockenspiel>();
        case 113: return make_instrument<Agogo>();
        case 114: return make_instrument<SteelPan>();
        case 115: return make_instrument<Woodblock>();
        case 116: return make_instrument<Taiko>();
        case 117: return make_instrument<Tom>();
        case 118: return make_instrument<Kick>();
        case 119: return make_instrument<CrashCymbal>();
        case 120: return make_instrument<FoundPercussion>();
        case 121: return make_instrument<Breath>();
        case 122: return make_instrument<RainStick>();
        case 123: return make_instrument<Piccolo>();
        case 124: return make_instrument<Bell>();
        case 125: return make_instrument<ThunderSheet>();
        case 126: return make_instrument<Shaker>();
        case 127: return make_instrument<Whip>();
        default: return make_instrument<AcousticGrandPiano>();
    }
}

inline InstrumentPtr make_gm_percussion_instrument(int note) {
    using namespace wauvio::instruments;
    switch (note) {
        case 27: return make_instrument<Claves>();
        case 28: return make_instrument<Rimshot>();
        case 29: return make_instrument<Guiro>();
        case 30: return make_instrument<Guiro>();
        case 31: return make_instrument<Claves>();
        case 32: return make_instrument<Woodblock>();
        case 33: return make_instrument<Claves>();
        case 34: return make_instrument<Triangle>();
        case 35: return make_instrument<ConcertBassDrum>();
        case 36: return make_instrument<Kick>();
        case 37: return make_instrument<Rimshot>();
        case 38: return make_instrument<Snare>();
        case 39: return make_instrument<Tambourine>();
        case 40: return make_instrument<Snare>();
        case 41: return make_instrument<FloorTom>();
        case 42: return make_instrument<ClosedHiHat>();
        case 43: return make_instrument<FloorTom>();
        case 44: return make_instrument<ClosedHiHat>();
        case 45: return make_instrument<Tom>();
        case 46: return make_instrument<OpenHiHat>();
        case 47: return make_instrument<Tom>();
        case 48: return make_instrument<Tom>();
        case 49: return make_instrument<CrashCymbal>();
        case 50: return make_instrument<Tom>();
        case 51: return make_instrument<RideCymbal>();
        case 52: return make_instrument<ChinaCymbal>();
        case 53: return make_instrument<RideBell>();
        case 54: return make_instrument<Tambourine>();
        case 55: return make_instrument<SplashCymbal>();
        case 56: return make_instrument<Cowbell>();
        case 57: return make_instrument<CrashCymbal>();
        case 58: return make_instrument<Guiro>();
        case 59: return make_instrument<RideCymbal>();
        case 60: return make_instrument<Bongo>();
        case 61: return make_instrument<Bongo>();
        case 62: return make_instrument<Conga>();
        case 63: return make_instrument<Conga>();
        case 64: return make_instrument<Conga>();
        case 65: return make_instrument<Timbales>();
        case 66: return make_instrument<Timbales>();
        case 67: return make_instrument<Agogo>();
        case 68: return make_instrument<Agogo>();
        case 69: return make_instrument<Cabasa>();
        case 70: return make_instrument<Maracas>();
        case 71: return make_instrument<Triangle>();
        case 72: return make_instrument<Triangle>();
        case 73: return make_instrument<Guiro>();
        case 74: return make_instrument<Guiro>();
        case 75: return make_instrument<Claves>();
        case 76: return make_instrument<Woodblock>();
        case 77: return make_instrument<Woodblock>();
        case 78: return make_instrument<Cuica>();
        case 79: return make_instrument<Cuica>();
        case 80: return make_instrument<Triangle>();
        case 81: return make_instrument<Triangle>();
        case 82: return make_instrument<Shaker>();
        case 83: return make_instrument<Tambourine>();
        case 84: return make_instrument<BellTree>();
        case 85: return make_instrument<Castanets>();
        case 86: return make_instrument<Surdo>();
        case 87: return make_instrument<Surdo>();
        default: return make_instrument<Woodblock>();
    }
}

inline bool guess_program_from_name(const std::string& name, int& out_program) {
    std::string lower;
    lower.reserve(name.size());
    for (char c : name) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

    static const std::vector<std::pair<const char*, int>> keywords = {
        {"contrabass", 43}, {"double bass", 43}, {"upright bass", 43},
        {"cello", 42}, {"violoncello", 42},
        {"viola", 41},
        {"violin", 40},
        {"string", 48},
        {"harp", 46},
        {"timpani", 47},
        {"trumpet", 56}, {"cornet", 56},
        {"trombone", 57},
        {"tuba", 58},
        {"french horn", 60}, {"horn", 60},
        {"brass", 61},
        {"soprano sax", 64}, {"alto sax", 65}, {"tenor sax", 66}, {"baritone sax", 67}, {"sax", 65},
        {"oboe", 68},
        {"english horn", 69},
        {"bassoon", 70},
        {"clarinet", 71},
        {"piccolo", 72},
        {"flute", 73},
        {"recorder", 74},
        {"pan flute", 75},
        {"ocarina", 79},
        {"choir", 52}, {"vocal", 52}, {"aahs", 52}, {"oohs", 53}, {"voice", 53},
        {"organ", 19},
        {"accordion", 21},
        {"harmonica", 22},
        {"nylon guitar", 24}, {"classical guitar", 24},
        {"acoustic guitar", 25},
        {"electric guitar", 27}, {"guitar", 27},
        {"bass guitar", 33}, {"bass", 33},
        {"piano", 0}, {"keys", 0},
        {"synth lead", 81}, {"lead", 81},
        {"synth pad", 89}, {"pad", 89},
        {"synth", 80},
    };

    for (auto& [kw, prog] : keywords) {
        if (lower.find(kw) != std::string::npos) {
            out_program = prog;
            return true;
        }
    }
    return false;
}

}
}
