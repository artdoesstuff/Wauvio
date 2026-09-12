#pragma once

#include "midi_music.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <thread>

namespace wauvio {
namespace track {

namespace detail {

inline std::string sanitize_filename(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') out.push_back(c);
        else if (c == ' ') out.push_back('_');
    }
    if (out.empty()) out = "part";
    return out;
}

} // namespace detail

/// Renders every part of a MidiMusic, optionally in parallel, and returns
/// the deterministic full mix. Parts are always combined in a fixed order
/// regardless of which worker thread finishes first, so this produces
/// bit-identical output to MidiMusic::render() at worker_threads=1.
inline StereoBuffer render_to_buffer(const MidiMusic& music, const RenderOptions& opts = RenderOptions()) {
    auto prior_quality = audio::default_interpolation_quality();
    audio::default_interpolation_quality() = opts.interpolation;

    int sample_rate = opts.sample_rate > 0 ? opts.sample_rate : global_config().sample_rate;
    size_t total = music.total_samples(sample_rate);
    const size_t n_parts = music.part_count();

    std::vector<StereoBuffer> rendered(n_parts);
    unsigned workers = wauvio::render_detail::resolve_worker_count(opts.worker_threads);

    if (workers <= 1 || n_parts <= 1) {
        for (size_t i = 0; i < n_parts; ++i)
            rendered[i] = music.render_part(i, sample_rate, total);
    } else {
        std::atomic<size_t> next{0};
        std::vector<std::thread> pool;
        pool.reserve(workers);
        for (unsigned w = 0; w < workers; ++w) {
            pool.emplace_back([&]() {
                size_t idx;
                while ((idx = next.fetch_add(1)) < n_parts)
                    rendered[idx] = music.render_part(idx, sample_rate, total);
            });
        }
        for (auto& th : pool) th.join();
    }

    StereoBuffer mix(total, 0.0f);
    for (size_t i = 0; i < n_parts; ++i)
        for (size_t s = 0; s < rendered[i].size(); ++s) {
            mix.L[s] += rendered[i].L[s];
            mix.R[s] += rendered[i].R[s];
        }

    audio::default_interpolation_quality() = prior_quality;
    return mix;
}

/// Renders each part to its own WAV file in `output_dir`, using the same
/// instrument assignment, controllers, and timing as the normal mix -- so
/// each stem sounds exactly like that part's contribution to render().
/// Per-stem files are NOT independently peak-normalized against each other
/// (that would destroy the relative balance between stems); only the
/// requested bit depth/dither/tail are applied per file.
inline std::vector<std::string> render_stems(const MidiMusic& music, const std::string& output_dir,
                                              const RenderOptions& opts = RenderOptions())
{
    std::filesystem::create_directories(output_dir);
    auto prior_quality = audio::default_interpolation_quality();
    audio::default_interpolation_quality() = opts.interpolation;

    int sample_rate = opts.sample_rate > 0 ? opts.sample_rate : global_config().sample_rate;
    size_t total = music.total_samples(sample_rate);
    const size_t n_parts = music.part_count();

    RenderOptions stem_opts = opts;
    stem_opts.normalize = false;

    std::vector<std::string> written_paths(n_parts);
    unsigned workers = wauvio::render_detail::resolve_worker_count(opts.worker_threads);

    auto render_and_write = [&](size_t i) {
        StereoBuffer buf = music.render_part(i, sample_rate, total);
        apply_render_options(buf, stem_opts);
        const MidiPart& p = music.part(i);
        std::string base = detail::sanitize_filename(p.name.empty() ? ("part" + std::to_string(i)) : p.name);
        std::string filename = std::to_string(i) + "_" + base + ".wav";
        std::string full_path = (std::filesystem::path(output_dir) / filename).string();
        render_detail::write_wav_custom(buf, full_path, sample_rate, opts.bit_depth, opts.dither,
                                         opts.seed, opts.deterministic_seed_enabled);
        written_paths[i] = full_path;
    };

    if (workers <= 1 || n_parts <= 1) {
        for (size_t i = 0; i < n_parts; ++i) render_and_write(i);
    } else {
        std::atomic<size_t> next{0};
        std::vector<std::thread> pool;
        pool.reserve(workers);
        for (unsigned w = 0; w < workers; ++w) {
            pool.emplace_back([&]() {
                size_t idx;
                while ((idx = next.fetch_add(1)) < n_parts) render_and_write(idx);
            });
        }
        for (auto& th : pool) th.join();
    }

    audio::default_interpolation_quality() = prior_quality;
    return written_paths;
}

} // namespace track

inline void render(const track::MidiMusic& music, const std::string& path, const RenderOptions& opts = RenderOptions()) {
    StereoBuffer buf = track::render_to_buffer(music, opts);
    apply_render_options(buf, opts);
    render_detail::write_wav_custom(buf, path, opts.sample_rate, opts.bit_depth, opts.dither,
                                     opts.seed, opts.deterministic_seed_enabled);
}

} // namespace wauvio
