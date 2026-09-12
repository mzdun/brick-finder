// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module img;

import std;
import img.png;
import img.reg;

namespace img {
	void register_image_factories() { register_png(); }

	image_ptr load(std::filesystem::path const& path) noexcept { return load_from_registry(path); }

	void store_png_image(view const& source, std::filesystem::path const& filename) noexcept {
		auto const ptr = open(filename, true);
		if (!ptr) {
			return;
		}

		store_png_image_impl(ptr.get(), source);
	}
};  // namespace img
