#include "FastBlockBreak/Dispatch.h"

namespace fastblockbreak {

DispatchResult DispatchAttackTick(DispatchCallbacks const& callbacks) noexcept {
    DispatchResult result{};
    if (!callbacks.context || !callbacks.snapshot || !callbacks.clearCooldown ||
        !callbacks.originalPass) {
        return result;
    }

    auto state = callbacks.snapshot(callbacks.context);
    if (!IsEligibleBlockPass(state)) {
        callbacks.originalPass(callbacks.context);
        result.originalPasses = 1;
        return result;
    }

    callbacks.originalPass(callbacks.context);
    result.originalPasses = 1;

    state = callbacks.snapshot(callbacks.context);
    if (!IsEligibleBlockPass(state)) {
        return result;
    }

    callbacks.originalPass(callbacks.context);
    result.originalPasses = 2;
    result.secondPassCompleted = true;
    callbacks.clearCooldown(callbacks.context);
    return result;
}

} // namespace fastblockbreak
