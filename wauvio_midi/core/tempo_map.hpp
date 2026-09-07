#pragma once

#include <algorithm>
#include <cstdint>
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

    double tick_to_seconds(uint64_t tick) const {
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

    uint16_t ticks_per_quarter() const noexcept { return tpq_; }

private:
    struct TempoEvent { uint64_t tick; uint32_t micros_per_quarter; };
    struct Segment { uint64_t start_tick; double start_sec; uint32_t micros_per_quarter; };

    uint16_t tpq_ = 480;
    std::vector<TempoEvent> raw_events_;
    std::vector<Segment> segments_;
};

}
}
