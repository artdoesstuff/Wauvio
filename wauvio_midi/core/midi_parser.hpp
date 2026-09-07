#pragma once

#include "byte_reader.hpp"
#include "midi_types.hpp"

#include <cstring>
#include <fstream>
#include <vector>
#include <string>

namespace wauvio {
namespace midi {

inline std::vector<unsigned char> read_whole_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw MidiParseError("Cannot open MIDI file: " + path);

    f.seekg(0, std::ios::end);
    std::streamoff sz = f.tellg();
    if (sz < 0) throw MidiParseError("Cannot determine size of MIDI file: " + path);
    f.seekg(0, std::ios::beg);

    std::vector<unsigned char> buf(static_cast<size_t>(sz));
    if (sz > 0 && !f.read(reinterpret_cast<char*>(buf.data()), sz))
        throw MidiParseError("Failed to read MIDI file (possibly truncated): " + path);
    return buf;
}

namespace detail {

inline bool magic_is(const unsigned char* p, const char* expect) {
    return std::memcmp(p, expect, 4) == 0;
}

inline int channel_event_data_bytes(uint8_t status_hi) {
    switch (status_hi) {
        case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: return 2;
        case 0xC0: case 0xD0: return 1;
        default: return -1;
    }
}

}

inline ParsedMidiFile parse_midi_bytes(const std::vector<unsigned char>& bytes, const std::string& source_label) {
    ByteReader r(bytes.data(), bytes.size());

    if (r.remaining() < 8)
        throw MidiParseError("File too small to contain a valid MIDI header: " + source_label);

    unsigned char magic[4];
    for (int i = 0; i < 4; ++i) magic[i] = r.u8();
    if (!detail::magic_is(magic, "MThd"))
        throw MidiParseError("Invalid MIDI header: missing 'MThd' chunk magic in " + source_label);

    uint32_t hdr_len = r.u32be();
    if (hdr_len < 6)
        throw MidiParseError("Invalid MIDI header chunk length (" + std::to_string(hdr_len) +
                              "), expected at least 6, in " + source_label);
    if (r.remaining() < hdr_len)
        throw MidiParseError("Truncated MIDI header chunk in " + source_label);
    size_t header_end = r.position() + hdr_len;

    ParsedMidiFile out;
    out.format = r.u16be();
    if (out.format > 2)
        throw MidiParseError("Unsupported MIDI format " + std::to_string(out.format) +
                              " (expected 0, 1, or 2) in " + source_label);
    uint16_t ntrks = r.u16be();
    out.declared_track_count = ntrks;
    uint16_t division = r.u16be();
    if (division & 0x8000) {
        out.smpte_timing = true;
        out.division = 480;
    } else {
        out.division = division == 0 ? 480 : division;
    }

    if (r.position() < header_end) r.seek(header_end);

    out.tracks.reserve(ntrks);
    for (uint16_t t = 0; t < ntrks; ++t) {
        if (r.eof()) break;
        if (r.remaining() < 8)
            throw MidiParseError("Truncated file: missing chunk header for track " + std::to_string(t) +
                                  " in " + source_label);

        unsigned char tmagic[4];
        for (int i = 0; i < 4; ++i) tmagic[i] = r.u8();
        uint32_t tlen = r.u32be();
        if (r.remaining() < tlen)
            throw MidiParseError("Truncated track chunk: declared length exceeds remaining file data "
                                  "for track " + std::to_string(t) + " in " + source_label);
        size_t track_end = r.position() + tlen;

        if (!detail::magic_is(tmagic, "MTrk")) {
            r.seek(track_end);
            continue;
        }

        ParsedTrack track;
        uint64_t abs_tick = 0;
        uint8_t running_status = 0;
        bool have_running_status = false;

        while (r.position() < track_end) {
            uint32_t delta = r.read_vlq();
            abs_tick += delta;

            if (r.position() >= track_end)
                throw MidiParseError("Unexpected end of track data (dangling delta-time) in track " +
                                      std::to_string(t) + " of " + source_label);
            uint8_t first = r.u8();

            uint8_t status;
            bool first_is_status = (first & 0x80) != 0;
            if (first_is_status) {
                status = first;
            } else {
                if (!have_running_status)
                    throw MidiParseError("Invalid running status: data byte with no prior status byte "
                                          "in track " + std::to_string(t) + " of " + source_label);
                status = running_status;
            }

            if (status == 0xFF) {
                have_running_status = false;
                if (r.position() >= track_end)
                    throw MidiParseError("Truncated meta event in track " + std::to_string(t) + " of " + source_label);
                uint8_t meta_type = r.u8();
                uint32_t len = r.read_vlq();
                if (r.remaining() < len || r.position() + len > track_end)
                    throw MidiParseError("Meta event length exceeds remaining track data in track " +
                                          std::to_string(t) + " of " + source_label);
                std::vector<uint8_t> data(len);
                for (uint32_t i = 0; i < len; ++i) data[i] = r.u8();

                MidiEvent ev;
                ev.abs_tick = abs_tick;
                ev.type = MidiEventType::Meta;
                ev.meta_type = meta_type;
                ev.meta_data = data;

                if (meta_type == 0x03) track.name.assign(data.begin(), data.end());
                if (meta_type == 0x04) track.instrument_name.assign(data.begin(), data.end());

                track.events.push_back(std::move(ev));
                continue;
            }

            if (status == 0xF0 || status == 0xF7) {
                have_running_status = false;
                uint32_t len = r.read_vlq();
                if (r.remaining() < len || r.position() + len > track_end)
                    throw MidiParseError("SysEx event length exceeds remaining track data in track " +
                                          std::to_string(t) + " of " + source_label);
                r.skip(len);
                continue;
            }

            if (status >= 0xF1 && status <= 0xFE) {

                if (status == 0xF2) { if (r.position() + 2 <= track_end) r.skip(2); }
                else if (status == 0xF1 || status == 0xF3) { if (r.position() + 1 <= track_end) r.skip(1); }
                if (status <= 0xF7) have_running_status = false;
                continue;
            }

            uint8_t status_hi = static_cast<uint8_t>(status & 0xF0);
            uint8_t channel = static_cast<uint8_t>(status & 0x0F);
            int n_data = detail::channel_event_data_bytes(status_hi);
            if (n_data < 0)
                throw MidiParseError("Malformed or unsupported status byte 0x" +
                                      std::to_string(static_cast<int>(status)) + " in track " +
                                      std::to_string(t) + " of " + source_label);

            running_status = status;
            have_running_status = true;

            uint8_t d1 = first_is_status ? r.u8() : first;
            uint8_t d2 = 0;
            if (n_data == 2) d2 = r.u8();

            MidiEvent ev;
            ev.abs_tick = abs_tick;
            ev.channel = channel;
            ev.data1 = d1;
            ev.data2 = d2;

            switch (status_hi) {
                case 0x80: ev.type = MidiEventType::NoteOff; break;
                case 0x90: ev.type = (d2 == 0) ? MidiEventType::NoteOff : MidiEventType::NoteOn; break;
                case 0xA0: ev.type = MidiEventType::PolyAftertouch; break;
                case 0xB0: ev.type = MidiEventType::ControlChange; break;
                case 0xC0: ev.type = MidiEventType::ProgramChange; break;
                case 0xD0: ev.type = MidiEventType::ChannelAftertouch; break;
                case 0xE0: ev.type = MidiEventType::PitchBend; break;
                default:   ev.type = MidiEventType::Other; break;
            }
            track.events.push_back(std::move(ev));
        }

        r.seek(track_end);
        out.tracks.push_back(std::move(track));
    }

    return out;
}

inline ParsedMidiFile parse_midi_file(const std::string& path) {
    auto bytes = read_whole_file(path);
    return parse_midi_bytes(bytes, path);
}

}
}
