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
}  // namespace img
