// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module;

#include <png.h>

export module img.png;

import std;
import img.base;
import img.reg;

export namespace img {
	void register_png();
}  // namespace img

using namespace std::literals;

namespace img {
	namespace png {
		struct exception {};

		void png_error_fn(png_structp, png_const_charp) { throw png::exception{}; }
		void png_warning_fn(png_structp, png_const_charp) {}

		void stream_read(png_structp png_ptr, png_bytep buffer, size_t size) {
			if (png_ptr == NULL) return;

			auto& instream = *reinterpret_cast<stream*>(png_get_io_ptr(png_ptr));
			auto const check = instream.read(buffer, size);

			if (check != size) png_error(png_ptr, "Read Error");
		}

		void stream_write(png_structp png_ptr, png_bytep buffer, size_t size) {
			if (png_ptr == NULL) return;

			auto& outstream = *reinterpret_cast<stream*>(png_get_io_ptr(png_ptr));
			auto const check = outstream.write(buffer, size);

			if (check != size) png_error(png_ptr, "Write Error");
		}

		void stream_flush(png_structp png_ptr) {
			if (png_ptr == NULL) return;

			auto& outstream = *reinterpret_cast<stream*>(png_get_io_ptr(png_ptr));
			outstream.flush();
		}

		std::vector<png_bytep> rows(view const& image) noexcept {
			std::vector<png_bytep> result(image.height);
			for (auto y = 0u; y < image.height; ++y) {
				result[y] = image.buffer_at(0, y);
			}
			return result;
		}

		struct image_info {
			unsigned width;
			unsigned height;
			size_t row_bytes;
			unsigned channels;
		};

		struct read_struct_deleter {
			png_info* attached{};
			void operator()(png_struct* png_ptr) { png_destroy_read_struct(&png_ptr, &attached, nullptr); }
		};

		struct write_struct_deleter {
			png_info* attached{};
			void operator()(png_struct* png_ptr) { png_destroy_write_struct(&png_ptr, &attached); }
		};

		template <typename Deleter>
		struct rw_struct : std::unique_ptr<png_struct, Deleter> {
			using base = std::unique_ptr<png_struct, Deleter>;

			explicit rw_struct(png_struct* value) : base{value} {
				if (value) {
					auto& deleter = this->get_deleter();
					deleter.attached = png_create_info_struct(value);
					if (!deleter.attached) {
						this->reset();
					}
				}
			}

			png_info* info() const noexcept { return this->get_deleter().attached; }
		};

		struct read_struct : private rw_struct<read_struct_deleter> {
			using base = rw_struct<read_struct_deleter>;
			using base::base;
			using base::operator bool;

			static read_struct create() {
				return read_struct{
				    png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, png_error_fn, png_warning_fn)};
			}

			void set_read_stream(stream& instream) const noexcept { png_set_read_fn(get(), &instream, stream_read); }

			image_info read_info() const noexcept {
				auto const png_ptr = get();
				auto const info_ptr = info();

				png_read_info(png_ptr, info_ptr); /* read all PNG info up to image data */

				png_uint_32 width{}, height{};
				int bit_depth{}, color_type{};
				png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

				if (color_type == PNG_COLOR_TYPE_PALETTE || (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) ||
				    png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
					png_set_expand(png_ptr);
				}
				if (bit_depth == 16) {
					png_set_strip_16(png_ptr);
				}
				if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
					png_set_gray_to_rgb(png_ptr);
				}
				if (color_type == PNG_COLOR_TYPE_RGB) {
					png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
				}

				/* all transformations have been registered; now update info_ptr data,
				 * get rowbytes and channels, and allocate image memory */

				png_read_update_info(png_ptr, info_ptr);

				return {
				    .width = width,
				    .height = height,
				    .row_bytes = png_get_rowbytes(png_ptr, info_ptr),
				    .channels = png_get_channels(png_ptr, info_ptr),
				};
			}

			void read_into(view const& target) const noexcept {
				auto const png_ptr = get();
				png_read_image(png_ptr, rows(target).data());
				png_read_end(png_ptr, info());
			}
		};

		struct write_struct : private rw_struct<write_struct_deleter> {
			using base = rw_struct<write_struct_deleter>;
			using base::base;
			using base::operator bool;

			static write_struct create() {
				return write_struct{
				    png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, png_error_fn, png_warning_fn)};
			}

			void set_write_stream(stream& outstream) const noexcept {
				png_set_write_fn(get(), &outstream, stream_write, stream_flush);
			}

			void write_view(view const& source) const noexcept {
				auto const png_ptr = get();
				auto const info_ptr = info();

				png_set_IHDR(png_ptr, info_ptr, source.width, source.height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
				             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
				png_write_info(png_ptr, info_ptr);
				png_write_image(png_ptr, rows(source).data());
				png_write_end(png_ptr, info_ptr);
			}
		};
	}  // namespace png

	class png_factory : public image_factory {
	public:
		std::span<std::string_view const> extensions() const noexcept override {
			static constexpr std::string_view exts[] = {".png"sv};
			return exts;
		}
		bool is_valid(stream& instream) const noexcept override {
			unsigned char sig[8];
			instream.read(sig, 8);
			return png_sig_cmp(sig, 0, 8) == 0;
		}
		std::unique_ptr<image> load(stream& instream) const noexcept override {
			auto png_ptr = png::read_struct::create();
			if (!png_ptr) {
				return {};
			}

			try {
				png_ptr.set_read_stream(instream);
				auto const info = png_ptr.read_info();

				auto result = image::create(info.width, info.height, info.row_bytes);
				if (!result) {
					return result;
				}

				png_ptr.read_into(result->to_view());

				return result;
			} catch (png::exception const&) {
				return {};
			}
		}

		void store(view const& source, stream& outstream, store_format) const noexcept {
			auto png_ptr = png::write_struct::create();
			if (!png_ptr) {
				return;
			}
			try {
				png_ptr.set_write_stream(outstream);
				png_ptr.write_view(source);
			} catch (png::exception const&) {
				return;
			}
		}
	};

	void register_png() { register_factory<png_factory>(); }
}  // namespace img
