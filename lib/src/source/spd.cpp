#include <source/spd.hpp>
#include <bitset>
#include <type_traits>
#include <cstring>

namespace hwctrl::source {
	[[nodiscard]] static constexpr memory_clock round_ddr4_jdec_mem_clk(uint8_t mtb, uint8_t ftb) noexcept {
		uint16_t cycle_time = static_cast<uint16_t>(mtb * 125 + *reinterpret_cast<int8_t*>(&ftb));
		switch (cycle_time) {
			case 1500:
				return memory_clock{cycle_time, 666, 1333, mtb, ftb};
			case 938:
				return memory_clock{cycle_time, 1066, 2133, mtb, ftb};
			case 666:
				return memory_clock{cycle_time, 1500, 3000, mtb, ftb};
			default:
				{
					uint16_t clk = static_cast<uint16_t>(1e6f / static_cast<float>(cycle_time));
					return memory_clock{cycle_time, clk, static_cast<uint16_t>(clk * 2u), mtb, ftb};
				}
		}
	}

	[[nodiscard]] static constexpr memory_timing calculate_ddr4_timing(uint16_t mtb, uint8_t ftb) noexcept {
		return memory_timing(static_cast<uint32_t>(mtb * 125 + *reinterpret_cast<int8_t*>(&ftb)), mtb, ftb);
	}

	[[nodiscard]] static constexpr memory_timing calculate_ddr4_timing(uint16_t mtb) noexcept {
		return memory_timing(mtb * 125, mtb, 0);
	}

	[[nodiscard]] static xmp_20_data parse_xmp_20(const unsigned char* data) noexcept {
		xmp_20_data xmp_data{};
		std::bitset<8> byte386{data[386]};
		xmp_data.profiles[0].enable = byte386.test(0);
		xmp_data.profiles[1].enable = byte386.test(1);
		xmp_data.profiles[0].dimms_per_channel = (data[386] & 0b00001100) >> 2;
		xmp_data.profiles[1].dimms_per_channel = (data[386] & 0b00110000) >> 4;
		auto* profile_data = &data[393];
		for (auto i = 0u; i < 2u; i++) {
			auto& profile = xmp_data.profiles[i];
			profile.dimm_voltage = {(profile_data[0] >> 7) * 1000u + (profile_data[0] & 0b01111111) * 10u, 0, 0, 0};
			profile.clk = round_ddr4_jdec_mem_clk(profile_data[3], profile_data[38]);
			profile.tCL = calculate_ddr4_timing(profile_data[8], profile_data[37]);
			profile.tRCD = calculate_ddr4_timing(profile_data[9], profile_data[36]);
			profile.tRP = calculate_ddr4_timing(profile_data[10], profile_data[35]);
			profile.tRAS = calculate_ddr4_timing(static_cast<uint16_t>(((profile_data[11] & 0b00001111) << 8) | profile_data[12]));
			profile.tRC = calculate_ddr4_timing(static_cast<uint16_t>(((profile_data[11] & 0b11110000) << 4) | profile_data[13]), profile_data[34]);
			profile.tRFC1 = calculate_ddr4_timing(*reinterpret_cast<const uint16_t*>(&profile_data[14]));
			profile.tRFC2 = calculate_ddr4_timing(*reinterpret_cast<const uint16_t*>(&profile_data[16]));
			profile.tRFC4 = calculate_ddr4_timing(*reinterpret_cast<const uint16_t*>(&profile_data[18]));
			profile.tFAW = calculate_ddr4_timing(static_cast<uint16_t>(((profile_data[20] & 0b00001111) << 8) | profile_data[21]));
			profile.tRRD_S = calculate_ddr4_timing(profile_data[22], profile_data[32]);
			profile.tRRD_L = calculate_ddr4_timing(profile_data[23], profile_data[31]);
			profile_data += 47;
		}
		return xmp_data;
	}

	static void parse_spd_ddr4_bank_groups(spd_ddr4& spd_data, unsigned char byte) noexcept {
		spd_data.die_size_mb = byte & 0b00001111;
		switch (byte) {
			case 0x94:
				spd_data.bank_groups = 4;
				spd_data.internal_banks = 8;
				break;
			case 0x95:
				spd_data.bank_groups = 4;
				spd_data.internal_banks = 8;
				break;
			case 0x54:
				spd_data.bank_groups = 2;
				spd_data.internal_banks = 8;
				break;
			case 0x55:
				spd_data.bank_groups = 2;
				spd_data.internal_banks = 8;
				break;
			default:
				break;
		}
	}

	static void parse_spd_ddr4_row_column(spd_ddr4& spd_data, unsigned char byte) noexcept {
		switch (byte) {
			case 0x21:
				spd_data.row_count = 16;
				spd_data.column_count = 10;
				break;
			case 0x19:
				spd_data.row_count = 15;
				spd_data.column_count = 10;
				break;
			case 0x29:
				spd_data.row_count = 17;
				spd_data.column_count = 10;
				break;
			default:
				break;
		}
	}

	[[nodiscard]] static std::optional<spd_ddr4::sdram_device_type_t> parse_spd_ddr4_device_type(unsigned char byte) noexcept {
		switch (byte) {
			case 0x00:
				return spd_ddr4::MONOLITHIC_SINGLE_DIE;
			case 0x91:
				return spd_ddr4::NON_MONOLITHIC_2_MULTI_LOAD;
			case 0xa1:
				return spd_ddr4::NON_MONOLITHIC_4_MULTI_LOAD;
			case 0xb1:
				return spd_ddr4::NON_MONOLITHIC_8_MULTI_LOAD;
			case 0x92:
				return spd_ddr4::NON_MONOLITHIC_2_3D;
			case 0xa2:
				return spd_ddr4::NON_MONOLITHIC_4_3D;
			case 0xb2:
				return spd_ddr4::NON_MONOLITHIC_8_3D;
			default:
				return {};
		}
	}

	static void parse_spd_ddr4_tMAW_mul_tRRMAC(spd_ddr4& spd_data, unsigned char byte) noexcept {
		switch (byte) {
			case 0x01:
				spd_data.tMAW_mul = 8192;
				spd_data.tRRMAC_thousands = 700;
				break;
			case 0x02:
				spd_data.tMAW_mul = 8192;
				spd_data.tRRMAC_thousands = 600;
				break;
			case 0x03:
				spd_data.tMAW_mul = 8192;
				spd_data.tRRMAC_thousands = 500;
				break;
			case 0x04:
				spd_data.tMAW_mul = 8192;
				spd_data.tRRMAC_thousands = 400;
				break;
			case 0x05:
				spd_data.tMAW_mul = 8192;
				spd_data.tRRMAC_thousands = 300;
				break;
			case 0x11:
				spd_data.tMAW_mul = 4096;
				spd_data.tRRMAC_thousands = 700;
				break;
			case 0x12:
				spd_data.tMAW_mul = 4096;
				spd_data.tRRMAC_thousands = 600;
				break;
			case 0x13:
				spd_data.tMAW_mul = 4096;
				spd_data.tRRMAC_thousands = 500;
				break;
			case 0x14:
				spd_data.tMAW_mul = 4096;
				spd_data.tRRMAC_thousands = 400;
				break;
			case 0x15:
				spd_data.tMAW_mul = 4096;
				spd_data.tRRMAC_thousands = 300;
				break;
			case 0x21:
				spd_data.tMAW_mul = 2048;
				spd_data.tRRMAC_thousands = 700;
				break;
			case 0x22:
				spd_data.tMAW_mul = 2048;
				spd_data.tRRMAC_thousands = 600;
				break;
			case 0x23:
				spd_data.tMAW_mul = 2048;
				spd_data.tRRMAC_thousands = 500;
				break;
			case 0x24:
				spd_data.tMAW_mul = 2048;
				spd_data.tRRMAC_thousands = 400;
				break;
			case 0x25:
				spd_data.tMAW_mul = 2048;
				spd_data.tRRMAC_thousands = 300;
				break;
			default:
				spd_data.tMAW_mul = {};
				spd_data.tRRMAC_thousands = {};
				break;
		}
	}

	[[nodiscard]] static std::optional<bool> parse_spd_ddr4_endure_higher_vdd(unsigned char byte) noexcept {
		switch (byte) {
			case 0x03:
				return false;
			case 0x0b:
				return true;
			default:
				return {};
		}
	}

	static void parse_spd_ddr4_ranks_chip_size(spd_ddr4& spd_data, unsigned char byte) noexcept {
		switch (byte) {
			case 0x00:
				spd_data.ranks = 1;
				spd_data.chip_size = spd_ddr4::Mb_4;
				break;
			case 0x01:
				spd_data.ranks = 1;
				spd_data.chip_size = spd_ddr4::Mb_8;
				break;
			case 0x02:
				spd_data.ranks = 1;
				spd_data.chip_size = spd_ddr4::Mb_16;
				break;
			case 0x08:
				spd_data.ranks = 2;
				spd_data.chip_size = spd_ddr4::Mb_4;
				break;
			case 0x09:
				spd_data.ranks = 2;
				spd_data.chip_size = spd_ddr4::Mb_8;
				break;
			case 0x0a:
				spd_data.ranks = 2;
				spd_data.chip_size = spd_ddr4::Mb_16;
				break;
			case 0x18:
				spd_data.ranks = 4;
				spd_data.chip_size = spd_ddr4::Mb_4;
				break;
			case 0x19:
				spd_data.ranks = 4;
				spd_data.chip_size = spd_ddr4::Mb_8;
				break;
			default:
				spd_data.ranks = 0;
				spd_data.chip_size = spd_ddr4::UNKNOWN_CHIP_SIZE;
				break;
		}
	}

	[[nodiscard]] static std::optional<uint8_t> parse_spd_ddr4_memory_bus_width_bits(unsigned char byte) noexcept {
		switch (byte) {
			case 0x01:
				return 16;
			case 0x02:
				return 32;
			case 0x03:
				return 64;
			case 0x0b:
				return 72;
			default:
				return {};
		}
	}

	[[nodiscard]] static std::optional<bool> parse_spd_ddr4_module_thermal_sensor(unsigned char byte) noexcept {
		switch (byte) {
			case 0x0000:
				return false;
			case 0x0080:
				return true;
			default:
				return {};
		}
	}

	[[nodiscard]] static ddr_module_manufacturer parse_spd_ddr4_module_manufacturer(uint16_t code) noexcept {
		using name_enum = ddr_module_manufacturer::ddr_module_manufacturer_name;
		return {[&]() noexcept -> name_enum {
			switch (code) {
				case 0x0001:
					return name_enum::KINGSTON;
				case 0x0004:
					return name_enum::G_SKILL;
				case 0x80ce:
					return name_enum::SAMSUNG;
				default:
					return name_enum::UNKNOWN;
			}
		}(), code};
	}

	[[nodiscard]] static ddr_dram_manufacturer parse_spd_ddr4_dram_manufacturer(uint16_t code) noexcept {
		using name_enum = ddr_dram_manufacturer::ddr_dram_manufacturer_name;
		return {[&]() noexcept -> name_enum {
			switch (code) {
				case 0x0080:
					return name_enum::SAMSUNG;
				default:
					return name_enum::UNKNOWN;
			}
		}(), code};
	}

	[[nodiscard]] static std::variant<spd, hwctrl_error> parse_spd_ddr4(const unsigned char* data, [[maybe_unused]] uint32_t size) noexcept {
		spd_ddr4 spd_data{};
		spd_data.spd_revision_major = (data[1] & 0xF0) >> 4;
		spd_data.spd_revision_minor = data[1] & 0x0F;
		switch (data[3]) {
			case 0x00:
				spd_data.device_type = spd_ddr4::UNKNOWN_DEVICE_TYPE;
				break;
			case 0x01:
				spd_data.device_type = spd_ddr4::RDIMM;
				break;
			case 0x02:
				spd_data.device_type = spd_ddr4::UDIMM;
				break;
			case 0x03:
				spd_data.device_type = spd_ddr4::SODIMM;
				break;
			case 0x04:
				spd_data.device_type = spd_ddr4::LRDIMM;
				break;
			default:
				return hwctrl_error{"error - unknown spd device type"};
		}
		parse_spd_ddr4_bank_groups(spd_data, data[4]);
		parse_spd_ddr4_row_column(spd_data, data[5]);
		spd_data.sdram_device_type = parse_spd_ddr4_device_type(data[6]);
		parse_spd_ddr4_tMAW_mul_tRRMAC(spd_data, data[7]);
		spd_data.endure_higher_vdd = parse_spd_ddr4_endure_higher_vdd(data[11]);
		parse_spd_ddr4_ranks_chip_size(spd_data, data[12]);
		spd_data.module_memory_bus_width_bits = parse_spd_ddr4_memory_bus_width_bits(data[13]);
		spd_data.module_thermal_sensor = std::bitset<8>(data[14]).test(7);
		if (data[17] != 0x00) {
			return hwctrl_error{"error - unknown mtb and ftb"};
		}
		spd_data.clock_max = round_ddr4_jdec_mem_clk(data[18], data[125]);
		spd_data.clock_min = round_ddr4_jdec_mem_clk(data[19], data[124]);
		std::bitset<8> cas_supported_byte_0(data[20]);
		std::bitset<8> cas_supported_byte_1(data[21]);
		std::bitset<8> cas_supported_byte_2(data[22]);
		spd_data.cas_supported[0] = cas_supported_byte_0.test(0);
		spd_data.cas_supported[1] = cas_supported_byte_0.test(1);
		spd_data.cas_supported[2] = cas_supported_byte_0.test(2);
		spd_data.cas_supported[3] = cas_supported_byte_0.test(3);
		spd_data.cas_supported[4] = cas_supported_byte_0.test(4);
		spd_data.cas_supported[5] = cas_supported_byte_0.test(5);
		spd_data.cas_supported[6] = cas_supported_byte_0.test(6);
		spd_data.cas_supported[7] = cas_supported_byte_0.test(7);
		spd_data.cas_supported[8] = cas_supported_byte_1.test(0);
		spd_data.cas_supported[9] = cas_supported_byte_1.test(1);
		spd_data.cas_supported[10] = cas_supported_byte_1.test(2);
		spd_data.cas_supported[11] = cas_supported_byte_1.test(3);
		spd_data.cas_supported[12] = cas_supported_byte_1.test(4);
		spd_data.cas_supported[13] = cas_supported_byte_1.test(5);
		spd_data.cas_supported[14] = cas_supported_byte_1.test(6);
		spd_data.cas_supported[15] = cas_supported_byte_1.test(7);
		spd_data.cas_supported[16] = cas_supported_byte_2.test(0);
		spd_data.cas_supported[17] = cas_supported_byte_2.test(1);
		spd_data.tCL_min = calculate_ddr4_timing(data[24], data[123]);
		spd_data.tRCD_min = calculate_ddr4_timing(data[25], data[122]);
		spd_data.tRP_min = calculate_ddr4_timing(data[26], data[121]);
		spd_data.tRAS_min = calculate_ddr4_timing(static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint16_t>(data[27] & 0xF0) << 8) | static_cast<uint16_t>(data[28])));
		spd_data.tRC_min = calculate_ddr4_timing(static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint16_t>(data[27] & 0x0F) << 4) | static_cast<uint16_t>(data[29])), data[120]);
		spd_data.tRFC1_min = calculate_ddr4_timing(static_cast<uint16_t>(static_cast<uint16_t>(data[30]) | static_cast<uint16_t>(static_cast<uint16_t>(data[31]) << 8)));
		spd_data.tRFC2_min = calculate_ddr4_timing(static_cast<uint16_t>(static_cast<uint16_t>(data[32]) | static_cast<uint16_t>(static_cast<uint16_t>(data[33]) << 8)));
		spd_data.tRFC4_min = calculate_ddr4_timing(static_cast<uint16_t>(static_cast<uint16_t>(data[34]) | static_cast<uint16_t>(static_cast<uint16_t>(data[35]) << 8)));
		spd_data.tFAW_min = calculate_ddr4_timing(static_cast<uint16_t>(static_cast<uint16_t>(static_cast<uint16_t>(data[36]) << 8) | static_cast<uint16_t>(data[37])));
		spd_data.tRRD_S_min = calculate_ddr4_timing(data[38], data[119]);
		spd_data.tRRD_L_min = calculate_ddr4_timing(data[39], data[118]);
		spd_data.tCCD_L_min = calculate_ddr4_timing(data[40], data[117]);
		spd_data.module_height = data[128] & 0b00011111;
		spd_data.module_max_thickness = data[129];
		spd_data.ref_raw_card_used = data[130];
		spd_data.module_manufacturer = parse_spd_ddr4_module_manufacturer(static_cast<uint16_t>(data[320]));
		spd_data.module_manufacturing_location = data[322];
		spd_data.module_manufacturing_year = data[323];
		spd_data.module_manufacturing_week = data[324];
		spd_data.serial_number = *reinterpret_cast<const uint32_t*>(&data[325]);
		std::memcpy(spd_data.part_number, &data[329], 20);
		spd_data.module_revision_code = data[349];
		spd_data.dram_manufacturer = parse_spd_ddr4_dram_manufacturer(static_cast<uint16_t>(data[350]));
		spd_data.dram_stepping = data[352];
		if (data[384] == 0x0c && data[385] == 0x4a) {
			uint8_t xmp_major = ((data[387] & 0b00111000) >> 4);
			uint8_t xmp_minor = ((data[387] & 0b00000111) >> 2);
			if (xmp_major == 2 && xmp_minor == 0) {
				spd_data.xmp_data = parse_xmp_20(data);
			}
		}
		return spd_data;
	}

	[[nodiscard]] static std::variant<spd, hwctrl_error> parse_spd_ddr3(const unsigned char* data, [[maybe_unused]] uint32_t size) noexcept {
		spd_ddr3 spd_data{};
		spd_data.spd_revision_major = (data[1] & 0xF0) >> 4;
		spd_data.spd_revision_minor = data[1] & 0x0F;

		return spd_data;
	}

	[[nodiscard]] std::variant<spd, hwctrl_error> parse_spd(const unsigned char* data, uint32_t size) noexcept {
		if (size < 4) {
			return hwctrl_error{"error - spd is to small - cannot determie spd type"};
		}
		uint32_t spd_size = data[0];
		if (spd_size > size) {
			return hwctrl_error{"error - spd reported size is larger than buffer size"};
		}
		if (data[2] == 0x0c) {
			return parse_spd_ddr4(data, size);
		} else if (data[2] == 0x0b) {
			return parse_spd_ddr3(data, size);
		} else {
			return hwctrl_error{"error - unknown spd dram device type"};
		}
	}

	static void add_xmp_20_data(const xmp_20_data& data, std::string& str) noexcept {
		str += "====xmp 2.0====\n";
		for (auto i = 0u; i < 2u; i++) {
			str += "===profile ";
			str += std::to_string(i + 1);
			str += "===\nenable = ";
			str += data.profiles[i].enable ? "true\ndimms per channel = " : "false\ndimms per channel = ";
			str += std::to_string(data.profiles[i].dimms_per_channel);
			str += "\ndimm voltage = ";
			str += std::to_string(data.profiles[i].dimm_voltage.millivolts);
			str += "mV\nmclk = ";
			str += std::to_string(data.profiles[i].clk.clock_mhz);
			str += "Mhz\ntCL = ";
			str += std::to_string(data.profiles[i].tCL.timing_picoseconds);
			str += "ps\ntRCD = ";
			str += std::to_string(data.profiles[i].tRCD.timing_picoseconds);
			str += "ps\ntRP = ";
			str += std::to_string(data.profiles[i].tRP.timing_picoseconds);
			str += "ps\ntRAS = ";
			str += std::to_string(data.profiles[i].tRAS.timing_picoseconds);
			str += "ps\ntRC = ";
			str += std::to_string(data.profiles[i].tRC.timing_picoseconds);
			str += "ps\ntRFC1 = ";
			str += std::to_string(data.profiles[i].tRFC1.timing_picoseconds);
			str += "ps\ntRFC2 = ";
			str += std::to_string(data.profiles[i].tRFC2.timing_picoseconds);
			str += "ps\ntRFC4 = ";
			str += std::to_string(data.profiles[i].tRFC4.timing_picoseconds);
			str += "ps\ntFAW = ";
			str += std::to_string(data.profiles[i].tFAW.timing_picoseconds);
			str += "ps\ntRRD_S = ";
			str += std::to_string(data.profiles[i].tRRD_S.timing_picoseconds);
			str += "ps\ntRRD_L = ";
			str += std::to_string(data.profiles[i].tRRD_L.timing_picoseconds);
			str += "ps\n";
		}
	}

	[[nodiscard]] std::string spd_string(const spd& spd_parsed, bool serial) noexcept {
		return std::visit([&](auto&& arg) noexcept -> std::string {
			using T = std::decay_t<decltype(arg)>;
			std::string str{};
			if constexpr (std::is_same_v<T, spd_ddr4>) {
				str += "ddr4\n";
				str += "spd revsion: ";
				str += std::to_string(arg.spd_revision_major);
				str += ".";
				str += std::to_string(arg.spd_revision_minor);
				str += "\nsupported cas latencies = ";
				for (uint8_t i = 0; i < sizeof(arg.cas_supported); i++) {
					if (arg.cas_supported[i]) {
						str += std::to_string(i + 7);
						str += "-";
					}
				}
				str.back() = '\n';
				str += std::to_string(arg.clock_min.clock_mt);
				str += "Mhz-";
				str += std::to_string(arg.clock_max.clock_mt);
				str += "Mhz\ntCL_min = ";
				str += std::to_string(arg.tCL_min.timing_picoseconds);
				str += "ps\ntRCD_min = ";
				str += std::to_string(arg.tRCD_min.timing_picoseconds);
				str += "ps\ntRP_min = ";
				str += std::to_string(arg.tRP_min.timing_picoseconds);
				str += "ps\ntRAS_min = ";
				str += std::to_string(arg.tRAS_min.timing_picoseconds);
				str += "ps\ntRC_min = ";
				str += std::to_string(arg.tRC_min.timing_picoseconds);
				str += "ps\ntRFC1_min = ";
				str += std::to_string(arg.tRFC1_min.timing_picoseconds);
				str += "ps\ntRFC2_min = ";
				str += std::to_string(arg.tRFC2_min.timing_picoseconds);
				str += "ps\ntRFC4_min = ";
				str += std::to_string(arg.tRFC4_min.timing_picoseconds);
				str += "ps\ntFAW_min = ";
				str += std::to_string(arg.tFAW_min.timing_picoseconds);
				str += "ps\ntRRD_S = ";
				str += std::to_string(arg.tRRD_S_min.timing_picoseconds);
				str += "ps\ntRRD_L = ";
				str += std::to_string(arg.tRRD_L_min.timing_picoseconds);
				str += "ps\ntCCD_L = ";
				str += std::to_string(arg.tCCD_L_min.timing_picoseconds);
				str += "ps\nmodule height = ";
				str += std::to_string(arg.module_height);
				str += "\nmodule width = ";
				str += std::to_string(arg.module_max_thickness);
				str += "\nreference card = ";
				str += std::to_string(arg.ref_raw_card_used);
				str += "\nmodule manufacturer id = ";
				str += std::to_string(arg.module_manufacturer.id_code);
				str += " (";
				str += get_module_manufacturer_name_string(arg.module_manufacturer);
				str += ")\nmodule_manufacturing_location = ";
				str += std::to_string(arg.module_manufacturing_location);
				str += "\nmodule_manufacturing_year = ";
				str += std::to_string(arg.module_manufacturing_location);
				str += "\nmodule_manufacturing_week = ";
				str += std::to_string(arg.module_manufacturing_week);
				str += "\npart number = ";
				str += std::string_view{arg.part_number, 20};
				str += "\nmodule revision code = ";
				str += std::to_string(arg.module_revision_code);
				str += "\ndram manufacturer = ";
				str += std::to_string(arg.dram_manufacturer.id_code);
				str += " (";
				str += get_dram_manufacturer_name_string(arg.dram_manufacturer);
				str += ")\nranks = ";
				str += std::to_string(arg.ranks);
				str += "\ndie size (Mb) = ";
				str += std::to_string(arg.die_size_mb);
				str += "\nchip size = ";
				str += [&]() noexcept -> std::string_view {
					switch (arg.chip_size) {
						case spd_ddr4::Mb_4:
							return "4 Mb";
						case spd_ddr4::Mb_8:
							return "8 Mb";
						case spd_ddr4::Mb_16:
							return "16 Mb";
						case spd_ddr4::UNKNOWN_CHIP_SIZE:
							//[[fallthrough]]
							return "unknown";
						default:
							return "unknown";
					}
				}();
				str += "\n";
				if (serial) {
					str += "serial = ";
					str += std::to_string(arg.serial_number);
					str += "\n";
				}
				if (arg.xmp_data != std::nullopt) {
					add_xmp_20_data(arg.xmp_data.value(), str);
				}
			} else if (std::is_same_v<T, spd_ddr3>) {
				str += "ddr3\n";
				str += "spd revsion: ";
				str += std::to_string(arg.spd_revision_major);
				str += ".";
				str += std::to_string(arg.spd_revision_minor);
				str += "\n";
			}
			return str;
		}, spd_parsed);
	}

	[[nodiscard]] std::string_view get_module_manufacturer_name_string(const ddr_module_manufacturer& module_manufacturer) noexcept {
		switch (module_manufacturer.name) {
			case ddr_module_manufacturer::KINGSTON:
				return "Kingston";
			case ddr_module_manufacturer::G_SKILL:
				return "G.Skill";
			case ddr_module_manufacturer::SAMSUNG:
				return "Samsung";
			case ddr_module_manufacturer::CRUCIAL:
				return "Crucial";
			case ddr_module_manufacturer::TEAM:
				return "Team";
			case ddr_module_manufacturer::UNKNOWN:
				return "unknown";
				//[[fallthrough]]
			default:
				return "unknown";
		}
	}

	[[nodiscard]] std::string_view get_dram_manufacturer_name_string(const ddr_dram_manufacturer& dram_manufacturer) noexcept {
		switch (dram_manufacturer.name) {
			case ddr_dram_manufacturer::SAMSUNG:
				return "Samsung";
			case ddr_dram_manufacturer::SK_HYNIX:
				return "SK Hynix";
			case ddr_dram_manufacturer::MICRON:
				return "Micron";
			case ddr_dram_manufacturer::UNKNOWN:
				return "unknown";
				//[[fallthrough]]
			default:
				return "unknown";
		}
	}
} // namespace hwctrl::source
