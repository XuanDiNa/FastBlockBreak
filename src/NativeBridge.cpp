#include "FastBlockBreak/NativeBridge.h"

#include "FastBlockBreak/NativeState.h"

#include <windows.h>

namespace fastblockbreak {

GateSnapshot MapNativeSnapshot(RawNativeSnapshot const& raw) noexcept {
    return {
        raw.worldReady,
        raw.playerReady,
        raw.gameFocused && raw.foregroundProcess,
        raw.blockingScreenOpen,
        raw.attackHeld,
        MapNativeHitType(raw.rawHitType),
    };
}

GateSnapshot SnapshotNativeCall(void* opaque) noexcept {
    auto* context = static_cast<NativeCallContext*>(opaque);
    if (!context || !context->clientInstance || !context->inputFlags || !context->hitResult) {
        return {};
    }
    __try {
        auto** vtable = *reinterpret_cast<void***>(context->clientInstance);
        if (!vtable) {
            return {};
        }
        using Accessor = void*(__fastcall*)(void*);
        auto player = reinterpret_cast<Accessor>(vtable[0xF8 / sizeof(void*)])(
            context->clientInstance);
        auto world = reinterpret_cast<Accessor>(vtable[0x548 / sizeof(void*)])(
            context->clientInstance);
        DWORD foregroundPid = 0;
        auto foreground = GetForegroundWindow();
        if (foreground) {
            GetWindowThreadProcessId(foreground, &foregroundPid);
        }
        RawNativeSnapshot raw{
            world != nullptr,
            player != nullptr,
            true, // Reaching this callback already passed the native input-allowed gate at +0x4E8.
            foregroundPid == GetCurrentProcessId(),
            false, // Blocking screens do not route through the world-input callback.
            IsNativeAttackHeld(*context->inputFlags),
            *reinterpret_cast<std::uint32_t*>(
                static_cast<std::uint8_t*>(context->hitResult) + 24),
        };
        return MapNativeSnapshot(raw);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return {};
    }
}

void ClearNativeCooldown(void* opaque) noexcept {
    auto* context = static_cast<NativeCallContext*>(opaque);
    if (!context || !context->clientInstance) {
        return;
    }
    __try {
        auto** vtable = *reinterpret_cast<void***>(context->clientInstance);
        using SetNextAllowedTime = void(__fastcall*)(void*, std::int64_t);
        reinterpret_cast<SetNextAllowedTime>(vtable[0xBD0 / sizeof(void*)])(
            context->clientInstance, 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool RunOriginalNativePass(void* opaque) noexcept {
    auto* context = static_cast<NativeCallContext*>(opaque);
    if (!context || !context->original) {
        return false;
    }
    __try {
        auto const result = context->original(context->clientInstance, context->inputFlags,
                                              context->hitResult,
                                              context->auxiliaryHitResult);
        if (!context->firstCalled) {
            context->firstResult = result;
            context->firstCalled = true;
        }
        return result != 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

} // namespace fastblockbreak
