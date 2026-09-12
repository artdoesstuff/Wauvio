#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace wauvio {
namespace midi {

class TempoMap {
public:
    void add_tempo_event(uint64_t tick, uint32_t micros_per_quarter) {
        raw_events_.push_back({tick, micros_per_quarter});
    }

    void finalize(uint16_t ticks_per_quarter) {
        tpq_ = ticks_per_quarter > 0 ? ticks_per_quarter : 480;
        smpte_mode_ = false;

        std::stable_sort(raw_events_.begin(), raw_events_.end(),
                          [](const TempoEvent& a, const TempoEvent& b) { return a.tick < b.tick; });

        std::vector<TempoEvent> merged;
        for (auto& e : raw_events_) {
            if (!merged.empty() && merged.back().tick == e.tick) merged.back() = e;
            else merged.push_back(e);
        }
        raw_events_ = std::move(merged);

        if (raw_events_.empty() || raw_events_.front().tick != 0)
            raw_events_.insert(raw_events_.begin(), TempoEvent{0, 500000});

        segments_.clear();
        double sec = 0.0;
        for (size_t i = 0; i < raw_events_.size(); ++i) {
            segments_.push_back(Segment{raw_events_[i].tick, sec, raw_events_[i].micros_per_quarter});
            if (i + 1 < raw_events_.size()) {
                uint64_t dt = raw_events_[i + 1].tick - raw_events_[i].tick;
                sec += static_cast<double>(dt) * raw_events_[i].micros_per_quarter / 1e6 / tpq_;
            }
        }
    }

    /// SMPTE-divided files use a fixed linear tick->time mapping (tempo
    /// meta events do not apply, per the Standard MIDI File spec).
    void finalize_smpte(int fps, int ticks_per_frame) {
        smpte_mode_ = true;
        smpte_seconds_per_tick_ = 1.0 / (static_cast<double>(fps) * static_cast<double>(ticks_per_frame));
    }

    double tick_to_seconds(uint64_t tick) const {
        if (smpte_mode_) return static_cast<double>(tick) * smpte_seconds_per_tick_;
        if (segments_.empty()) return static_cast<double>(tick) * 500000.0 / 1e6 / tpq_;
        size_t lo = 0, hi = segments_.size();
        while (lo < hi) {
            size_t mid = (lo + hi) / 2;
            if (segments_[mid].start_tick <= tick) lo = mid + 1;
            else hi = mid;
        }
        size_t idx = (lo > 0) ? lo - 1 : 0;
        const Segment& s = segments_[idx];
        return s.start_sec + static_cast<double>(tick - s.start_tick) * s.micros_per_quarter / 1e6 / tpq_;
    }

    double bpm_at_start() const {
        return raw_events_.empty() ? 120.0 : 60000000.0 / raw_events_.front().micros_per_quarter;
    }

    uint64_t seconds_to_ticks(double seconds) const {
        if (smpte_mode_) return static_cast<uint64_t>(std::llround(seconds / smpte_seconds_per_tick_));
        if (segments_.empty()) return static_cast<uint64_t>(std::llround(seconds * 1e6 / 500000.0 * tpq_));
        size_t lo = 0, hi = segments_.size();
        while (lo < hi) {
            size_t mid = (lo + hi) / 2;
            if (segments_[mid].start_sec <= seconds) lo = mid + 1;
            else hi = mid;
        }
        size_t idx = (lo > 0) ? lo - 1 : 0;
        const Segment& s = segments_[idx];
        double dt_sec = seconds - s.start_sec;
        double ticks = dt_sec * 1e6 * tpq_ / s.micros_per_quarter;
        return s.start_tick + static_cast<uint64_t>(std::llround(ticks));
    }

    std::vector<std::pair<uint64_t,uint32_t>> tempo_events() const {
        std::vector<std::pair<uint64_t,uint32_t>> out;
        out.reserve(raw_events_.size());
        for (auto& e : raw_events_) out.emplace_back(e.tick, e.micros_per_quarter);
        return out;
    }

    uint16_t ticks_per_quarter() const noexcept { return tpq_; }

private:
    struct TempoEvent { uint64_t tick; uint32_t micros_per_quarter; };
    struct Segment { uint64_t start_tick; double start_sec; uint32_t micros_per_quarter; };

    uint16_t tpq_ = 480;
    bool smpte_mode_ = false;
    double smpte_seconds_per_tick_ = 0.0;
    std::vector<TempoEvent> raw_events_;
    std::vector<Segment> segments_;
};

}
}
