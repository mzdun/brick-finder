// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module sprites;

import img;
import std;

export namespace sprites {
	constexpr img::rgba_t max_rgb_error = 30u;

	struct info {
		img::image_ptr pixels{};
		img::rgba_t color{};

		std::string identifier(std::size_t ref, int digits, std::string_view ext = {}) const {
			auto const R = img::get_red(color) >> 4;
			auto const G = img::get_green(color) >> 4;
			auto const B = img::get_blue(color) >> 4;
			return std::format("{:x}{:x}{:x}-{:0{}}{}", R, G, B, ref, digits, ext);
		}

		void store(std::filesystem::path const& filename) { img::store_png_image(pixels->to_view(), filename); }
	};

	struct placement {
		img::rect pos;
		img::rgba_t marker;
	};

	struct record {
		info key{};
		std::set<std::size_t> pages{};
	};

	struct sorter {
		unsigned value{};       // sort darker blocks before lighter
		unsigned saturation{};  // sort dimmer blocks before colorful
		unsigned hue{};         // sort in roygbiv order
		unsigned width{};
		unsigned height{};
		std::size_t original_pos{};

		constexpr auto operator<=>(sorter const&) const noexcept = default;

		static sorter from(info const& key, std::size_t original_pos) {
			auto const [hue, saturation, value] = conv(key.color);
			auto const view = key.pixels->to_view();
			return {
			    .value = value,
			    .saturation = saturation,
			    .hue = hue,
			    .width = view.width,
			    .height = view.height,
			    .original_pos = original_pos,
			};
		}

	private:
		struct HSV {
			unsigned h{};
			unsigned s{};
			unsigned v{};
		};

		static unsigned percent(double d) { return static_cast<unsigned>((d * 100.0) + .5); }

		static HSV conv(img::rgba_t color) {
			// scale down to 16 values per channel
			auto const R = img::get_red(color) & 0xf0;
			auto const G = img::get_green(color) & 0xf0;
			auto const B = img::get_blue(color) & 0xf0;

			auto const r = static_cast<double>(R) / 255.0;
			auto const g = static_cast<double>(G) / 255.0;
			auto const b = static_cast<double>(B) / 255.0;

			auto min_val = std::min({r, g, b});
			auto max_val = std::max({r, g, b});
			auto delta = max_val - min_val;

			HSV result{};
			result.v = percent(max_val);

			if (delta < 0.00001) {
				result.s = 0;
				result.h = 0;  // Undefined, usually 0
				return result;
			}

			if (max_val > 0.0) {
				result.s = percent(delta / max_val);  // Saturation
			} else {
				result.s = 0;
				result.h = 0;
				return result;
			}

			double h{};
			if (r == max_val) {
				h = (g - b) / delta;  // Between yellow & magenta
			} else if (g == max_val) {
				h = 2.0 + (b - r) / delta;  // Between cyan & yellow
			} else {
				h = 4.0 + (r - g) / delta;  // Between magenta & cyan
			}

			h *= 60.0;  // Convert to degrees [0, 360]
			if (h < 0.0) {
				h += 360.0;
			}
			result.h = static_cast<unsigned>(h + .5);

			return result;
		}
	};

	void add_sprite_infos(std::vector<info>& result, std::vector<info>&& sprites);
	void add_sprite_infos(std::vector<record>& result, std::vector<info>&& sprites, std::size_t page_id);

	info cut_sprite(img::view const& flooded,
	                img::view const& orig,
	                img::rect const& pos,
	                img::rgba_t marker,
	                img::rgba_t background);
	img::rect fill_bom_area(img::view const& surface, unsigned orig_x, unsigned orig_y, img::rgba_t color);
	placement fill_sprite_area(img::view const& copy, unsigned orig_x, unsigned orig_y, img::rgba_t color);

	img::rgba_t clr_diff(img::rgba_t lhs, img::rgba_t rhs, unsigned offset) noexcept;

	inline img::rgba_t clr_diff(img::rgba_t lhs, img::rgba_t rhs) noexcept {
		return clr_diff(lhs, rhs, img::red_offset) + clr_diff(lhs, rhs, img::green_offset) +
		       clr_diff(lhs, rhs, img::blue_offset);
	}

	inline img::rgba_t clamp(img::rgba_t value) { return std::min<img::rgba_t>(255, value); }

	inline bool color_matches(img::rgba_t lhs, img::rgba_t rhs) {
		if (lhs == rhs) return true;
		return clr_diff(lhs, rhs) <= max_rgb_error;
	}
}  // namespace sprites
