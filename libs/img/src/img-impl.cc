// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module img;

import std;
import img.png;
import img.reg;

namespace img {
	void register_image_factories() { register_png(); }

	image_ptr load(std::filesystem::path const& path) noexcept { return load_from_registry(path); }

	bool store(view const& source, std::filesystem::path const& filename, store_format format) noexcept {
		return store_using_registry(source, filename, format);
	}
};  // namespace img
