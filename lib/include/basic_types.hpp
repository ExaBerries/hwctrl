#pragma once
#include <cstdint>
#include <string>

namespace hwctrl {
	struct hwctrl_error {
		std::string message;
	};

	struct voltage {
		uint32_t millivolts = 0;
		uint32_t millivolts_min = 0;
		uint32_t millivolts_max = 0;
		uint32_t millivolts_increment = 0;
	};

	struct memory_clock {
		uint16_t clock_picoseconds = 0;
		uint16_t clock_mhz = 0;
		uint16_t clock_mt = 0;
		uint16_t clock_mtb = 0;
		uint16_t clock_ftb = 0;
	};

	struct memory_timing {
		uint32_t timing_picoseconds = 0;
		uint16_t timing_mtb = 0;
		uint16_t timing_ftb = 0;
	};
} // namespace hwctrl

