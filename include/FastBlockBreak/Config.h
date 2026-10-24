#pragma once

#include <filesystem>
#include <string>

namespace fastblockbreak {

[[nodiscard]] bool ParseEnabledConfig(std::string const& json, bool& enabled) noexcept;
[[nodiscard]] bool LoadEnabledConfig(std::filesystem::path const& path, bool& enabled) noexcept;

} // namespace fastblockbreak
