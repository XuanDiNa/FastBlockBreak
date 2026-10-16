#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <windows.h>

namespace fastblockbreak {

struct ExecutableIdentity {
    std::uint16_t machine{};
    std::uint32_t timeDateStamp{};
    std::uint32_t sizeOfImage{};
    std::uint32_t checkSum{};
};

struct ByteGuard {
    std::uint64_t rva{};
    std::array<std::uint8_t, 32> bytes{};
    std::array<std::uint8_t, 32> mask{};
    std::size_t length{};
};

struct VersionProfile {
    char const* gameVersion{};
    std::array<ExecutableIdentity, 2> acceptedExecutables{};
    ByteGuard continueDestroyBlockHook{};
    ByteGuard readGameType{};
    std::uint32_t blockBreakDelayOffset{};
    std::uint32_t hitTypeOffset{};
    std::uint32_t inputFlagsOffset{};
    std::uint32_t cooldownVtableOffset{};
    std::uint32_t blockHitValue{};
    std::uint32_t entityHitValue{};
    std::uint32_t missHitValue{};
};

[[nodiscard]] bool MatchBytes(std::uint8_t const* actual, ByteGuard const& guard) noexcept;
[[nodiscard]] bool IdentityMatches(ExecutableIdentity const& actual,
                                   ExecutableIdentity const& expected) noexcept;
[[nodiscard]] VersionProfile const& TargetProfile1265203() noexcept;
[[nodiscard]] VersionProfile const& TargetProfile1264501() noexcept;
[[nodiscard]] VersionProfile const& TargetProfile1265101() noexcept;
[[nodiscard]] std::array<VersionProfile const*, 3> const& SupportedProfiles() noexcept;
[[nodiscard]] VersionProfile const* SelectVersionProfile(ExecutableIdentity const& identity) noexcept;
[[nodiscard]] VersionProfile const* FindSupportedProfile(HMODULE module) noexcept;
[[nodiscard]] bool ReadExecutableIdentity(HMODULE module, ExecutableIdentity& out) noexcept;
[[nodiscard]] bool ValidateLoadedModule(HMODULE module, VersionProfile const& profile) noexcept;

} // namespace fastblockbreak
