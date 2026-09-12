// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module img;

import std;

namespace img {
	static inline unsigned size(int value) noexcept { return value < 0 ? 0 : static_cast<unsigned>(value); }

	view view::subview(int x, int y, int new_width, int new_height) const noexcept {
		if (x < 0) {
			new_width += x;
			x = 0;
		}
		if (y < 0) {
			new_height += y;
			y = 0;
		}

		if (static_cast<unsigned>(x) >= width || static_cast<unsigned>(y) >= height) {
			return {
			    .pitch{pitch},
			};
		}

		if (static_cast<unsigned>(x + new_width) > width) {
			new_width = static_cast<int>(width) - x;
		}

		if (static_cast<unsigned>(y + new_height) > height) {
			new_height = static_cast<int>(height) - y;
		}

		return {
		    .width{size(new_width)},
		    .height{size(new_height)},
		    .pitch{pitch},
		    .hidden_buffer{buffer_at(static_cast<std::size_t>(x), static_cast<std::size_t>(y))},
		};
	}

	image_ptr view::clone() const {
		auto result = image::create(width, height);
		result->to_view().copy_from(0, 0, *this);

		return result;
	}

	void view::copy_from(int x, int y, view const& src) {
		auto target = subview(x, y, src.width, src.height);
		auto source = src.subview(0, 0, target.width, target.height);
		target = target.subview(0, 0, source.width, source.height);

		if (source.empty() || target.empty()) {
			return;
		}

		for (auto row_index = 0u; row_index < target.height; ++row_index) {
			std::memcpy(target.row_at(row_index), source.row_at(row_index),
			            static_cast<std::size_t>(target.width_in_bytes));
		}
	}
}  // namespace img
