#pragma once

#include "FastBlockBreak/Policy.h"

#include <cstdint>

namespace fastblockbreak {

[[nodiscard]] HitType MapNativeHitType(std::uint32_t value) noexcept;
[[nodiscard]] bool IsNativeAttackHeld(std::uint32_t flags) noexcept;
[[nodiscard]] bool ClearGameModeBlockBreakDelay(void* gameMode) noexcept;

using ContinueDestroyBlockCallback = std::int64_t(__fastcall*)(
    void* gameMode, void* blockPosition, std::uint8_t face,
    float* playerPosition, std::uint8_t* destroyed);
using ReadGameTypeCallback = std::int64_t(__fastcall*)(void* playerContext);

[[nodiscard]] std::int64_t InvokeContinueDestroyBlockWithoutDelay(
    ContinueDestroyBlockCallback original, void* gameMode, void* blockPosition,
    std::uint8_t face, float* playerPosition, std::uint8_t* destroyed) noexcept;

[[nodiscard]] std::int64_t InvokeCreativeContinueDestroyBlockWithoutDelay(
    ContinueDestroyBlockCallback original, ReadGameTypeCallback readGameType,
    void* gameMode, void* blockPosition, std::uint8_t face,
    float* playerPosition, std::uint8_t* destroyed) noexcept;

} // namespace fastblockbreak
