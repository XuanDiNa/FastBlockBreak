#include "FastBlockBreak/NativeState.h"
#include "FastBlockBreak/VersionProfile.h"

#include <MinHook.h>
#include <atomic>
#include <cstdint>
#include <windows.h>

namespace {

std::atomic<int> g_status{0};
std::atomic<bool> g_initializing{false};
fastblockbreak::ContinueDestroyBlockCallback g_originalContinueDestroyBlock = nullptr;
fastblockbreak::ReadGameTypeCallback g_readGameType = nullptr;
void* g_continueDestroyBlockTarget = nullptr;

std::int64_t __fastcall HookContinueDestroyBlock(
    void* gameMode, void* blockPosition, std::uint8_t face,
    float* playerPosition, std::uint8_t* destroyed) {
    return fastblockbreak::InvokeCreativeContinueDestroyBlockWithoutDelay(
        g_originalContinueDestroyBlock, g_readGameType, gameMode, blockPosition,
        face, playerPosition, destroyed);
}

int Install() noexcept {
    if (g_status.load(std::memory_order_acquire) == 1) {
        return 1;
    }
    auto game = GetModuleHandleW(nullptr);
    auto const* selected = fastblockbreak::FindSupportedProfile(game);
    if (!selected) {
        OutputDebugStringA("[FastBlockBreak] inactive: unsupported binary or entry guard mismatch\n");
        return 0;
    }
    auto const& profile = *selected;
    g_continueDestroyBlockTarget = reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(game) + profile.continueDestroyBlockHook.rva);
    g_readGameType = reinterpret_cast<fastblockbreak::ReadGameTypeCallback>(
        reinterpret_cast<std::uintptr_t>(game) + profile.readGameType.rva);
    if (MH_Initialize() != MH_OK ||
        MH_CreateHook(g_continueDestroyBlockTarget, &HookContinueDestroyBlock,
                      reinterpret_cast<void**>(&g_originalContinueDestroyBlock)) != MH_OK ||
        MH_EnableHook(g_continueDestroyBlockTarget) != MH_OK) {
        MH_Uninitialize();
        g_originalContinueDestroyBlock = nullptr;
        g_readGameType = nullptr;
        g_continueDestroyBlockTarget = nullptr;
        OutputDebugStringA("[FastBlockBreak] inactive: hook installation failed\n");
        return 0;
    }
    g_status.store(1, std::memory_order_release);
    OutputDebugStringA("[FastBlockBreak] 0.2.0-test2 ready; Minecraft ");
    OutputDebugStringA(profile.gameVersion);
    OutputDebugStringA("\n");
    return 1;
}

DWORD WINAPI InitializeThread(void*) {
    HMODULE pinned{};
    if (!GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(&InitializeThread), &pinned)) {
        g_initializing.store(false, std::memory_order_release);
        return 0;
    }
    Install();
    g_initializing.store(false, std::memory_order_release);
    return 0;
}

} // namespace

extern "C" __declspec(dllexport) int FastBlockBreakInit(HINSTANCE) {
    if (g_status.load(std::memory_order_acquire) == 1) {
        return 1;
    }
    bool expected = false;
    if (!g_initializing.compare_exchange_strong(expected, true)) {
        return 0;
    }
    auto thread = CreateThread(nullptr, 0, InitializeThread, nullptr, 0, nullptr);
    if (!thread) {
        g_initializing.store(false, std::memory_order_release);
        return 0;
    }
    CloseHandle(thread);
    return 1;
}

extern "C" __declspec(dllexport) int FastBlockBreakStop() {
    if (g_status.exchange(0, std::memory_order_acq_rel) != 1) {
        return 1;
    }
    auto const disabled = MH_DisableHook(g_continueDestroyBlockTarget) == MH_OK;
    MH_RemoveHook(g_continueDestroyBlockTarget);
    MH_Uninitialize();
    g_originalContinueDestroyBlock = nullptr;
    g_readGameType = nullptr;
    g_continueDestroyBlockTarget = nullptr;
    return disabled ? 1 : 0;
}

extern "C" __declspec(dllexport) int FastBlockBreakStatus() {
    return g_status.load(std::memory_order_acquire);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        FastBlockBreakInit(instance);
    }
    return TRUE;
}
