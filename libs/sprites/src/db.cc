// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module sprites;

import img;
import std;

namespace sprites {
	bool pixels_are_similar(img::view const& left, img::view const& right) {
		if (left.width != right.width || left.height != right.height) {
			return false;
		}

		auto const area = left.width * left.height;
		if (area == 0) {
			return true;
		}

		unsigned long long summed_error{};
		static constexpr auto max_ull = std::numeric_limits<unsigned long long>::max();

		for (auto y = 0u; y < left.height; ++y) {
			auto const* lhs = left.row_at(y);
			auto const* rhs = right.row_at(y);
			for (auto x = 0u; x < left.width; ++x, ++lhs, ++rhs) {
				auto const error = static_cast<unsigned long long>(clr_diff(*lhs, *rhs));
				if ((max_ull - error) <= summed_error) {
					[[unlikely]];
					summed_error = max_ull;
				} else {
					summed_error += error;
				}
			}
		}

		auto const error_per_pixel = summed_error / area;
		return error_per_pixel <= max_rgb_error;
	}

	img::view offset_view_from(img::view const& view, int x_offset, int y_offset, unsigned width, unsigned height) {
		if (view.width <= width) {
			x_offset = 0;
			width = view.width;
		}
		if (view.height <= height) {
			y_offset = 0;
			height = view.height;
		}

		return view.subview(x_offset, y_offset, width, height);
	}

	template <std::integral T>
	static inline T abs_diff(T a, T b) {
		return a > b ? a - b : b - a;
	}

	bool is_similar(img::view const& lhs, img::view const& rhs) {
		static constexpr int offsets[] = {0, 1, 2, 3, 4};
		static constexpr auto max_offset = offsets[std::size(offsets) - 1];

		auto const diff_width = abs_diff(lhs.width, rhs.width);
		auto const diff_height = abs_diff(lhs.height, rhs.height);
		if (diff_width > max_offset || diff_height > max_offset) return false;

		auto const width_offsets = std::span<int const>{offsets, static_cast<std::size_t>(diff_width) + 1};
		auto const height_offsets = std::span<int const>{offsets, static_cast<std::size_t>(diff_height) + 1};

		for (auto y_offset : height_offsets) {
			for (auto x_offset : width_offsets) {
				auto const checked_width = std::min(lhs.width, rhs.width);
				auto const checked_height = std::min(lhs.height, rhs.height);
				if (pixels_are_similar(offset_view_from(lhs, x_offset, y_offset, checked_width, checked_height),
				                       offset_view_from(rhs, x_offset, y_offset, checked_width, checked_height))) {
					return true;
				}
			}
		}

		return false;
	}

	inline img::view get_view(info const& info) { return info.pixels->to_view(); }
	inline img::view get_view(record const& row) { return get_view(row.key); }

	template <typename Collection>
	auto locate(Collection& pixmaps, img::view const& rhs) -> std::remove_reference_t<decltype(*std::begin(pixmaps))>* {
		for (auto& pixmap : pixmaps) {
			if (is_similar(get_view(pixmap), rhs)) return &pixmap;
		}

		return nullptr;
	}

	void add_sprite_infos(std::vector<info>& result, std::vector<info>&& sprites) {
		result.reserve(result.size() + sprites.size());

		for (auto& sprite : sprites) {
			if (locate(result, get_view(sprite))) continue;
			result.push_back(std::move(sprite));
		}
	}

	void add_sprite_infos(std::vector<record>& result, std::vector<info>&& sprites, std::size_t page_id) {
		result.reserve(result.size() + sprites.size());

		for (auto& sprite : sprites) {
			auto const ptr = locate(result, get_view(sprite));
			if (ptr) {
				ptr->pages.insert(page_id);
				continue;
			}
			result.push_back(record{.key = std::move(sprite), .pages = {page_id}});
		}
	}
}  // namespace sprites
