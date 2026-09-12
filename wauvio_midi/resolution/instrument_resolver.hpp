#pragma once

#include "gm_program_map.hpp"

#include <functional>
#include <unordered_map>

namespace wauvio {
namespace midi {

using InstrumentFactory = std::function<InstrumentPtr()>;

/// Combines bank MSB/LSB and program number into a single lookup key.
/// Bank 0/0 is plain GM; anything else is treated as a banked (GS/XG-style)
/// instrument identity that a soundfont or explicit override can resolve.
struct BankProgram {
    int bank_msb = 0;
    int bank_lsb = 0;
    int program  = 0;

    bool operator==(const BankProgram& o) const noexcept {
        return bank_msb == o.bank_msb && bank_lsb == o.bank_lsb && program == o.program;
    }
};

struct BankProgramHash {
    size_t operator()(const BankProgram& b) const noexcept {
        return (static_cast<size_t>(b.bank_msb) << 16) ^ (static_cast<size_t>(b.bank_lsb) << 8) ^
               static_cast<size_t>(b.program);
    }
};

/// Optional interface a loaded soundfont (or any other instrument bank)
/// can implement so the MIDI resolver can prefer real sampled instruments
/// over the built-in synthesized GM fallback when available.
using IBankProvider = audio::IBankProvider;

class InstrumentResolver {
public:
    void override_channel(int channel, InstrumentFactory factory) {
        channel_overrides_[channel] = std::move(factory);
    }

    void override_program(int program, InstrumentFactory factory) {
        program_overrides_[program] = std::move(factory);
    }

    void override_bank_program(int bank_msb, int bank_lsb, int program, InstrumentFactory factory) {
        bank_program_overrides_[BankProgram{bank_msb, bank_lsb, program}] = std::move(factory);
    }

    void override_percussion_note(int note, InstrumentFactory factory) {
        percussion_overrides_[note] = std::move(factory);
    }

    void clear_overrides() {
        channel_overrides_.clear();
        program_overrides_.clear();
        bank_program_overrides_.clear();
        percussion_overrides_.clear();
    }

    /// Attach a soundfont (or any IBankProvider) so banked/GM instrument
    /// lookups can prefer real sampled presets when one exists, falling
    /// back to the built-in synthesized instruments otherwise.
    void set_bank_provider(std::shared_ptr<IBankProvider> provider) { bank_provider_ = std::move(provider); }
    std::shared_ptr<IBankProvider> bank_provider() const { return bank_provider_; }

    InstrumentPtr resolve_melodic(int channel, int program, int bank_msb = 0, int bank_lsb = 0) const {
        auto cit = channel_overrides_.find(channel);
        if (cit != channel_overrides_.end()) return cit->second();

        auto bit = bank_program_overrides_.find(BankProgram{bank_msb, bank_lsb, program});
        if (bit != bank_program_overrides_.end()) return bit->second();

        if (bank_provider_ && bank_provider_->has_preset(bank_msb, bank_lsb, program))
            return bank_provider_->create_instrument(bank_msb, bank_lsb, program);

        auto pit = program_overrides_.find(program);
        if (pit != program_overrides_.end()) return pit->second();

        return make_gm_instrument(program);
    }

    InstrumentPtr resolve_percussion(int channel, int gm_note, int bank_msb = 0, int bank_lsb = 0) const {
        auto cit = channel_overrides_.find(channel);
        if (cit != channel_overrides_.end()) return cit->second();

        if (bank_provider_ && bank_provider_->has_percussion_preset(bank_msb, bank_lsb, gm_note))
            return bank_provider_->create_percussion_instrument(bank_msb, bank_lsb, gm_note);

        auto nit = percussion_overrides_.find(gm_note);
        if (nit != percussion_overrides_.end()) return nit->second();
        return make_gm_percussion_instrument(gm_note);
    }

    static bool is_percussion_channel(int channel) noexcept { return channel == 9; }

private:
    std::unordered_map<int, InstrumentFactory> channel_overrides_;
    std::unordered_map<int, InstrumentFactory> program_overrides_;
    std::unordered_map<BankProgram, InstrumentFactory, BankProgramHash> bank_program_overrides_;
    std::unordered_map<int, InstrumentFactory> percussion_overrides_;
    std::shared_ptr<IBankProvider> bank_provider_;
};

}
}
