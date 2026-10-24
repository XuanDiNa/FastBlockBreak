#include "FastBlockBreak/Config.h"
#include "FastBlockBreak/NativeState.h"
#include "FastBlockBreak/VersionProfile.h"

#include <MinHook.h>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <windows.h>

namespace {

std::atomic<int> g_status{0};
std::atomic<bool> g_initializing{false};
std::atomic<bool> g_enabled{true};
std::atomic<bool> g_configWatcherRunning{false};
HANDLE g_configWatcherThread = nullptr;
std::filesystem::path g_configPath;
fastblockbreak::ContinueDestroyBlockCallback g_originalContinueDestroyBlock = nullptr;
fastblockbreak::ReadGameTypeCallback g_readGameType = nullptr;
void* g_continueDestroyBlockTarget = nullptr;

std::int64_t __fastcall HookContinueDestroyBlock(
    void* gameMode, void* blockPosition, std::uint8_t face,
    float* playerPosition, std::uint8_t* destroyed) {
    if (!g_enabled.load(std::memory_order_acquire)) {
        return g_originalContinueDestroyBlock
            ? g_originalContinueDestroyBlock(gameMode, blockPosition, face, playerPosition, destroyed)
            : 0;
    }
    return fastblockbreak::InvokeCreativeContinueDestroyBlockWithoutDelay(
        g_originalContinueDestroyBlock, g_readGameType, gameMode, blockPosition,
        face, playerPosition, destroyed);
}

DWORD WINAPI ConfigWatcherThread(void*) {
    bool enabled = true;
    std::error_code pathError;
    if (std::filesystem::exists(g_configPath, pathError)) {
        if (fastblockbreak::LoadEnabledConfig(g_configPath, enabled)) {
            g_enabled.store(enabled, std::memory_order_release);
        } else {
            OutputDebugStringA("[FastBlockBreak] config invalid; retaining previous enabled state\n");
        }
    } else {
        std::ofstream create(g_configPath, std::ios::binary | std::ios::trunc);
        if (create) create << "{\n  \"enabled\": true\n}\n";
    }
    BY_HANDLE_FILE_INFORMATION lastInfo{};
    bool haveLastInfo = false;
    while (g_configWatcherRunning.load(std::memory_order_acquire)) {
        auto file = CreateFileW(g_configPath.c_str(), GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            BY_HANDLE_FILE_INFORMATION info{};
            auto const queried = GetFileInformationByHandle(file, &info) != FALSE;
            CloseHandle(file);
            auto const changed = queried && (!haveLastInfo ||
                info.dwVolumeSerialNumber != lastInfo.dwVolumeSerialNumber ||
                info.nFileIndexHigh != lastInfo.nFileIndexHigh ||
                info.nFileIndexLow != lastInfo.nFileIndexLow ||
                CompareFileTime(&lastInfo.ftLastWriteTime, &info.ftLastWriteTime) != 0 ||
                info.nFileSizeHigh != lastInfo.nFileSizeHigh ||
                info.nFileSizeLow != lastInfo.nFileSizeLow);
            if (changed) {
                lastInfo = info;
                haveLastInfo = true;
                if (fastblockbreak::LoadEnabledConfig(g_configPath, enabled)) {
                    g_enabled.store(enabled, std::memory_order_release);
                } else {
                    OutputDebugStringA("[FastBlockBreak] config invalid; retaining previous enabled state\n");
                }
            }
        }
        Sleep(250);
    }
    return 0;
}

void StartConfigWatcher(HMODULE module) noexcept {
    wchar_t buffer[MAX_PATH]{};
    auto length = GetModuleFileNameW(module, buffer, MAX_PATH);
    if (!length || length >= MAX_PATH) return;
    g_configPath = std::filesystem::path(buffer).parent_path() / L"config.json";
    g_configWatcherRunning.store(true, std::memory_order_release);
    g_configWatcherThread = CreateThread(nullptr, 0, ConfigWatcherThread, nullptr, 0, nullptr);
    if (!g_configWatcherThread) g_configWatcherRunning.store(false, std::memory_order_release);
}

void StopConfigWatcher() noexcept {
    g_configWatcherRunning.store(false, std::memory_order_release);
    if (g_configWatcherThread) {
        WaitForSingleObject(g_configWatcherThread, 2000);
        CloseHandle(g_configWatcherThread);
        g_configWatcherThread = nullptr;
    }
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
    OutputDebugStringA("[FastBlockBreak] 0.2.0-test3 ready; Minecraft ");
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
    if (Install()) {
        StartConfigWatcher(pinned);
    }
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
    StopConfigWatcher();
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

extern "C" __declspec(dllexport) int FastBlockBreakEnabled() {
    return g_enabled.load(std::memory_order_acquire) ? 1 : 0;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        FastBlockBreakInit(instance);
    }
    return TRUE;
}
