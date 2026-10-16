#include "FastBlockBreak/NativeState.h"

#include <windows.h>

namespace fastblockbreak {

HitType MapNativeHitType(std::uint32_t value) noexcept {
    switch (value) {
    case 0u:
        return HitType::Block;
    case 1u:
        return HitType::Entity;
    case 2u:
    case 3u:
        return HitType::Miss;
    default:
        return HitType::Unknown;
    }
}

bool IsNativeAttackHeld(std::uint32_t flags) noexcept {
    return (flags & 0x6u) != 0u;
}

bool ClearGameModeBlockBreakDelay(void* gameMode) noexcept {
    if (!gameMode) {
        return false;
    }
    __try {
        auto& delay = *reinterpret_cast<std::uint64_t*>(
            static_cast<std::uint8_t*>(gameMode) + 0x90);
        if (delay == 0) {
            return false;
        }
        delay = 0;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

std::int64_t InvokeContinueDestroyBlockWithoutDelay(
    ContinueDestroyBlockCallback original, void* gameMode, void* blockPosition,
    std::uint8_t face, float* playerPosition, std::uint8_t* destroyed) noexcept {
    if (!original || !gameMode) {
        return 0;
    }
    (void)ClearGameModeBlockBreakDelay(gameMode);
    std::int64_t result{};
    __try {
        result = original(gameMode, blockPosition, face, playerPosition, destroyed);
    } __finally {
        (void)ClearGameModeBlockBreakDelay(gameMode);
    }
    return result;
}

std::int64_t InvokeCreativeContinueDestroyBlockWithoutDelay(
    ContinueDestroyBlockCallback original, ReadGameTypeCallback readGameType,
    void* gameMode, void* blockPosition, std::uint8_t face,
    float* playerPosition, std::uint8_t* destroyed) noexcept {
    if (!original || !readGameType || !gameMode) {
        return 0;
    }
    auto* bytes = static_cast<std::uint8_t*>(gameMode);
    bool creative = false;
    __try {
        auto* playerContext = *reinterpret_cast<std::uint8_t**>(bytes + 0x08);
        if (playerContext) {
            auto const mode = readGameType(playerContext + 0x08);
            creative = mode == 1;
            if (mode == 5) {
                // IDA-verified original branch: mode 5 resolves the world default
                // through [player+0x1D8], vslot +0x460; only result 1 is creative.
                auto* receiver = *reinterpret_cast<void**>(playerContext + 0x1D8);
                auto* table = receiver ? *reinterpret_cast<std::uint8_t**>(receiver) : nullptr;
                auto getter = table ? *reinterpret_cast<ReadGameTypeCallback*>(table + 0x460)
                                    : nullptr;
                creative = getter && getter(receiver) == 1;
            }
        }
        if (creative) {
            *reinterpret_cast<float*>(bytes + 0x30) = 1.0001f;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        creative = false;
    }
    (void)ClearGameModeBlockBreakDelay(gameMode);
    std::int64_t result{};
    __try {
        result = original(gameMode, blockPosition, face, playerPosition, destroyed);
    } __finally {
        (void)ClearGameModeBlockBreakDelay(gameMode);
        if (creative) {
            __try {
                *reinterpret_cast<float*>(bytes + 0x30) = 0.0f;
            } __except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
    }
    return result;
}

} // namespace fastblockbreak
