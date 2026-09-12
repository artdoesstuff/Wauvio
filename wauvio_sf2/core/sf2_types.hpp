#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wauvio {
namespace sf2 {

enum class Generator : uint16_t {
    StartAddrsOffset = 0,
    EndAddrsOffset = 1,
    StartloopAddrsOffset = 2,
    EndloopAddrsOffset = 3,
    StartAddrsCoarseOffset = 4,
    ModLfoToPitch = 5,
    VibLfoToPitch = 6,
    ModEnvToPitch = 7,
    InitialFilterFc = 8,
    InitialFilterQ = 9,
    ModLfoToFilterFc = 10,
    ModEnvToFilterFc = 11,
    EndAddrsCoarseOffset = 12,
    ModLfoToVolume = 13,
    ChorusEffectsSend = 15,
    ReverbEffectsSend = 16,
    Pan = 17,
    DelayModLFO = 21,
    FreqModLFO = 22,
    DelayVibLFO = 23,
    FreqVibLFO = 24,
    DelayModEnv = 25,
    AttackModEnv = 26,
    HoldModEnv = 27,
    DecayModEnv = 28,
    SustainModEnv = 29,
    ReleaseModEnv = 30,
    KeynumToModEnvHold = 31,
    KeynumToModEnvDecay = 32,
    DelayVolEnv = 33,
    AttackVolEnv = 34,
    HoldVolEnv = 35,
    DecayVolEnv = 36,
    SustainVolEnv = 37,
    ReleaseVolEnv = 38,
    KeynumToVolEnvHold = 39,
    KeynumToVolEnvDecay = 40,
    Instrument = 41,
    KeyRange = 43,
    VelRange = 44,
    StartloopAddrsCoarseOffset = 45,
    Keynum = 46,
    Velocity = 47,
    InitialAttenuation = 48,
    EndloopAddrsCoarseOffset = 50,
    CoarseTune = 51,
    FineTune = 52,
    SampleID = 53,
    SampleModes = 54,
    ScaleTuning = 56,
    ExclusiveClass = 57,
    OverridingRootKey = 58,
    EndOper = 60
};

union GenAmount {
    int16_t sh_amount;
    uint16_t w_amount;
    struct { uint8_t lo; uint8_t hi; } ranges;
};

struct GenListEntry {
    Generator oper;
    GenAmount amount;
};

struct ModListEntry {
    uint16_t src_oper = 0;
    uint16_t dest_oper = 0;
    int16_t amount = 0;
    uint16_t amt_src_oper = 0;
    uint16_t trans_oper = 0;
};

struct BagEntry {
    uint16_t gen_ndx = 0;
    uint16_t mod_ndx = 0;
};

struct PresetHeader {
    std::string name;
    uint16_t preset = 0;
    uint16_t bank = 0;
    uint16_t bag_ndx = 0;
};

struct InstHeader {
    std::string name;
    uint16_t bag_ndx = 0;
};

struct SampleHeader {
    std::string name;
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t loop_start = 0;
    uint32_t loop_end = 0;
    uint32_t sample_rate = 44100;
    uint8_t original_pitch = 60;
    int8_t pitch_correction = 0;
    uint16_t sample_link = 0;
    uint16_t sample_type = 1;
};

struct Hydra {
    std::vector<PresetHeader> phdr;
    std::vector<BagEntry> pbag;
    std::vector<ModListEntry> pmod;
    std::vector<GenListEntry> pgen;
    std::vector<InstHeader> inst;
    std::vector<BagEntry> ibag;
    std::vector<ModListEntry> imod;
    std::vector<GenListEntry> igen;
    std::vector<SampleHeader> shdr;
};

}
}
