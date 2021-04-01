#include <util/file.hpp>
#include <iomanip>
#include <fstream>
#include <iterator>

namespace hwctrl::util::file {
	[[nodiscard]] std::variant<std::vector<char>, hwctrl_error> read_binary_file(const std::filesystem::path& path) noexcept {
		std::ifstream input(path, std::ios::binary);

		if (input.is_open()) {
			return std::vector<char>(std::istreambuf_iterator<char>(input), {});
		}

		return hwctrl_error{"could not read file - \"" + path.string() + "\""};
	}

	[[nodiscard]] std::variant<std::string, hwctrl_error> read_ram_file(const std::filesystem::path& path) noexcept {
		std::ifstream input(path);

		if (input.is_open()) {
			return std::string(std::istreambuf_iterator<char>(input), {});
		}

		return hwctrl_error{"could not read file - \"" + path.string() + "\""};
	}
} // namespace hwctrl::util::file
