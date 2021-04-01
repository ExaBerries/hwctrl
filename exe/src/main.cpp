#include <source/spd.hpp>
#include <source/cpuinfo.hpp>
#include <util/file.hpp>
#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <lyra/lyra.hpp>
#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
#include <variant>
#include <optional>
#include <string_view>
#include <string>
#include <type_traits>
#include <filesystem>

namespace hwctrl::exe {
	template <typename T>
	concept HwctrlCommand = std::is_default_constructible_v<T> && requires(T t) {
		T::NAME;
		t.setup_cli(std::declval<lyra::cli_parser&>());
		t.execute();
	};

	template <HwctrlCommand ... commands>
	struct command_match {
		std::optional<std::variant<commands...>> command_optional{};

		// used for compare in fold expression
		[[nodiscard]] constexpr command_match operator|(command_match cmd_match) const noexcept {
			if (cmd_match.command_optional != std::nullopt) {
				return cmd_match;
			} else {
				return *this;
			}
		}
	};

	// instantiates a command_match if command::name == command_string
	template <HwctrlCommand command, HwctrlCommand ... commands>
	[[nodiscard]] constexpr command_match<commands...> compare_name(std::string_view command_string) noexcept  {
		if (command::NAME == command_string) {
			return {{command()}};
		} else {
			return {};
		}
	}

	template <HwctrlCommand ... commands>
	[[nodiscard]] constexpr std::optional<std::variant<commands...>> match_command(const std::string& command_string) noexcept {
		return (compare_name<commands, commands...>(command_string) | ...).command_optional;
	}

	namespace cmd {
		struct spd {
			static constexpr auto NAME = "spd";
			std::filesystem::path path{};
			bool serial = false;

			void setup_cli(lyra::cli_parser& parser) noexcept {
				parser |= lyra::arg(path, "path to spd binary file").required();
				parser |= lyra::opt(serial)["--serial"].optional();
			}

			void execute() noexcept {
				auto file_result = util::file::read_binary_file(path);
				if (const auto* err_ptr = std::get_if<hwctrl_error>(&file_result)) {
					std::cerr << err_ptr->message << std::endl;
					exit(EXIT_FAILURE);
				}
				auto& file_contents = std::get<std::vector<char>>(file_result);
				
				auto spd_parsed = source::parse_spd(reinterpret_cast<unsigned char*>(file_contents.data()), static_cast<uint32_t>(file_contents.size()));
				if (const auto* err = std::get_if<hwctrl_error>(&spd_parsed)) {
					std::cerr << err->message << std::endl;
					exit(EXIT_FAILURE);
				}
				
				std::cout << "===spd info===" << std::endl;
				std::cout << spd_string(std::get<source::spd>(spd_parsed), serial) << std::endl;
			}
		};

		struct debug {
			static constexpr auto NAME = "debug";
			bool serial = false;

			void setup_cli([[maybe_unused]] lyra::cli_parser& parser) noexcept {
				parser |= lyra::opt(serial)["--serial"].optional();
			}

			void execute() {
				{
					// cpuinfo
					auto read_result = source::read_cpuinfo();
					if (const auto* err_ptr = std::get_if<hwctrl_error>(&read_result)) {
						std::cerr << err_ptr->message << std::endl;
						exit(EXIT_FAILURE);
					}
					auto result = source::parse_cpuinfo(std::get<std::string>(read_result));
					if (const auto* err_ptr = std::get_if<hwctrl_error>(&result)) {
						std::cerr << err_ptr->message << std::endl;
						exit(EXIT_FAILURE);
					}
					std::cout << "===cpuinfo===" << std::endl;
					std::cout << source::cpuinfo_string(std::get<source::cpuinfo>(result)) << std::endl;
				}
				{
					// spd
					/*auto file_result = util::file::read_binary_file(path);
					if (const auto* err_ptr = std::get_if<hwctrl_error>(&file_result)) {
						std::cerr << err_ptr->message << std::endl;
						exit(EXIT_FAILURE);
					}
					auto& file_contents = std::get<std::vector<char>>(file_result);

					auto spd_parsed = source::parse_spd(reinterpret_cast<unsigned char*>(file_contents.data()), static_cast<uint32_t>(file_contents.size()));
					if (const auto* err = std::get_if<hwctrl_error>(&spd_parsed)) {
						std::cerr << err->message << std::endl;
						exit(EXIT_FAILURE);
					}

					std::cout << "===spd info===" << std::endl;
					std::cout << spd_string(std::get<source::spd>(spd_parsed), serial) << std::endl;*/
				}
			}
		};
	} // namespace cmd
} // namespace hwctrl::exe

int main(int argc, char* argv[]) {
	using namespace hwctrl::exe;

	bool version = false;
	bool help = false;
	std::string command_string;

	auto cli = lyra::cli_parser();
	cli |= lyra::opt([&](bool flag) {
		version = flag;
		return lyra::parser_result::ok(lyra::parser_result_type::short_circuit_all);
	})["-v"]["--version"]("Show version info").optional();
	cli |= lyra::help(help);
	cli |= lyra::arg(command_string, "command");

	auto result = cli.parse({argc, argv});

	if (version) {
		std::cout << "hwctrl 0.1.0" << std::endl;
		return EXIT_SUCCESS;
	}

	if (help) {
		std::cout << cli << std::endl;
	}

	if (command_string.empty()) {
		std::cerr << cli << std::endl;
		return EXIT_FAILURE;
	}

	auto cmd_opt = match_command<cmd::spd, cmd::debug>(command_string);

	if (cmd_opt != std::nullopt) {
		std::visit([&cli](auto&& arg) noexcept {
			arg.setup_cli(cli);
		}, cmd_opt.value());
	} else {
		std::cerr << cli << std::endl;
		return EXIT_FAILURE;
	}

	result = cli.parse({argc, argv});

	if (!result) {
		std::cerr << result.errorMessage() << std::endl;
		std::cerr << cli << std::endl;
		return EXIT_FAILURE;
	}

	std::visit([](auto&& arg) noexcept {
		arg.execute();
	}, cmd_opt.value());

	return EXIT_SUCCESS;
}
