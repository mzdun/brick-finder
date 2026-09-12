// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module img;

export import img.base;
import std;

export namespace img {
	void register_image_factories();
	image_ptr load(std::filesystem::path const& path) noexcept;
	void store_png_image(view const& source, std::filesystem::path const& filename) noexcept;
}  // namespace img
