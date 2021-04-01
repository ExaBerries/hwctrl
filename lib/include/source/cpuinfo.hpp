#pragma once
#include "../basic_types.hpp"
#include <string>
#include <vector>
#include <variant>

namespace hwctrl::source {
	struct cpuinfo {
		struct processor {
			uint32_t id = 0;
			double mhz = 0;
		};

		struct core {
			uint32_t id = 0;
			std::vector<processor> processors{};
		};

		struct cpu {
			uint32_t physical_id = 0;
			std::string name{};
			std::string vendor_id{};
			uint32_t family = 0;
			uint32_t model = 0;
			uint32_t physical_cores = 0;
			std::vector<core> cores{};
		};
		std::vector<cpu> cpus{};
	};

	[[nodiscard]] std::variant<std::string, hwctrl_error> read_cpuinfo() noexcept;
	[[nodiscard]] std::variant<cpuinfo, hwctrl_error> parse_cpuinfo(const std::string& str) noexcept;
	[[nodiscard]] std::string cpuinfo_string(const cpuinfo& ci) noexcept;
} // namespace hwctrl::source
