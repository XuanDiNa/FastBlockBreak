#include "FastBlockBreak/Policy.h"

namespace fastblockbreak {

bool IsEligibleBlockPass(GateSnapshot const& state) noexcept {
    return state.worldReady && state.playerReady && state.focused &&
           !state.blockingScreenOpen && state.attackHeld &&
           state.hitType == HitType::Block;
}

unsigned DesiredPassCount(GateSnapshot const& state) noexcept {
    return IsEligibleBlockPass(state) ? 2u : 1u;
}

} // namespace fastblockbreak
