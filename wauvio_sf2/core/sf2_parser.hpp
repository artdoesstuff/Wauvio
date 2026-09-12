#pragma once

#include "riff_reader.hpp"
#include "sf2_types.hpp"

#include <memory>

namespace wauvio {
namespace sf2 {

struct RawSoundFont {
    std::string name;
    std::shared_ptr<std::vector<float>> sample_data;
    uint32_t sample_data_frames = 0;
    Hydra hydra;
};

namespace parser_detail {

inline void parse_info(ByteReader& r, size_t end, RawSoundFont& out) {
    while (r.position() < end) {
        std::string id = r.fourcc();
        uint32_t size = r.u32le();
        size_t chunk_end = r.position() + size;
        if (chunk_end > end) throw Sf2ParseError("INFO subchunk '" + id + "' overruns LIST bounds");
        if (id == "INAM") out.name = r.fixed_string(size);
        else r.skip(size);
        r.seek(chunk_end + (size & 1));
    }
}

inline void parse_sdta(ByteReader& r, size_t end, RawSoundFont& out) {
    while (r.position() < end) {
        std::string id = r.fourcc();
        uint32_t size = r.u32le();
        size_t chunk_end = r.position() + size;
        if (chunk_end > end) throw Sf2ParseError("sdta subchunk '" + id + "' overruns LIST bounds");

        if (id == "smpl") {
            uint32_t n_samples = size / 2;
            auto data = std::make_shared<std::vector<float>>(n_samples);
            const unsigned char* base = r.ptr();
            for (uint32_t i = 0; i < n_samples; ++i) {
                int16_t v = static_cast<int16_t>(base[i * 2] | (base[i * 2 + 1] << 8));
                (*data)[i] = v / 32768.0f;
            }
            out.sample_data = data;
            out.sample_data_frames = n_samples;
        }
        r.seek(chunk_end + (size & 1));
    }
}

inline void parse_pdta(ByteReader& r, size_t end, RawSoundFont& out) {
    Hydra& h = out.hydra;
    while (r.position() < end) {
        std::string id = r.fourcc();
        uint32_t size = r.u32le();
        size_t chunk_end = r.position() + size;
        if (chunk_end > end) throw Sf2ParseError("pdta subchunk '" + id + "' overruns LIST bounds");

        if (id == "phdr") {
            uint32_t n = size / 38;
            h.phdr.reserve(n);
            for (uint32_t i = 0; i < n; ++i) {
                PresetHeader ph;
                ph.name = r.fixed_string(20);
                ph.preset = r.u16le();
                ph.bank = r.u16le();
                ph.bag_ndx = r.u16le();
                r.skip(12);
                h.phdr.push_back(std::move(ph));
            }
        } else if (id == "pbag" || id == "ibag") {
            uint32_t n = size / 4;
            auto& vec = (id == "pbag") ? h.pbag : h.ibag;
            vec.reserve(n);
            for (uint32_t i = 0; i < n; ++i) {
                BagEntry b;
                b.gen_ndx = r.u16le();
                b.mod_ndx = r.u16le();
                vec.push_back(b);
            }
        } else if (id == "pmod" || id == "imod") {
            uint32_t n = size / 10;
            auto& vec = (id == "pmod") ? h.pmod : h.imod;
            vec.reserve(n);
            for (uint32_t i = 0; i < n; ++i) {
                ModListEntry m;
                m.src_oper = r.u16le();
                m.dest_oper = r.u16le();
                m.amount = r.s16le();
                m.amt_src_oper = r.u16le();
                m.trans_oper = r.u16le();
                vec.push_back(m);
            }
        } else if (id == "pgen" || id == "igen") {
            uint32_t n = size / 4;
            auto& vec = (id == "pgen") ? h.pgen : h.igen;
            vec.reserve(n);
            for (uint32_t i = 0; i < n; ++i) {
                GenListEntry g;
                g.oper = static_cast<Generator>(r.u16le());
                g.amount.w_amount = r.u16le();
                vec.push_back(g);
            }
        } else if (id == "inst") {
            uint32_t n = size / 22;
            h.inst.reserve(n);
            for (uint32_t i = 0; i < n; ++i) {
                InstHeader ih;
                ih.name = r.fixed_string(20);
                ih.bag_ndx = r.u16le();
                h.inst.push_back(std::move(ih));
            }
        } else if (id == "shdr") {
            uint32_t n = size / 46;
            h.shdr.reserve(n);
            for (uint32_t i = 0; i < n; ++i) {
                SampleHeader sh;
                sh.name = r.fixed_string(20);
                sh.start = r.u32le();
                sh.end = r.u32le();
                sh.loop_start = r.u32le();
                sh.loop_end = r.u32le();
                sh.sample_rate = r.u32le();
                sh.original_pitch = r.u8();
                sh.pitch_correction = r.s8();
                sh.sample_link = r.u16le();
                sh.sample_type = r.u16le();
                h.shdr.push_back(std::move(sh));
            }
        }
        r.seek(chunk_end + (size & 1));
    }
}

}

inline RawSoundFont parse_sf2_file(const std::string& path) {
    auto bytes = read_whole_file(path);
    if (bytes.size() < 12) throw Sf2ParseError("File too small to be a valid SoundFont: " + path);

    ByteReader r(bytes.data(), bytes.size());
    std::string riff_magic = r.fourcc();
    if (riff_magic != "RIFF") throw Sf2ParseError("Not a RIFF file (missing 'RIFF' magic): " + path);
    uint32_t riff_size = r.u32le();
    if (r.position() + riff_size > bytes.size())
        throw Sf2ParseError("RIFF chunk size exceeds file size (truncated SoundFont): " + path);
    size_t riff_end = r.position() + riff_size;

    std::string form = r.fourcc();
    if (form != "sfbk") throw Sf2ParseError("Not a SoundFont 2 file (expected 'sfbk', found '" + form + "'): " + path);

    RawSoundFont out;
    bool saw_info = false, saw_sdta = false, saw_pdta = false;

    while (r.position() < riff_end) {
        std::string id = r.fourcc();
        uint32_t size = r.u32le();
        size_t chunk_end = r.position() + size;
        if (chunk_end > riff_end) throw Sf2ParseError("Top-level chunk '" + id + "' overruns file bounds: " + path);

        if (id == "LIST") {
            std::string list_type = r.fourcc();
            size_t list_body_end = chunk_end;
            if (list_type == "INFO") { parser_detail::parse_info(r, list_body_end, out); saw_info = true; }
            else if (list_type == "sdta") { parser_detail::parse_sdta(r, list_body_end, out); saw_sdta = true; }
            else if (list_type == "pdta") { parser_detail::parse_pdta(r, list_body_end, out); saw_pdta = true; }
        }
        r.seek(chunk_end + (size & 1));
    }

    if (!saw_info) throw Sf2ParseError("SoundFont missing required INFO chunk: " + path);
    if (!saw_sdta) throw Sf2ParseError("SoundFont missing required sdta (sample data) chunk: " + path);
    if (!saw_pdta) throw Sf2ParseError("SoundFont missing required pdta (preset data) chunk: " + path);
    if (out.hydra.phdr.size() < 2) throw Sf2ParseError("SoundFont has no usable presets (phdr too short): " + path);
    if (!out.sample_data) throw Sf2ParseError("SoundFont has no sample audio data: " + path);

    return out;
}

}
}
