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
	void store_png_image_impl(std::FILE* outfile, view const& source) noexcept;
}  // namespace img

namespace img {
	namespace png {
		struct exception {};

		struct image_info {
			unsigned width;
			unsigned height;
			size_t row_bytes;
			unsigned channels;
		};

		struct read_struct_deleter {
			png_info* attached{};
			bool is_write{false};
			void operator()(png_struct* png_ptr) {
				is_write ? png_destroy_write_struct(&png_ptr, &attached)
				         : png_destroy_read_struct(&png_ptr, &attached, nullptr);
			}
		};

		struct rw_struct : private std::unique_ptr<png_struct, read_struct_deleter> {
			using base = std::unique_ptr<png_struct, read_struct_deleter>;
			using base::operator bool;

			explicit rw_struct(bool is_write)
			    : base{(is_write ? png_create_write_struct : png_create_read_struct)(
			          PNG_LIBPNG_VER_STRING,
			          nullptr,
			          [](auto, auto) { throw png::exception{}; },
			          [](auto, auto) noexcept {})} {
				get_deleter().is_write = is_write;
			}

			static rw_struct create(bool is_write) {
				auto result = rw_struct{is_write};
				if (result) {
					auto& deleter = result.get_deleter();
					deleter.attached = png_create_info_struct(result.get());
					if (!deleter.attached) {
						result.reset();
					}
				}
				return result;
			}
			png_info* info() const noexcept { return get_deleter().attached; }

			image_info read_info(std::FILE* infile) const noexcept {
				png_init_io(get(), infile);
				png_read_info(get(), info()); /* read all PNG info up to image data */

				auto const png_ptr = get();
				auto const info_ptr = info();

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

			void write_info(std::FILE* outfile, view const& source) const noexcept {
				auto const png_ptr = get();
				auto const info_ptr = info();
				png_init_io(png_ptr, outfile);
				png_set_IHDR(png_ptr, info_ptr, source.width, source.height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
				             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
				png_write_info(png_ptr, info_ptr);
			}

			void write_from(view const& source) const noexcept {
				auto const png_ptr = get();
				png_write_image(png_ptr, rows(source).data());
				png_write_end(png_ptr, info());
			}

		private:
			std::vector<png_bytep> rows(view const& image) const noexcept {
				std::vector<png_bytep> result(image.height);
				for (auto y = 0u; y < image.height; ++y) {
					result[y] = image.buffer_at(0, y);
				}
				return result;
			}
		};
	}  // namespace png

	class png_factory : public image_factory {
	public:
		bool is_valid(std::FILE* infile) const noexcept override {
			unsigned char sig[8];
			fread(sig, 1, 8, infile);
			return png_sig_cmp(sig, 0, 8) == 0;
		}
		std::unique_ptr<image> load(std::FILE* infile) const noexcept override {
			auto png_ptr = png::rw_struct::create(false);
			if (!png_ptr) {
				return {};
			}

			try {
				auto const info = png_ptr.read_info(infile);

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
	};

	void register_png() { register_factory<png_factory>(); }

	void store_png_image_impl(std::FILE* outfile, view const& source) noexcept {
		auto png_ptr = png::rw_struct::create(true);
		if (!png_ptr) {
			return;
		}
		try {
			png_ptr.write_info(outfile, source);
			png_ptr.write_from(source);
		} catch (png::exception const&) {
			return;
		}
	}
}  // namespace img
