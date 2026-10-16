#pragma once

#include "FastBlockBreak/Policy.h"

namespace fastblockbreak {

struct DispatchCallbacks {
    void* context{};
    GateSnapshot (*snapshot)(void*) noexcept{};
    void (*clearCooldown)(void*) noexcept{};
    bool (*originalPass)(void*) noexcept{};
};

struct DispatchResult {
    unsigned originalPasses{};
    bool secondPassCompleted{};
};

[[nodiscard]] DispatchResult DispatchAttackTick(DispatchCallbacks const& callbacks) noexcept;

} // namespace fastblockbreak
