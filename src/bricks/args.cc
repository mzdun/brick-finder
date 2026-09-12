// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module;

#include <args/parser.hpp>

export module bricks.args;

import version;
import std;

namespace args {
	template <>
	struct converter<std::filesystem::path> {
		static inline std::filesystem::path value(parser&, std::string const& arg, std::string const&) {
			return std::u8string_view{reinterpret_cast<char8_t const*>(arg.data()), arg.size()};
		}
	};
};  // namespace args

namespace bricks {
	namespace {
		[[noreturn]] void show_version() {
			std::print("{} version {}\n", version::program, version::ui);
			std::exit(0);
		}  // GCOV_EXCL_LINE[WIN32]
	}  // namespace
}  // namespace bricks

export namespace bricks {
	struct command_line {
		std::filesystem::path in{};
		std::filesystem::path out{};
		std::string color{};
		unsigned threads{std::thread::hardware_concurrency()};
		int page_count{-1};
	};

	command_line parse_cli(::args::args_view const& arguments) {
		command_line result{};

		args::null_translator tr{};
		args::parser parser{"Brick Finder", arguments, &tr};

		parser.custom(bricks::show_version, "v", "version").help("show version and exit").opt();
		parser.arg(result.page_count, "n")
		    .meta("<number>")
		    .help("sets the number of pages to process; defaults to all pages in directory")
		    .opt();
		parser.arg(result.threads, "j")
		    .meta("<number>")
		    .help("sets the number of threads to use; defaults to CPU capabilites")
		    .opt();
		parser.arg(result.color, "c", "color")
		    .meta("<css>")
		    .help("sets the background of block bill of materials; either RGB, RRGGBB, #RGB or #RRGGBB");
		parser.arg(result.out, "o", "outdir").meta("<output>").help("sets the destination directory");
		parser.arg(result.in).meta("<pages>").help("sets the directory with instruction renders");
		parser.parse();

		return result;
	}
}  // namespace bricks
