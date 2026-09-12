// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module img.reg;

import std;
import img.base;

export namespace img {
	class stream {
	public:
		virtual ~stream() = default;

		virtual bool is_eof() const noexcept = 0;
		virtual std::size_t read(void* buffer, std::size_t length) = 0;
		virtual std::size_t write(void const* buffer, std::size_t length) = 0;
		virtual std::size_t tell() const = 0;
		virtual void seek(std::size_t pos) = 0;
		virtual void flush() = 0;

		enum class mode { read, write };

		static std::unique_ptr<stream> open(std::filesystem::path const& filename, mode rw);
	};

	class image_factory {
	public:
		virtual ~image_factory() = default;
		virtual std::span<std::string_view const> extensions() const noexcept = 0;
		virtual bool is_valid(stream& instream) const noexcept = 0;
		virtual std::unique_ptr<image> load(stream& instream) const noexcept = 0;
		virtual void store(view const& pixmap, stream& outstream, store_format format) const noexcept = 0;
	};

	void register_factory(image_factory* factory);

	template <std::derived_from<image_factory> factory_type>
	void register_factory() {
		static factory_type factory{};
		register_factory(&factory);
	}

	std::unique_ptr<image> load_from_registry(stream& instream, std::string_view ext_hint) noexcept;
	std::unique_ptr<image> load_from_registry(std::filesystem::path const& path) noexcept;
	bool store_using_registry(view const& source,
	                          stream& instream,
	                          std::string_view ext_hint,
	                          store_format format) noexcept;
	bool store_using_registry(view const& source, std::filesystem::path const& filename, store_format format) noexcept;
}  // namespace img
