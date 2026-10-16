#pragma once

#include <cstdint>

namespace fastblockbreak {

enum class HitType : std::uint8_t { Unknown, Miss, Block, Entity };

struct GateSnapshot {
    bool worldReady{};
    bool playerReady{};
    bool focused{};
    bool blockingScreenOpen{};
    bool attackHeld{};
    HitType hitType{HitType::Unknown};
};

[[nodiscard]] bool IsEligibleBlockPass(GateSnapshot const& state) noexcept;
[[nodiscard]] unsigned DesiredPassCount(GateSnapshot const& state) noexcept;

} // namespace fastblockbreak
