#include "FastBlockBreak/VersionProfile.h"

#include <algorithm>

namespace fastblockbreak {

bool MatchBytes(std::uint8_t const* actual, ByteGuard const& guard) noexcept {
    if (!actual || guard.length == 0 || guard.length > guard.bytes.size()) {
        return false;
    }
    for (std::size_t index = 0; index < guard.length; ++index) {
        if (guard.mask[index] != 0 && actual[index] != guard.bytes[index]) {
            return false;
        }
    }
    return true;
}

bool IdentityMatches(ExecutableIdentity const& actual,
                     ExecutableIdentity const& expected) noexcept {
    return actual.machine == expected.machine &&
           actual.timeDateStamp == expected.timeDateStamp &&
           actual.sizeOfImage == expected.sizeOfImage &&
           actual.checkSum == expected.checkSum;
}

VersionProfile const& TargetProfile1265203() noexcept {
    static VersionProfile const profile = [] {
        VersionProfile value{};
        value.gameVersion = "1.26.52.03";
        value.acceptedExecutables = {{
            {0x8664, 0x6AB54E37, 0x12C01000, 0x128CD801},
            {0x8664, 0x6AB54E37, 0x12C02000, 0x128DA293},
        }};
        value.continueDestroyBlockHook.rva = 0x259AFD0;
        value.continueDestroyBlockHook.length = 32;
        value.continueDestroyBlockHook.bytes = {
            0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x55, 0x41,
            0x54, 0x56, 0x57, 0x53, 0x48, 0x81, 0xEC, 0xD8,
            0x00, 0x00, 0x00, 0x48, 0x8D, 0xAC, 0x24, 0x80,
            0x00, 0x00, 0x00, 0x0F, 0x29, 0x7D, 0x40, 0x0F,
        };
        value.continueDestroyBlockHook.mask.fill(0xFF);
        value.readGameType.rva = 0x253D9E0;
        value.readGameType.length = 32;
        value.readGameType.bytes = {
            0x48, 0x8B, 0x51, 0x08, 0x8B, 0x49, 0x10, 0x48,
            0x8B, 0x42, 0x48, 0x4C, 0x8B, 0x42, 0x50, 0x49,
            0x29, 0xC0, 0x49, 0xC1, 0xE8, 0x03, 0x41, 0xFF,
            0xC8, 0x41, 0x81, 0xE0, 0xDF, 0xED, 0xD3, 0x88,
        };
        value.readGameType.mask.fill(0xFF);
        value.blockBreakDelayOffset = 0x90;
        value.inputFlagsOffset = 0;
        value.hitTypeOffset = 24;
        value.cooldownVtableOffset = 0xBD0;
        value.blockHitValue = 0;
        value.entityHitValue = 1;
        value.missHitValue = 3;
        return value;
    }();
    return profile;
}

VersionProfile const& TargetProfile1264501() noexcept {
    static VersionProfile const profile = [] {
        // Entry bytes and shared GameMode layout separately verified in IDA 9.3.
        auto value = TargetProfile1265203();
        value.gameVersion = "1.26.45.01";
        value.acceptedExecutables = {{
            {0x8664, 0x6A8378BA, 0x12888000, 0x12568F46},
            {0x8664, 0x6A8378BA, 0x12889000, 0x1256BF65},
        }};
        value.continueDestroyBlockHook.rva = 0x2D2E310;
        value.readGameType.rva = 0x380C960;
        return value;
    }();
    return profile;
}

VersionProfile const& TargetProfile1265101() noexcept {
    static VersionProfile const profile = [] {
        auto value = TargetProfile1265203();
        value.gameVersion = "1.26.51.01";
        value.acceptedExecutables = {{
            {0x8664, 0x6AA482FD, 0x12C0C000, 0x128E3A0F},
            {0x8664, 0x6AA482FD, 0x12C0D000, 0x128DA158},
        }};
        value.continueDestroyBlockHook.rva = 0x259B230;
        value.readGameType.rva = 0x253DC40;
        return value;
    }();
    return profile;
}

std::array<VersionProfile const*, 3> const& SupportedProfiles() noexcept {
    static std::array<VersionProfile const*, 3> const profiles{
        &TargetProfile1264501(), &TargetProfile1265101(), &TargetProfile1265203()};
    return profiles;
}

VersionProfile const* SelectVersionProfile(ExecutableIdentity const& identity) noexcept {
    for (auto const* profile : SupportedProfiles()) {
        if (std::any_of(profile->acceptedExecutables.begin(), profile->acceptedExecutables.end(),
                        [&](auto const& expected) { return IdentityMatches(identity, expected); })) {
            return profile;
        }
    }
    return nullptr;
}

VersionProfile const* FindSupportedProfile(HMODULE module) noexcept {
    ExecutableIdentity identity{};
    if (!ReadExecutableIdentity(module, identity)) {
        return nullptr;
    }
    auto const* profile = SelectVersionProfile(identity);
    return profile && ValidateLoadedModule(module, *profile) ? profile : nullptr;
}

bool ReadExecutableIdentity(HMODULE module, ExecutableIdentity& out) noexcept {
    if (!module) {
        return false;
    }
    __try {
        auto const* base = reinterpret_cast<std::uint8_t const*>(module);
        auto const* dos = reinterpret_cast<IMAGE_DOS_HEADER const*>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0) {
            return false;
        }
        auto const* nt = reinterpret_cast<IMAGE_NT_HEADERS64 const*>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE ||
            nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            return false;
        }
        out.machine = nt->FileHeader.Machine;
        out.timeDateStamp = nt->FileHeader.TimeDateStamp;
        out.sizeOfImage = nt->OptionalHeader.SizeOfImage;
        out.checkSum = nt->OptionalHeader.CheckSum;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ValidateLoadedModule(HMODULE module, VersionProfile const& profile) noexcept {
    ExecutableIdentity actual{};
    if (!ReadExecutableIdentity(module, actual)) {
        return false;
    }
    auto const identityAccepted = std::any_of(
        profile.acceptedExecutables.begin(), profile.acceptedExecutables.end(),
        [&](ExecutableIdentity const& expected) { return IdentityMatches(actual, expected); });
    if (!identityAccepted) {
        return false;
    }
    auto const base = reinterpret_cast<std::uintptr_t>(module);
    if (profile.continueDestroyBlockHook.length > actual.sizeOfImage ||
        profile.readGameType.length > actual.sizeOfImage ||
        profile.continueDestroyBlockHook.rva >
            actual.sizeOfImage - profile.continueDestroyBlockHook.length ||
        profile.readGameType.rva > actual.sizeOfImage - profile.readGameType.length) {
        return false;
    }
    __try {
        return MatchBytes(reinterpret_cast<std::uint8_t const*>(
                              base + profile.continueDestroyBlockHook.rva),
                          profile.continueDestroyBlockHook) &&
               MatchBytes(reinterpret_cast<std::uint8_t const*>(
                              base + profile.readGameType.rva),
                          profile.readGameType);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

} // namespace fastblockbreak
