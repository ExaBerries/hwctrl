#pragma once
#include "../basic_types.hpp"
#include <variant>
#include <optional>
#include <string>

namespace hwctrl::source {
	struct ddr_module_manufacturer {
		enum ddr_module_manufacturer_name {
			UNKNOWN,
			KINGSTON,
			G_SKILL,
			SAMSUNG,
			CRUCIAL,
			TEAM
		};
		ddr_module_manufacturer_name name;
		uint16_t id_code;
	};

	struct ddr_dram_manufacturer {
		enum ddr_dram_manufacturer_name {
			UNKNOWN,
			SAMSUNG,
			SK_HYNIX,
			MICRON
		};
		ddr_dram_manufacturer_name name;
		uint16_t id_code;
	};

	struct xmp_20_data {
		struct xmp_profile {
			bool enable = false;
			uint8_t dimms_per_channel = 0;
			voltage dimm_voltage;
			memory_clock clk;
			memory_timing tCL;
			memory_timing tRCD;
			memory_timing tRP;
			memory_timing tRAS;
			memory_timing tRC;
			memory_timing tRFC1;
			memory_timing tRFC2;
			memory_timing tRFC4;
			memory_timing tFAW;
			memory_timing tRRD_S;
			memory_timing tRRD_L;
		};
		xmp_profile profiles[2];
	};

	struct spd_ddr4 {
		uint8_t spd_revision_major = 0;
		uint8_t spd_revision_minor = 0;
		enum device_type_t {
			UNKNOWN_DEVICE_TYPE,
			RDIMM,
			UDIMM,
			SODIMM,
			LRDIMM
		} device_type;
		std::optional<uint8_t> bank_groups = 0;
		std::optional<uint8_t> internal_banks = 0;
		uint8_t die_size_mb;
		std::optional<uint8_t> row_count;
		std::optional<uint8_t> column_count;
		enum sdram_device_type_t {
			MONOLITHIC_SINGLE_DIE,
			NON_MONOLITHIC_2_MULTI_LOAD,
			NON_MONOLITHIC_4_MULTI_LOAD,
			NON_MONOLITHIC_8_MULTI_LOAD,
			NON_MONOLITHIC_2_3D,
			NON_MONOLITHIC_4_3D,
			NON_MONOLITHIC_8_3D
		};
		std::optional<sdram_device_type_t> sdram_device_type;
		std::optional<uint16_t> tMAW_mul;
		std::optional<uint16_t> tRRMAC_thousands;
		std::optional<bool> endure_higher_vdd;
		uint8_t ranks;
		enum chip_size_t {
			UNKNOWN_CHIP_SIZE,
			Mb_4,
			Mb_8,
			Mb_16
		};
		chip_size_t chip_size;
		std::optional<uint8_t> module_memory_bus_width_bits;
		bool module_thermal_sensor;
		uint16_t mtb_ps = 125;
		uint16_t ftb_ps = 1;
		memory_clock clock_min;
		memory_clock clock_max;
		bool cas_supported[18];
		memory_timing tCL_min;
		memory_timing tRCD_min;
		memory_timing tRP_min;
		memory_timing tRAS_min;
		memory_timing tRC_min;
		memory_timing tRFC1_min;
		memory_timing tRFC2_min;
		memory_timing tRFC4_min;
		memory_timing tFAW_min;
		memory_timing tRRD_S_min;
		memory_timing tRRD_L_min;
		memory_timing tCCD_L_min;
		uint8_t module_height;
		uint8_t module_max_thickness;
		uint8_t ref_raw_card_used;
		ddr_module_manufacturer module_manufacturer;
		uint8_t module_manufacturing_location;
		uint16_t module_manufacturing_year;
		uint16_t module_manufacturing_week;
		uint32_t serial_number;
		char part_number[20];
		uint8_t module_revision_code;
		ddr_dram_manufacturer dram_manufacturer;
		uint8_t dram_stepping;
		std::optional<xmp_20_data> xmp_data;
	};

	struct spd_ddr3 {
		uint8_t spd_revision_major = 0;
		uint8_t spd_revision_minor = 0;
		enum device_type_t {
			UNKNOWN_DEVICE_TYPE,
			RDIMM,
			UDIMM,
			SODIMM
		} device_type;
		enum chip_size_t {
			CHIP_SIZE_512Mb,
			CHIP_SIZE_1Gb,
			CHIP_SIZE_2Gb,
			CHIP_SIZE_4Gb
		} chip_size;
		std::optional<uint8_t> row_count;
		std::optional<uint8_t> column_count;
	};

	using spd = std::variant<spd_ddr4, spd_ddr3>;

	[[nodiscard]] std::variant<spd, hwctrl_error> parse_spd(const unsigned char* data, uint32_t size) noexcept;
	[[nodiscard]] std::string spd_string(const spd& spd_parsed, bool serial) noexcept;
	[[nodiscard]] std::string_view get_module_manufacturer_name_string(const ddr_module_manufacturer& module_manufacturer) noexcept;
	[[nodiscard]] std::string_view get_dram_manufacturer_name_string(const ddr_dram_manufacturer& dram_manufacturer) noexcept;
} // namespace hwctrl::source
