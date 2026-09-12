// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)
module;

#include <cstring>  // there is a na ICE in GCC15, when using memcopy from module std

module img;

namespace img {
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
