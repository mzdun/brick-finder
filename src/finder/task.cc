// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module finder.task;

// import finder.pool;
// import finder.str;
import sprites;
import img;
import std;
// import version;

export namespace finder {
	struct task_context {
		std::mutex db_mutex{};
		std::vector<std::filesystem::path> filelist{};
		std::vector<sprites::record> db{};
		std::filesystem::path report_dir{};
		std::vector<sprites::sorter> sorted{};
		img::rgba_t color{};

		void clean_report();
		void list_pages(std::filesystem::path const& dirname);
		void load_pages(int pages_count_int);
		void load_page(std::size_t page_id, std::filesystem::path const& filename);
		void process_page(std::size_t page_id, img::image_ptr const& pixmap);
		void add_sprites(std::size_t page_id, std::vector<sprites::info>& sprites);
		void write_report();

	private:
		void sort_sprites();
		void store_sprites();
		void write_yaml();
		void write_index(std::filesystem::path const& dirname, std::string_view title);
		void write_brick_page(std::filesystem::path const& dirname, std::size_t original_pos);
		void write_brick_pages();
		void write_booklet(std::filesystem::path const& dirname);
	};
}  // namespace finder
