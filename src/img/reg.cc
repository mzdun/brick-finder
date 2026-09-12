// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module;

#ifdef _WIN32
#include <Windows.h>
#endif

export module img.reg;

import std;
import img.base;

export namespace img {
	class image_factory {
	public:
		virtual ~image_factory() = default;
		virtual bool is_valid(std::FILE*) const noexcept = 0;
		virtual std::unique_ptr<image> load(std::FILE*) const noexcept = 0;
	};

	void register_factory(image_factory* factory);

	template <std::derived_from<image_factory> factory_type>
	void register_factory() {
		static factory_type factory{};
		register_factory(&factory);
	}

	std::unique_ptr<image> load_from_registry(std::FILE*) noexcept;
	std::unique_ptr<image> load_from_registry(std::filesystem::path const& path) noexcept;

	struct file_closer {
		void operator()(std::FILE* file) { std::fclose(file); }
	};
	using file_ptr = std::unique_ptr<std::FILE, file_closer>;

	file_ptr open(std::filesystem::path const& path, bool write);
}  // namespace img
