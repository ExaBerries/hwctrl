#include <source/cpuinfo.hpp>
#include <util/file.hpp>
#include <string_view>
#include <memory>
#include <iostream>

namespace hwctrl::source {
	[[nodiscard]] std::variant<std::string, hwctrl_error> read_cpuinfo() noexcept {
		return util::file::read_ram_file("/proc/cpuinfo");
	}

	[[nodiscard]] std::variant<cpuinfo, hwctrl_error> parse_cpuinfo(const std::string& str) noexcept {
		cpuinfo ci{};
		std::vector<std::vector<std::pair<std::string_view, std::string_view>>> entries{};
		entries.emplace_back();
		entries.back().emplace_back();
		enum {
			KEY,
			VALUE
		} mode = KEY;
		std::string_view::const_iterator last_non_space = nullptr;
		for (const char* current = str.data(); current != std::addressof(*str.end()); current++) {
			char c = *current;
			if (c == '\n') {
				if (mode == KEY) {
					// add new section
					entries.emplace_back();
					// add new entry to new section
					entries.back().emplace_back();
				} else {
					// add new entry to current section
					auto& str_view = entries.back().back().second;
					str_view = std::string_view{str_view.begin(), last_non_space + 1};
					entries.back().emplace_back();
					mode = KEY;
				}
			} else if (c == ':') {
				// switch from key to value
				mode = VALUE;
				// remove trailing spaces from key
				auto& str_view = entries.back().back().first;
				str_view = std::string_view{str_view.begin(), last_non_space + 1};
			} else {
				if (c != ' ' && c != '\t') {
					last_non_space = current;
				}
				if (mode == KEY) {
					auto& str_view = entries.back().back().first;
					if (str_view.data() == nullptr) {
						// create string view for key
						str_view = std::string_view{current, 1};
					} else {
						// add character to key string view
						str_view = std::string_view{str_view.begin(), str_view.end() + 1};
					}
				} else {
					auto& str_view = entries.back().back().second;
					if (str_view.data() == nullptr) {
						// create string view for value
						str_view = std::string_view{current, 1};
					} else {
						// add character to value string view
						str_view = std::string_view{str_view.begin(), str_view.end() + 1};
					}
				}
			}
		}
		auto starts_with = [](std::string_view a, std::string_view b) noexcept {
			if (a.size() < b.size()) {
				return false;
			}
			return std::string_view{a.data(), b.size()} == b;
		};
		uint32_t processor_id = 0;
		std::string vendor_id;
		uint32_t cpu_family = 0;
		uint32_t model = 0;
		std::string model_name;
		double mhz = 0;
		uint32_t physical_id = 0;
		uint32_t core_id = 0;
		for (const auto& entry : entries) {
			for (const auto& [key, value] : entry) {
				if (starts_with(key, "processor")) {
					processor_id = static_cast<uint32_t>(std::atoi(value.data()));
				} else if (starts_with(key, "vendor_id")) {
					vendor_id = value;
				} else if (starts_with(key, "cpu family")) {
					cpu_family = static_cast<uint32_t>(std::atoi(value.data()));
				} else if (starts_with(key, "model name")) {
					model_name = value;
				} else if (starts_with(key, "model")) {
					model = static_cast<uint32_t>(std::atoi(value.data()));
				} else if (starts_with(key, "cpu MHz")) {
					mhz = std::stod(value.data());
				} else if (starts_with(key, "physical id")) {
					physical_id = static_cast<uint32_t>(std::atoi(value.data()));
				} else if (starts_with(key, "core id")) {
					core_id = static_cast<uint32_t>(std::atoi(value.data()));
				}
			}
			auto& cpu = [&]()noexcept -> cpuinfo::cpu& {
					for (auto& c : ci.cpus) {
						if (c.physical_id == physical_id) {
							return c;
						}
					}
					return ci.cpus.emplace_back();
			}();
			cpu.physical_id = physical_id;
			cpu.vendor_id = vendor_id;
			cpu.family = cpu_family;
			cpu.model = model;
			cpu.name = model_name;
			auto& core = [&]() noexcept -> cpuinfo::core& {
				for (auto& c : cpu.cores) {
					if (c.id == core_id) {
						return c;
					}
				}
				return cpu.cores.emplace_back();
			}();
			core.id = core_id;
			auto& proc = [&]() noexcept -> cpuinfo::processor& {
				for (auto& p : core.processors) {
					if (p.id == processor_id) {
						return p;
					}
				}
				return core.processors.emplace_back();
			}();
			proc.id = processor_id;
			proc.mhz = mhz;
		}
		return ci;
	}

	[[nodiscard]] std::string cpuinfo_string(const cpuinfo& ci) noexcept {
		std::string str;
		for (auto& cpu : ci.cpus) {
			str += "cpu ";
			str += std::to_string(cpu.physical_id);
			str += ":\n";
			str += "\tname: ";
			str += cpu.name;
			str += "\t(";
			str += std::to_string(cpu.model);
			str += ")\n";
			str += "\tvendor: ";
			str += cpu.vendor_id;
			str += "\n";
			str += "\tfamily: ";
			str += std::to_string(cpu.family);
			str += "\n";
			for (auto& core : cpu.cores) {
				str += "\tcore ";
				str += std::to_string(core.id);
				str += ": \n";
				for (auto& proc : core.processors) {
					str += "\t\tprocessor ";
					str += std::to_string(proc.id);
					str += ": ";
					str += std::to_string(proc.mhz);
					str += "\n";
				}
			}
		}
		return str;
	}
} // namespace spruce::source
