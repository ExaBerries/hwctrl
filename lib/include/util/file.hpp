#pragma once
#include "../basic_types.hpp"
#include <variant>
#include <vector>
#include <filesystem>

namespace hwctrl::util::file {
	[[nodiscard]] std::variant<std::vector<char>, hwctrl_error> read_binary_file(const std::filesystem::path& path) noexcept;
	[[nodiscard]] std::variant<std::string, hwctrl_error> read_ram_file(const std::filesystem::path& path) noexcept;
} // namespace hwctrl::util::file
