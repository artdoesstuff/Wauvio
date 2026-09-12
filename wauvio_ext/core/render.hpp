#pragma once

#include "core.hpp"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <random>
#include <thread>
#include <vector>

namespace wauvio {

enum class DitherType { None, Triangular };

struct RenderOptions {
    int    sample_rate      = 44100;
    int    channels         = 2;
    int    bit_depth        = 16;
    bool   normalize        = true;
    float  normalize_peak   = 0.95f;
    DitherType dither       = DitherType::None;
    double render_tail_sec  = 0.0;
    int    loop_count       = 1;
    audio::InterpolationQuality interpolation = audio::InterpolationQuality::Linear;
    unsigned worker_threads = 0;
    bool     deterministic_seed_enabled = false;
    uint64_t seed = 0;
};

namespace render_detail {

inline unsigned resolve_worker_count(unsigned requested) {
    if (requested == 1) return 1;
    unsigned hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 2;
    return requested == 0 ? hw : requested;
}

inline void write_u32le(std::vector<unsigned char>& out, uint32_t v) {
    out.push_back(static_cast<unsigned char>(v & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 24) & 0xFF));
}
inline void write_u16le(std::vector<unsigned char>& out, uint16_t v) {
    out.push_back(static_cast<unsigned char>(v & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
}

inline float dither_sample(float s, DitherType dither, std::mt19937& rng) {
    if (dither == DitherType::None) return s;
    std::uniform_real_distribution<float> d(-1.0f, 1.0f);
    return s + (d(rng) + d(rng)) * (1.0f / 65536.0f);
}

inline void write_wav_custom(const StereoBuffer& buf, const std::string& path, int sample_rate,
                              int bit_depth, DitherType dither, uint64_t seed, bool use_seed)
{
    std::mt19937 rng(use_seed ? static_cast<uint32_t>(seed) : std::random_device{}());
    const uint16_t channels = 2;
    const uint32_t n_frames = static_cast<uint32_t>(buf.size());

    uint16_t format_tag = (bit_depth == 32) ? 3 : 1;
    uint16_t bytes_per_sample = static_cast<uint16_t>(bit_depth / 8);
    uint32_t data_size = n_frames * channels * bytes_per_sample;

    std::vector<unsigned char> data;
    data.reserve(data_size);

    for (uint32_t i = 0; i < n_frames; ++i) {
        float l = dither_sample(buf.L[i], dither, rng);
        float r = dither_sample(buf.R[i], dither, rng);
        l = std::max(-1.0f, std::min(1.0f, l));
        r = std::max(-1.0f, std::min(1.0f, r));

        if (bit_depth == 16) {
            int16_t li = static_cast<int16_t>(std::lround(l * 32767.0f));
            int16_t ri = static_cast<int16_t>(std::lround(r * 32767.0f));
            data.push_back(static_cast<unsigned char>(li & 0xFF));
            data.push_back(static_cast<unsigned char>((li >> 8) & 0xFF));
            data.push_back(static_cast<unsigned char>(ri & 0xFF));
            data.push_back(static_cast<unsigned char>((ri >> 8) & 0xFF));
        } else if (bit_depth == 24) {
            int32_t li = static_cast<int32_t>(std::lround(l * 8388607.0f));
            int32_t ri = static_cast<int32_t>(std::lround(r * 8388607.0f));
            data.push_back(static_cast<unsigned char>(li & 0xFF));
            data.push_back(static_cast<unsigned char>((li >> 8) & 0xFF));
            data.push_back(static_cast<unsigned char>((li >> 16) & 0xFF));
            data.push_back(static_cast<unsigned char>(ri & 0xFF));
            data.push_back(static_cast<unsigned char>((ri >> 8) & 0xFF));
            data.push_back(static_cast<unsigned char>((ri >> 16) & 0xFF));
        } else if (bit_depth == 32) {
            unsigned char lb[4], rb[4];
            std::memcpy(lb, &l, 4);
            std::memcpy(rb, &r, 4);
            for (int k = 0; k < 4; ++k) data.push_back(lb[k]);
            for (int k = 0; k < 4; ++k) data.push_back(rb[k]);
        } else {
            throw std::runtime_error("write_wav_custom: unsupported bit depth " + std::to_string(bit_depth));
        }
    }

    std::vector<unsigned char> header;
    header.reserve(44);
    header.insert(header.end(), {'R','I','F','F'});
    write_u32le(header, 36 + static_cast<uint32_t>(data.size()));
    header.insert(header.end(), {'W','A','V','E'});
    header.insert(header.end(), {'f','m','t',' '});
    write_u32le(header, 16);
    write_u16le(header, format_tag);
    write_u16le(header, channels);
    write_u32le(header, static_cast<uint32_t>(sample_rate));
    write_u32le(header, static_cast<uint32_t>(sample_rate) * channels * bytes_per_sample);
    write_u16le(header, static_cast<uint16_t>(channels * bytes_per_sample));
    write_u16le(header, static_cast<uint16_t>(bit_depth));
    header.insert(header.end(), {'d','a','t','a'});
    write_u32le(header, static_cast<uint32_t>(data.size()));

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) throw std::runtime_error("write_wav_custom: cannot open for writing: " + path);
    std::fwrite(header.data(), 1, header.size(), f);
    std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

}

inline void apply_render_options(StereoBuffer& buf, const RenderOptions& opts) {
    if (opts.render_tail_sec > 0.0) {
        size_t tail_samples = static_cast<size_t>(opts.render_tail_sec * opts.sample_rate);
        size_t old_size = buf.size();
        StereoBuffer padded(old_size + tail_samples);
        for (size_t i = 0; i < old_size; ++i) { padded.L[i] = buf.L[i]; padded.R[i] = buf.R[i]; }
        buf = std::move(padded);
    }
    if (opts.loop_count > 1) {
        size_t unit = buf.size();
        StereoBuffer looped(unit * static_cast<size_t>(opts.loop_count));
        for (int rep = 0; rep < opts.loop_count; ++rep)
            for (size_t i = 0; i < unit; ++i) {
                looped.L[static_cast<size_t>(rep) * unit + i] = buf.L[i];
                looped.R[static_cast<size_t>(rep) * unit + i] = buf.R[i];
            }
        buf = std::move(looped);
    }
    if (opts.normalize) normalize(buf, opts.normalize_peak);
    clamp_buffer(buf);
}

inline void render(const StereoBuffer& buf_in, const std::string& path, const RenderOptions& opts = RenderOptions()) {
    StereoBuffer buf = buf_in;
    apply_render_options(buf, opts);
    render_detail::write_wav_custom(buf, path, opts.sample_rate, opts.bit_depth, opts.dither,
                                     opts.seed, opts.deterministic_seed_enabled);
}

namespace audio {

inline StereoBuffer render_to_buffer(const Arrangement& arr, const RenderOptions& opts = RenderOptions()) {
    auto prior_quality = default_interpolation_quality();
    default_interpolation_quality() = opts.interpolation;

    const size_t n_tracks = arr.track_count();
    std::vector<StereoBuffer> rendered(n_tracks);

    unsigned workers = wauvio::render_detail::resolve_worker_count(opts.worker_threads);
    if (workers <= 1 || n_tracks <= 1) {
        for (size_t i = 0; i < n_tracks; ++i)
            rendered[i] = arr.track_at(i).render(opts.sample_rate);
    } else {
        std::atomic<size_t> next{0};
        std::vector<std::thread> pool;
        pool.reserve(workers);
        for (unsigned w = 0; w < workers; ++w) {
            pool.emplace_back([&]() {
                size_t idx;
                while ((idx = next.fetch_add(1)) < n_tracks)
                    rendered[idx] = arr.track_at(idx).render(opts.sample_rate);
            });
        }
        for (auto& th : pool) th.join();
    }

    MasterBus bus(opts.sample_rate);
    for (size_t i = 0; i < n_tracks; ++i)
        bus.schedule(std::move(rendered[i]), arr.start_time_at(i), arr.gain_at(i));
    StereoBuffer out = bus.mix_stereo();

    if (arr.has_master_reverb()) {
        Reverb rL = arr.master_reverb_settings(), rR = arr.master_reverb_settings();
        rL.process(out.L); rR.process(out.R);
    }
    if (arr.has_limiter()) {
        float th = arr.limiter_threshold();
        for (auto& s : out.L) s = distortion::soft_clip(s, 1.0f / std::max(0.05f, th));
        for (auto& s : out.R) s = distortion::soft_clip(s, 1.0f / std::max(0.05f, th));
    }

    default_interpolation_quality() = prior_quality;
    return out;
}

}

inline void render(const audio::Arrangement& arr, const std::string& path, const RenderOptions& opts = RenderOptions()) {
    StereoBuffer buf = audio::render_to_buffer(arr, opts);
    apply_render_options(buf, opts);
    render_detail::write_wav_custom(buf, path, opts.sample_rate, opts.bit_depth, opts.dither,
                                     opts.seed, opts.deterministic_seed_enabled);
}

}
