#pragma once

#include "core/sf2_parser.hpp"
#include "../wauvio_ext.hpp"

#include <algorithm>
#include <cmath>
#include <map>

namespace wauvio {
namespace sf2 {

namespace instr_detail {

inline std::vector<GenListEntry> bag_slice(const std::vector<BagEntry>& bag,
                                            const std::vector<GenListEntry>& gen_pool, size_t bag_idx)
{
    if (bag_idx >= bag.size()) return {};
    size_t g0 = bag[bag_idx].gen_ndx;
    size_t g1 = (bag_idx + 1 < bag.size()) ? bag[bag_idx + 1].gen_ndx : gen_pool.size();
    if (g1 > gen_pool.size()) g1 = gen_pool.size();
    if (g0 > g1) g0 = g1;
    return std::vector<GenListEntry>(gen_pool.begin() + static_cast<long>(g0), gen_pool.begin() + static_cast<long>(g1));
}

inline bool has_gen(const std::vector<GenListEntry>& gens, Generator g) {
    for (auto& e : gens) if (e.oper == g) return true;
    return false;
}

inline int gen_or(const std::vector<GenListEntry>& local, const std::vector<GenListEntry>& global_zone,
                   bool has_global, Generator g, int def)
{
    for (auto& e : local) if (e.oper == g) return e.amount.sh_amount;
    if (has_global) for (auto& e : global_zone) if (e.oper == g) return e.amount.sh_amount;
    return def;
}

inline void key_vel_range(const std::vector<GenListEntry>& local, const std::vector<GenListEntry>& global_zone,
                           bool has_global, int& key_lo, int& key_hi, int& vel_lo, int& vel_hi)
{
    key_lo = 0; key_hi = 127; vel_lo = 0; vel_hi = 127;
    bool local_key = false, local_vel = false;
    for (auto& e : local) {
        if (e.oper == Generator::KeyRange) { key_lo = e.amount.ranges.lo; key_hi = e.amount.ranges.hi; local_key = true; }
        if (e.oper == Generator::VelRange) { vel_lo = e.amount.ranges.lo; vel_hi = e.amount.ranges.hi; local_vel = true; }
    }
    if (has_global) {
        if (!local_key) for (auto& e : global_zone) if (e.oper == Generator::KeyRange) { key_lo = e.amount.ranges.lo; key_hi = e.amount.ranges.hi; }
        if (!local_vel) for (auto& e : global_zone) if (e.oper == Generator::VelRange) { vel_lo = e.amount.ranges.lo; vel_hi = e.amount.ranges.hi; }
    }
}

inline double timecents_to_seconds(int tc) {
    if (tc <= -32768) return 0.0;
    double s = std::pow(2.0, tc / 1200.0);
    return std::max(0.0005, std::min(60.0, s));
}

}

class SoundFont : public audio::IBankProvider {
public:
    explicit SoundFont(const std::string& path) {
        raw_ = parse_sf2_file(path);
        shared_buffer_ = build_shared_buffer();
        build_preset_index();
    }

    const std::string& name() const noexcept { return raw_.name; }

    struct PresetInfo { std::string name; int bank; int program; };
    std::vector<PresetInfo> list_presets() const {
        std::vector<PresetInfo> out;
        for (auto& p : raw_.hydra.phdr) {
            if (&p == &raw_.hydra.phdr.back()) continue;
            out.push_back({p.name, p.bank, p.preset});
        }
        return out;
    }

    bool has_preset(int bank_msb, int /*bank_lsb*/, int program) const override {
        return find_preset_index(bank_msb, program) >= 0;
    }

    audio::InstrumentPtr create_instrument(int bank_msb, int /*bank_lsb*/, int program) const override {
        int idx = find_preset_index(bank_msb, program);
        if (idx < 0) return nullptr;
        return build_instrument_for_preset(static_cast<size_t>(idx));
    }

    bool has_percussion_preset(int /*bank_msb*/, int /*bank_lsb*/, int /*gm_note*/) const override {
        return find_percussion_preset_index() >= 0;
    }

    audio::InstrumentPtr create_percussion_instrument(int /*bank_msb*/, int /*bank_lsb*/, int /*gm_note*/) const override {
        int idx = find_percussion_preset_index();
        if (idx < 0) return nullptr;
        if (cached_percussion_preset_idx_ == idx && cached_percussion_kit_) return cached_percussion_kit_;
        cached_percussion_kit_ = build_instrument_for_preset(static_cast<size_t>(idx));
        cached_percussion_preset_idx_ = idx;
        return cached_percussion_kit_;
    }

    audio::InstrumentPtr create_instrument_by_name(const std::string& preset_name) const {
        for (size_t i = 0; i + 1 < raw_.hydra.phdr.size(); ++i)
            if (raw_.hydra.phdr[i].name == preset_name) return build_instrument_for_preset(i);
        return nullptr;
    }

private:
    RawSoundFont raw_;
    std::shared_ptr<const StereoBuffer> shared_buffer_;
    std::map<std::pair<int,int>, size_t> preset_index_;
    mutable audio::InstrumentPtr cached_percussion_kit_;
    mutable int cached_percussion_preset_idx_ = -1;

    std::shared_ptr<const StereoBuffer> build_shared_buffer() const {
        auto sb = std::make_shared<StereoBuffer>(raw_.sample_data_frames);
        for (uint32_t i = 0; i < raw_.sample_data_frames; ++i) {
            float v = (*raw_.sample_data)[i];
            sb->L[i] = v;
            sb->R[i] = v;
        }
        return sb;
    }

    void build_preset_index() {
        for (size_t i = 0; i + 1 < raw_.hydra.phdr.size(); ++i)
            preset_index_[{raw_.hydra.phdr[i].bank, raw_.hydra.phdr[i].preset}] = i;
    }

    int find_preset_index(int bank, int program) const {
        auto it = preset_index_.find({bank, program});
        if (it != preset_index_.end()) return static_cast<int>(it->second);
        return -1;
    }

    int find_percussion_preset_index() const {
        for (int bank : {128, 120}) {
            for (auto& kv : preset_index_) if (kv.first.first == bank) return static_cast<int>(kv.second);
        }
        return -1;
    }

    audio::InstrumentPtr build_instrument_for_preset(size_t preset_idx) const {
        using namespace instr_detail;
        const Hydra& h = raw_.hydra;
        const PresetHeader& preset = h.phdr[preset_idx];

        auto instr = std::make_shared<audio::SampledInstrument>();
        instr->name = preset.name;
        instr->articulations = { audio::Articulation::Sustain };

        size_t pbag_begin = preset.bag_ndx;
        size_t pbag_end = (preset_idx + 1 < h.phdr.size()) ? h.phdr[preset_idx + 1].bag_ndx : h.pbag.size();

        std::vector<GenListEntry> preset_global;
        bool has_preset_global = false;

        for (size_t pz = pbag_begin; pz < pbag_end; ++pz) {
            auto pgens = bag_slice(h.pbag, h.pgen, pz);
            bool links_instrument = !pgens.empty() && pgens.back().oper == Generator::Instrument;

            if (!links_instrument) {
                if (pz == pbag_begin) { preset_global = pgens; has_preset_global = true; }
                continue;
            }

            int inst_idx = pgens.back().amount.w_amount;
            if (inst_idx < 0 || static_cast<size_t>(inst_idx) >= h.inst.size()) continue;

            int p_key_lo, p_key_hi, p_vel_lo, p_vel_hi;
            key_vel_range(pgens, preset_global, has_preset_global, p_key_lo, p_key_hi, p_vel_lo, p_vel_hi);

            auto p_additive = [&](Generator g) -> int {
                return gen_or(pgens, preset_global, has_preset_global, g, 0);
            };

            const InstHeader& inst = h.inst[static_cast<size_t>(inst_idx)];
            size_t ibag_begin = inst.bag_ndx;
            size_t ibag_end = (static_cast<size_t>(inst_idx) + 1 < h.inst.size())
                                   ? h.inst[static_cast<size_t>(inst_idx) + 1].bag_ndx : h.ibag.size();

            std::vector<GenListEntry> inst_global;
            bool has_inst_global = false;

            for (size_t iz = ibag_begin; iz < ibag_end; ++iz) {
                auto igens = bag_slice(h.ibag, h.igen, iz);
                bool links_sample = !igens.empty() && igens.back().oper == Generator::SampleID;

                if (!links_sample) {
                    if (iz == ibag_begin) { inst_global = igens; has_inst_global = true; }
                    continue;
                }

                int sample_idx = igens.back().amount.w_amount;
                if (sample_idx < 0 || static_cast<size_t>(sample_idx) >= h.shdr.size()) continue;
                const SampleHeader& sh = h.shdr[static_cast<size_t>(sample_idx)];

                auto i_val = [&](Generator g, int def) -> int {
                    return gen_or(igens, inst_global, has_inst_global, g, def);
                };

                int i_key_lo, i_key_hi, i_vel_lo, i_vel_hi;
                key_vel_range(igens, inst_global, has_inst_global, i_key_lo, i_key_hi, i_vel_lo, i_vel_hi);

                int eff_key_lo = std::max(p_key_lo, i_key_lo), eff_key_hi = std::min(p_key_hi, i_key_hi);
                int eff_vel_lo = std::max(p_vel_lo, i_vel_lo), eff_vel_hi = std::min(p_vel_hi, i_vel_hi);
                if (eff_key_lo > eff_key_hi || eff_vel_lo > eff_vel_hi) continue;
                if (sh.start >= sh.end || sh.end > raw_.sample_data_frames) continue;

                audio::SampleZone zone;
                zone.shared_audio = shared_buffer_;
                zone.sample_rate = static_cast<int>(sh.sample_rate);
                zone.low_note = eff_key_lo; zone.high_note = eff_key_hi;
                zone.low_vel = eff_vel_lo; zone.high_vel = eff_vel_hi;
                zone.articulation = audio::Articulation::Sustain;

                int overriding_root = i_val(Generator::OverridingRootKey, -1);
                zone.root_note = (overriding_root >= 0 && overriding_root <= 127) ? overriding_root : sh.original_pitch;

                zone.coarse_tune_semitones = i_val(Generator::CoarseTune, 0) + p_additive(Generator::CoarseTune);
                zone.fine_tune_cents = i_val(Generator::FineTune, 0) + p_additive(Generator::FineTune) + sh.pitch_correction;

                int scale = i_val(Generator::ScaleTuning, 100);
                zone.scale_tuning = std::max(0, scale) / 100.0;

                int pan_raw = i_val(Generator::Pan, 0) + p_additive(Generator::Pan);
                zone.pan = std::max(-1.0, std::min(1.0, pan_raw / 500.0));

                int atten_cb = i_val(Generator::InitialAttenuation, 0) + p_additive(Generator::InitialAttenuation);
                zone.attenuation_db = -std::max(0, atten_cb) / 10.0;

                int filter_fc_cents = i_val(Generator::InitialFilterFc, 13500) + p_additive(Generator::InitialFilterFc);
                double fc = 8.176 * std::pow(2.0, filter_fc_cents / 1200.0);
                zone.filter_cutoff_hz = std::max(20.0, std::min(20000.0, fc));

                int filter_q_cb = i_val(Generator::InitialFilterQ, 0) + p_additive(Generator::InitialFilterQ);
                zone.filter_q = 0.7 + std::max(0, filter_q_cb) / 100.0;

                long start_off = i_val(Generator::StartAddrsOffset, 0) +
                                  static_cast<long>(i_val(Generator::StartAddrsCoarseOffset, 0)) * 32768L;
                long end_off = i_val(Generator::EndAddrsOffset, 0) +
                                static_cast<long>(i_val(Generator::EndAddrsCoarseOffset, 0)) * 32768L;
                long loop_start_off = i_val(Generator::StartloopAddrsOffset, 0) +
                                        static_cast<long>(i_val(Generator::StartloopAddrsCoarseOffset, 0)) * 32768L;
                long loop_end_off = i_val(Generator::EndloopAddrsOffset, 0) +
                                      static_cast<long>(i_val(Generator::EndloopAddrsCoarseOffset, 0)) * 32768L;

                auto clamp_idx = [&](long v) -> size_t {
                    long c = std::max(0L, std::min(static_cast<long>(raw_.sample_data_frames), v));
                    return static_cast<size_t>(c);
                };
                zone.start = clamp_idx(static_cast<long>(sh.start) + start_off);
                zone.end = clamp_idx(static_cast<long>(sh.end) + end_off);
                zone.loop_start = clamp_idx(static_cast<long>(sh.loop_start) + loop_start_off);
                zone.loop_end = clamp_idx(static_cast<long>(sh.loop_end) + loop_end_off);
                if (zone.end <= zone.start) continue;
                if (zone.loop_end <= zone.loop_start) { zone.loop_start = zone.start; zone.loop_end = zone.end; }

                int sample_modes = i_val(Generator::SampleModes, 0);
                zone.loop_mode = (sample_modes == 1 || sample_modes == 3) ? audio::LoopMode::Forward : audio::LoopMode::None;
                zone.looping = (zone.loop_mode != audio::LoopMode::None);
                zone.loop_crossfade_samples = zone.looping ? 64 : 0;

                zone.exclusive_class = i_val(Generator::ExclusiveClass, 0);

                double delay_vol = timecents_to_seconds(i_val(Generator::DelayVolEnv, -12000) + p_additive(Generator::DelayVolEnv));
                double attack_vol = timecents_to_seconds(i_val(Generator::AttackVolEnv, -12000) + p_additive(Generator::AttackVolEnv));
                double hold_vol = timecents_to_seconds(i_val(Generator::HoldVolEnv, -12000) + p_additive(Generator::HoldVolEnv));
                double decay_vol = timecents_to_seconds(i_val(Generator::DecayVolEnv, -12000) + p_additive(Generator::DecayVolEnv));
                double release_vol = timecents_to_seconds(i_val(Generator::ReleaseVolEnv, -12000) + p_additive(Generator::ReleaseVolEnv));
                int sustain_cb = i_val(Generator::SustainVolEnv, 0) + p_additive(Generator::SustainVolEnv);
                double sustain_lvl = std::pow(10.0, -std::max(0, sustain_cb) / 200.0);

                zone.envelope = DAHDSR(delay_vol, attack_vol, hold_vol, decay_vol, sustain_lvl,
                                       std::max(0.005, release_vol));

                instr->samples.add_zone(std::move(zone));
            }
        }

        return instr;
    }
};

}
}
