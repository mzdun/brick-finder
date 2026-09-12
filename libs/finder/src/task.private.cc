// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module finder.task;

import finder.pool;
import img.str;
import std;
import version;

using namespace std::literals;

namespace finder {
	namespace {
		static constexpr auto ident_digits = 5;
		static constexpr auto error_while_removing = static_cast<std::uintmax_t>(-1);
		static constexpr auto sprites_dirname = "html/sprites"sv;
		static constexpr auto pages_dirname = "html/pages"sv;
		static constexpr auto index_dirname = "html"sv;

		std::mutex printing_mutex{};

		void make_directory(std::filesystem::path const& dirname) {
			std::error_code ec{};
			create_directories(dirname, ec);
			if (ec) {
				std::print(std::cerr, "{}: error: cannot create directory `{}`\n", version::program,
				           as_sv(dirname.generic_u8string()));
				return;
			}
		}

		static void enum_pages(std::filesystem::path const& dirname, auto&& cb) {
			std::error_code ec{};
			auto it = std::filesystem::directory_iterator{dirname, ec};
			if (ec) {
				std::print("{}: error {}: cannot list files in {}: {}\n", version::program, ec.value(),
				           as_sv(dirname.generic_u8string()), ec.message());
				std::exit(1);
			}

			for (auto const& entry : it) {
				if (entry.path().extension() != u8".png"sv) continue;
				cb(entry);
			}
		}

		static img::rgba_t marker_to_rgba(auto marker) noexcept {
			if constexpr (std::endian::native == std::endian::big) {
				return static_cast<img::rgba_t>(marker) && 0xFFFFFF << 8;
			} else if constexpr (std::endian::native == std::endian::little) {
				return static_cast<img::rgba_t>(marker) && 0xFFFFFF;
			} else {
				throw "this platform has neither little nor big endian arch";
			}
		}

		std::vector<sprites::info> process_sprites(img::view const& orig, img::rgba_t background) {
			std::vector<sprites::placement> places{};
			auto backed = orig.clone();
			auto const flood_surface = backed->to_view();

			for (auto y = 0u; y < flood_surface.height; ++y) {
				auto const* const bom_row = flood_surface.row_at(y);
				for (auto x = 0u; x < flood_surface.width; ++x) {
					if (!img::get_alpha(bom_row[x])) continue;
					places.push_back(sprites::fill_sprite_area(flood_surface, x, y, marker_to_rgba(places.size())));
				}
			}

			std::vector<sprites::info> result{};
			result.reserve(places.size());
			for (auto const& [pos, marker] : places) {
				if (!pos.left || !pos.top || pos.right == (flood_surface.width - 1) ||
				    pos.bottom == (flood_surface.height - 1))
					continue;
				if (pos.width() < 40 || pos.height() < 40) continue;
				result.push_back(sprites::cut_sprite(flood_surface, orig, pos, marker, background));
			}
			return result;
		}

		static void write_header(std::ofstream& output, std::string_view title, std::string_view style) {
			std::print(output, R"(<html>
<head>
	<title>{}</title>
	<style>{}</style>
</head>
<body>
)",
			           title, style);
		}

		static void write_footer(std::ofstream& output) { std::print(output, "</body>\n</html>\n"); }

		class jobs_t {
			std::atomic<std::size_t> finished{};
			std::atomic<std::size_t> expected{};

		public:
			void finish() { ++finished; }
			void expect(std::size_t count) { expected += count; }
			std::string tag() const { return std::format("[{}/{}]", finished.load(), expected.load()); }
		};

		jobs_t& jobs() {
			static jobs_t object{};
			return object;
		}

		struct job_marker {
			job_marker(std::string_view message) { sync_print(message); }
			~job_marker() { jobs().finish(); }
		};

#define START_JOB(MSG) \
	auto const job = job_marker { MSG }
	}  // namespace

	void sync_print(std::string_view message) {
		auto const tag = jobs().tag();
		std::lock_guard the{printing_mutex};
		std::print("\r{} {}\033[K", tag, message);
	}

	void task_context::clean_report() {
		// clean_report
		// list pages
		// sort_sprites
		// store_sprites
		// write_yaml
		// write_index
		// write_brick_page
		// write_brick_pages
		// write_booklet
		jobs().expect(9);

		START_JOB(std::format("removing {}", as_sv(report_dir.generic_u8string())));
		std::error_code ec{};
		auto const removed_count = std::filesystem::remove_all(report_dir, ec);
		if (removed_count == error_while_removing) {
			std::print(std::cerr, "\n{}: error: {}\n", version::program, ec.message());
		}
	}

	void task_context::list_pages(std::filesystem::path const& dirname) {
		START_JOB("list pages"sv);
		std::size_t count = 0;
		enum_pages(dirname, [&](auto const&) { ++count; });

		filelist.clear();
		filelist.reserve(count);
		enum_pages(dirname, [&](auto const& entry) { filelist.emplace_back(entry.path()); });

		// [N] load_page
		jobs().expect(filelist.size());
	}

	void task_context::load_pages(int pages_count_int) {
		auto const pages_count = std::min(
		    pages_count_int < 0 ? filelist.size() : static_cast<std::size_t>(pages_count_int), filelist.size());

		std::size_t page_id{0};
		for (auto& filename : filelist) {
			if (page_id == pages_count) break;
			post_task(task::loading, [&filename, this, page_id]() { load_page(page_id, filename); });
			++page_id;
		}
	}

	void task_context::load_page(std::size_t page_id, std::filesystem::path const& filename) {
		START_JOB(std::format("read {}", as_sv(filename.generic_u8string())));
		auto pixmap = img::load(filename);
		process_page(page_id, pixmap);
	}

	class rect_filter {
	public:
		bool seen(unsigned x, unsigned y) {
			for (auto const& rect : rects) {
				if (rect.contains(x, y)) {
					return true;
				}
			}
			return false;
		}

		void add(img::rect const& r) { rects.push_back(r); }

	private:
		std::vector<img::rect> rects{};
	};

	void task_context::process_page(std::size_t page_id, img::image_ptr const& pixmap) {
		rect_filter filter{};
		std::vector<sprites::info> sprites{};

		// START_JOB(std::format("proces {}", as_sv(filelist[page_id].generic_u8string())));

		auto const surface = pixmap->to_view();
		for (auto y = 0u; y < surface.height; ++y) {
			auto const* const row = surface.row_at(y);
			for (auto x = 0u; x < surface.width; ++x) {
				if (!sprites::color_matches(row[x], color) || filter.seen(x, y)) continue;

				auto const pos = sprites::fill_bom_area(surface, x, y, color);
				filter.add(pos);

				auto view = surface.subview(pos);
				add_sprite_infos(sprites, process_sprites(view, color));
			}
		}

		post_task(task::postprocessing,
		          [=, this, sprites = std::move(sprites)] mutable { add_sprites(page_id, sprites); });
	}

	void task_context::add_sprites(std::size_t page_id, std::vector<sprites::info>& sprites) {
		std::lock_guard the{db_mutex};
		add_sprite_infos(db, std::move(sprites), page_id);
	}

	void task_context::write_report() {
		sort_sprites();

		post_task(task::postprocessing, [this] { store_sprites(); });
		post_task(task::postprocessing, [this] {
			auto const dirname = report_dir / index_dirname;
			make_directory(dirname);

			write_booklet(dirname);
			write_index(dirname, "Reverse Brick search"sv);
			write_brick_pages();
			write_yaml();
		});
	}

	void task_context::sort_sprites() {
		START_JOB("sort sprites");
		sorted.clear();
		sorted.reserve(db.size());

		std::size_t id{};
		for (auto& [info, pages] : db) {
			sorted.push_back(sprites::sorter::from(info, id));
			++id;
		}

		std::sort(sorted.begin(), sorted.end());
	}

	void task_context::store_sprites() {
		START_JOB("write sprite images");

		auto const sprites_dir = report_dir / sprites_dirname;
		make_directory(sprites_dir);

		std::size_t id{};
		for (auto& [info, pages] : db) {
			info.store(sprites_dir / info.identifier(id, ident_digits, ".png"sv));
			++id;
		}
	}

#define JOB_WRITE(filename) START_JOB("write " filename ""sv)

	void task_context::write_yaml() {
		JOB_WRITE("index.yaml");
		std::ofstream index_yaml{report_dir / "index.yaml"sv};
		for (auto const& id : sorted) {
			auto const& ref = db[id.original_pos];

			std::print(index_yaml, "{}:\n", ref.key.identifier(id.original_pos, ident_digits));
			for (auto const page : ref.pages) {
				std::print(index_yaml, "  - {}\n", as_sv(filelist[page].generic_u8string()));
			}
		}
	}

	void task_context::write_index(std::filesystem::path const& dirname, std::string_view title) {
		JOB_WRITE("index.html");
		auto const [R, G, B, _] = img::to_rgb(color);

		std::ofstream output{dirname / "index.html"sv};
		write_header(output, title,
		             std::format(R"(
		body {{ background: #{:02x}{:02x}{:02x}; }}
		img {{
			padding: 2em;
			height: auto; 
			width: auto; 
			max-width: 75px; 
			max-height: 75px;
		}}
)",
		                         R, G, B));
		for (auto const& id : sorted) {
			std::print(output, "	<a href=\"pages/{0}.html\"><img src=\"sprites/{0}.png\"></a>\n",
			           db[id.original_pos].key.identifier(id.original_pos, ident_digits));
		}
		write_footer(output);
	}

	void task_context::write_brick_page(std::filesystem::path const& dirname, std::size_t original_pos) {
		auto const& [info, pages] = db[original_pos];

		std::ofstream output{dirname / info.identifier(original_pos, ident_digits, ".html"sv)};
		write_header(output, "Brick Pages Listing"sv, R"(
		body { padding: 0; margin: 0; }
		img {
			padding: 0;
			padding-bottom: 1em;
			height: auto;
			width: auto; 
			max-width: 100vw;
			max-height: 100vh;
		}
)"sv);

		for (auto const& page : pages) {
			std::print(output, "<a href=\"../booklet.html#page-{}\"><img src=\"{}\"></a>\n", page,
			           as_sv(filelist[page].generic_u8string()));
		}
		write_footer(output);
	}

	void task_context::write_brick_pages() {
		START_JOB("write brick indexes"sv);
		auto const dirname = report_dir / pages_dirname;
		make_directory(dirname);

		for (auto const& id : sorted) {
			write_brick_page(dirname, id.original_pos);
		}
	}

	void task_context::write_booklet(std::filesystem::path const& dirname) {
		JOB_WRITE("booklet.html");
		std::ofstream output{dirname / "booklet.html"sv};
		write_header(output, "Booklet"sv, R"(
		body { padding: 0; margin: 0; }
		img {
			padding: 0;
			padding-bottom: 1em;
			height: auto;
			width: auto; 
			max-width: 100vw;
			max-height: 100vh;
		}
)"sv);

		std::size_t page_id = 0;
		for (auto const& filename : filelist) {
			std::print(output, "<a id=\"page-{1}\" href=\"{0}\"><img src=\"{0}\"></a>\n",
			           as_sv(filename.generic_u8string()), page_id);
			++page_id;
		}
		write_footer(output);
	}
}  // namespace finder
