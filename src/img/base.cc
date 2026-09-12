// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module img.base;

import std;

export namespace img {
	class image;
	using image_ptr = std::unique_ptr<image>;
	constexpr auto bytes_per_pixel = 4;
	using rgba_t = std::uint32_t;

	template <unsigned big_endian_offset, unsigned little_endian_offset>
	consteval unsigned channel_offset() noexcept {
		if constexpr (std::endian::native == std::endian::big) {
			return big_endian_offset;
		} else if constexpr (std::endian::native == std::endian::little) {
			return little_endian_offset;
		} else {
			throw "this platform has neither little nor big endian arch";
		}
	}

	constexpr auto red_offset = channel_offset<24, 0>();
	constexpr auto green_offset = channel_offset<16, 8>();
	constexpr auto blue_offset = channel_offset<8, 16>();
	constexpr auto alpha_offset = channel_offset<0, 24>();

	struct rgb {
		unsigned char r{};
		unsigned char g{};
		unsigned char b{};
		unsigned char a{0xFF};

		constexpr auto operator<=>(rgb const&) const noexcept = default;
	};

	inline constexpr auto to_rgba(rgb const& clr) noexcept {
		return (static_cast<rgba_t>(clr.r) << red_offset) | (static_cast<rgba_t>(clr.g) << green_offset) |
		       (static_cast<rgba_t>(clr.b) << blue_offset) | (static_cast<rgba_t>(clr.a) << alpha_offset);
	}

	template <std::integral T>
	inline constexpr auto to_rgba(T R, T G, T B, T A = static_cast<T>(0xFF)) noexcept {
		return to_rgba({
		    .r = static_cast<unsigned char>(R),
		    .g = static_cast<unsigned char>(G),
		    .b = static_cast<unsigned char>(B),
		    .a = static_cast<unsigned char>(A),
		});
	}

	inline constexpr auto get_red(rgba_t clr) noexcept {
		return static_cast<unsigned char>((clr >> red_offset) & 0xFF);
	}

	inline constexpr auto get_green(rgba_t clr) noexcept {
		return static_cast<unsigned char>((clr >> green_offset) & 0xFF);
	}

	inline constexpr auto get_blue(rgba_t clr) noexcept {
		return static_cast<unsigned char>((clr >> blue_offset) & 0xFF);
	}

	inline constexpr auto get_alpha(rgba_t clr) noexcept {
		return static_cast<unsigned char>((clr >> alpha_offset) & 0xFF);
	}

	inline constexpr auto to_rgb(rgba_t clr) noexcept {
		return rgb{
		    .r = get_red(clr),
		    .g = get_green(clr),
		    .b = get_blue(clr),
		    .a = get_alpha(clr),
		};
	}

	struct rect {
		unsigned left{};
		unsigned top{};
		unsigned right{};
		unsigned bottom{};

		constexpr auto operator<=>(rect const&) const noexcept = default;
		constexpr auto width() const noexcept { return right - left; }
		constexpr auto height() const noexcept { return bottom - top; }

		constexpr bool contains(unsigned x, unsigned y) const noexcept {
			return x >= left && x <= right && y >= top && y <= bottom;
		}
	};

	struct view {
		unsigned width{};
		unsigned height{};
		std::size_t pitch{};
		std::size_t width_in_bytes{width * bytes_per_pixel};
		unsigned char* hidden_buffer{};

		bool empty() const noexcept { return !width || !height; }
		rgba_t* row_at(std::size_t y) const noexcept { return reinterpret_cast<rgba_t*>(hidden_buffer + y * pitch); }
		rgba_t* pixel_at(std::size_t x, std::size_t y) const noexcept { return row_at(y) + x; }
		unsigned char* buffer_at(std::size_t x, std::size_t y) const noexcept {
			return reinterpret_cast<unsigned char*>(pixel_at(x, y));
		}
		view subview(int x, int y, int new_width, int new_height) const noexcept;
		view subview(int x, int y, unsigned new_width, unsigned new_height) const noexcept {
			return subview(x, y, static_cast<int>(new_width), static_cast<int>(new_height));
		}
		view subview(rect const& r) const noexcept {
			static constexpr auto as_int = [](std::integral auto i) { return static_cast<int>(i); };
			return subview(as_int(r.left), as_int(r.top), as_int(r.width()), as_int(r.height()));
		}
		image_ptr clone() const;
		void copy_from(int x, int y, view const& src);
	};

	class image {
	public:
		static image_ptr create(unsigned width, unsigned height, std::size_t pitch = 0) {
			return std::unique_ptr<image>{new image(width, height, pitch)};
		}
		~image() = default;

		view to_view() const noexcept {
			return {
			    .width = width_,
			    .height = height_,
			    .pitch = pitch_,
			    .hidden_buffer = buffer_.get(),
			};
		}

	private:
		explicit image(unsigned width, unsigned height, std::size_t pitch)
		    : width_{width}
		    , height_{height}
		    , pitch_{std::max(pitch, static_cast<std::size_t>(width_) * bytes_per_pixel)} {}
		std::size_t area() const noexcept { return static_cast<std::size_t>(height_) * pitch_; }

		unsigned width_{};
		unsigned height_{};
		std::size_t pitch_{};
		std::unique_ptr<unsigned char[]> buffer_{std::make_unique_for_overwrite<unsigned char[]>(area())};
	};
}  // namespace img
