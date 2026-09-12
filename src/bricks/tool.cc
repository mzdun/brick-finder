// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module;

#include <args/parser.hpp>

export module bricks.tool;

import bricks.args;
import finder;
import sprites;
import img;
import std;

using namespace std::literals;

export namespace bricks {
	int tool(::args::args_view const& arguments);
}  // namespace bricks

namespace bricks {
	namespace {
		unsigned char get_hex(std::string_view view) {
			unsigned result{};
			auto const data = view.data();
			auto const end = data + view.size();
			auto const [ptr, err] = std::from_chars(data, end, result, 16);
			if (ptr != end || err != std::errc{} || result > 0xff) return 0;
			return static_cast<unsigned char>(result);
		}

		unsigned char get_hex(char a, char b) {
			char s[] = {a, b};
			return get_hex(std::string_view{s, 2});
		}

		img::rgba_t parse_color(std::string_view view) {
			if (view.starts_with('#')) {
				view = view.substr(1);
			}

			if (view.size() == 3) {
				auto const r_char = view[0];
				auto const g_char = view[1];
				auto const b_char = view[2];

				return img::to_rgba(get_hex(r_char, r_char), get_hex(g_char, g_char), get_hex(b_char, b_char));
			}

			if (view.size() == 6) {
				return img::to_rgba(get_hex(view.substr(0, 2)), get_hex(view.substr(2, 2)), get_hex(view.substr(4, 2)));
			}

			return 0;
		}

		auto make_loop(unsigned threads) {
			return [threads](finder::callback&& refref) {
				finder::post_task(finder::task::loading, std::move(refref));
				finder::run_tasks(threads);
			};
		}
	}  // namespace

	int tool(::args::args_view const& arguments) {
		auto [indir, outdir, color_str, threads, pages_count_int] = parse_cli(arguments);
		auto const run_loop = make_loop(threads);
		img::register_image_factories();

		finder::task_context ctx{.report_dir = std::move(outdir), .color = parse_color(color_str)};
		run_loop([&, pages_count_int] {
			ctx.clean_report();
			ctx.list_pages(indir);
			ctx.load_pages(pages_count_int);
		});

		std::print("-- Postprocessing\n");

		run_loop([&] { ctx.write_report(); });

		return 0;
	}
}  // namespace bricks
