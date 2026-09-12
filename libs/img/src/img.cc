// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module img;

export import img.base;
export import img.str;
import std;

export namespace img {
	void register_image_factories();
	image_ptr load(std::filesystem::path const& path) noexcept;
	bool store(view const& source,
	           std::filesystem::path const& filename,
	           store_format format = store_format::rgba) noexcept;
}  // namespace img
