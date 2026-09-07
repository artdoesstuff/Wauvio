#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace wauvio {
namespace midi {

enum class MidiEventType {
    NoteOff,
    NoteOn,
    PolyAftertouch,
    ControlChange,
    ProgramChange,
    ChannelAftertouch,
    PitchBend,
    Meta,
    SysEx,
    Other
};

struct MidiEvent {
    uint64_t abs_tick = 0;
    MidiEventType type = MidiEventType::Other;
    uint8_t channel = 0;
    uint8_t data1 = 0;
    uint8_t data2 = 0;
    uint8_t meta_type = 0;
    std::vector<uint8_t> meta_data;
};

struct ParsedTrack {
    std::string name;
    std::string instrument_name;
    std::vector<MidiEvent> events;
};

struct ParsedMidiFile {
    uint16_t format = 1;
    uint16_t declared_track_count = 0;
    uint16_t division = 480;
    bool smpte_timing = false;
    std::vector<ParsedTrack> tracks;
};

struct TimeSignatureEvent {
    uint64_t tick = 0;
    int numerator = 4;
    int denominator = 4;
};

struct KeySignatureEvent {
    uint64_t tick = 0;
    int sharps_or_flats = 0;
    bool is_minor = false;
};

}
}
