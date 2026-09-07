#pragma once

#include "gm_program_map.hpp"

#include <functional>
#include <unordered_map>

namespace wauvio {
namespace midi {

using InstrumentFactory = std::function<InstrumentPtr()>;

class InstrumentResolver {
public:
    void override_channel(int channel, InstrumentFactory factory) {
        channel_overrides_[channel] = std::move(factory);
    }

    void override_program(int program, InstrumentFactory factory) {
        program_overrides_[program] = std::move(factory);
    }

    void override_percussion_note(int note, InstrumentFactory factory) {
        percussion_overrides_[note] = std::move(factory);
    }

    void clear_overrides() {
        channel_overrides_.clear();
        program_overrides_.clear();
        percussion_overrides_.clear();
    }

    InstrumentPtr resolve_melodic(int channel, int program) const {
        auto cit = channel_overrides_.find(channel);
        if (cit != channel_overrides_.end()) return cit->second();
        auto pit = program_overrides_.find(program);
        if (pit != program_overrides_.end()) return pit->second();
        return make_gm_instrument(program);
    }

    InstrumentPtr resolve_percussion(int channel, int gm_note) const {
        auto cit = channel_overrides_.find(channel);
        if (cit != channel_overrides_.end()) return cit->second();
        auto nit = percussion_overrides_.find(gm_note);
        if (nit != percussion_overrides_.end()) return nit->second();
        return make_gm_percussion_instrument(gm_note);
    }

    static bool is_percussion_channel(int channel) noexcept { return channel == 9; }

private:
    std::unordered_map<int, InstrumentFactory> channel_overrides_;
    std::unordered_map<int, InstrumentFactory> program_overrides_;
    std::unordered_map<int, InstrumentFactory> percussion_overrides_;
};

}
}
