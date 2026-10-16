#pragma once

#include "FastBlockBreak/Policy.h"

#include <cstdint>
#include <windows.h>

namespace fastblockbreak {

struct RawNativeSnapshot {
    bool worldReady{};
    bool playerReady{};
    bool gameFocused{};
    bool foregroundProcess{};
    bool blockingScreenOpen{};
    bool attackHeld{};
    std::uint32_t rawHitType{};
};

[[nodiscard]] GateSnapshot MapNativeSnapshot(RawNativeSnapshot const& raw) noexcept;

using AttackTickFunction = std::int64_t(__fastcall*)(void*, std::uint32_t*, void*, void*);

struct NativeCallContext {
    void* clientInstance{};
    std::uint32_t* inputFlags{};
    void* hitResult{};
    void* auxiliaryHitResult{};
    AttackTickFunction original{};
    std::int64_t firstResult{};
    bool firstCalled{};
};

[[nodiscard]] GateSnapshot SnapshotNativeCall(void* context) noexcept;
void ClearNativeCooldown(void* context) noexcept;
[[nodiscard]] bool RunOriginalNativePass(void* context) noexcept;

} // namespace fastblockbreak
