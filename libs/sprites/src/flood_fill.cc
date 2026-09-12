// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module sprites;

import img;
import std;

namespace sprites {
	class block_color_finder {
	public:
		void add(img::rgba_t current) {
			auto found = false;
			for (auto& [key, value] : histogram) {
				if (key == current) {
					++value;
					found = true;
					continue;
				}
				if (color_matches(key, current)) {
					++value;
				}
			}
			if (!found) ++histogram[current];
		}

		img::rgba_t most_popular() const noexcept {
			std::size_t max{};
			for (auto const& [key, count] : histogram) {
				if (count > max) max = count;
			}
			img::rgba_t R{}, G{}, B{}, counter{};
			for (auto const& [key, count] : histogram) {
				if (count != max) continue;

				R += img::get_red(key);
				G += img::get_green(key);
				B += img::get_blue(key);
				++counter;
			}

			if (counter) {
				R = clamp(R / counter);
				G = clamp(G / counter);
				B = clamp(B / counter);
			}
			return img::to_rgba(R, G, B);
		}

	private:
		std::map<img::rgba_t, std::size_t> histogram{};
	};

	info cut_sprite(img::view const& flooded,
	                img::view const& orig,
	                img::rect const& pos,
	                img::rgba_t marker,
	                img::rgba_t background) {
		info result{};
		auto& [result_pixmap, color] = result;
		auto const mask = flooded.subview(pos);
		result_pixmap = orig.subview(pos).clone();
		auto const sprite = result_pixmap->to_view();

		block_color_finder colors{};

		for (auto y = 0u; y < mask.height; ++y) {
			auto const* mask_row = mask.row_at(y);
			auto row = sprite.row_at(y);
			for (auto x = 0u; x < mask.width; ++x, ++mask_row, ++row) {
				if (*mask_row != marker)
					*row = background;
				else
					colors.add(*row);
			}
		}

		color = colors.most_popular();
		return result;
	}

	struct point {
		unsigned x{};
		unsigned y{};
	};

	img::rect flood_fill(img::view const& pixmap, point orig, img::rgba_t color, img::rgba_t fill, auto&& validator) {
		img::rect result{.left{orig.x}, .top{orig.y}, .right{orig.x}, .bottom{orig.y}};

		std::queue<point> points{};
		points.push(orig);

		while (!points.empty()) {
			auto const [x, y] = points.front();
			points.pop();

			auto& pixel = *pixmap.pixel_at(x, y);
			if (!validator(pixel, color)) continue;

			if (x < result.left) result.left = x;
			if (y < result.top) result.top = y;
			if (x > result.right) result.right = x;
			if (y > result.bottom) result.bottom = y;

			pixel = fill;
			if (x) points.push({.x = x - 1, .y = y});
			if (y) points.push({.x = x, .y = y - 1});
			if (x < (pixmap.width - 1)) points.push({.x = x + 1, .y = y});
			if (y < (pixmap.height - 1)) points.push({.x = x, .y = y + 1});
		}

		return result;
	}

	static constexpr img::rgba_t bom_base = img::to_rgba({.r = 255, .g = 255, .b = 255, .a = 0});

	img::rect fill_bom_area(img::view const& pixmap, unsigned orig_x, unsigned orig_y, img::rgba_t color) {
		return flood_fill(pixmap, {.x = orig_x, .y = orig_y}, color, bom_base, color_matches);
	}

	placement fill_sprite_area(img::view const& copy, unsigned orig_x, unsigned orig_y, img::rgba_t color) {
		return {.pos = flood_fill(copy, {.x = orig_x, .y = orig_y}, 0, color,
		                          [](auto current, auto) { return img::get_alpha(current) == 0xFF; }),
		        .marker = color};
	}

	static inline auto abs_diff(auto a, auto b) noexcept { return a > b ? a - b : b - a; }

	img::rgba_t clr_diff(img::rgba_t lhs, img::rgba_t rhs, unsigned offset) noexcept {
		return abs_diff((lhs >> offset) & 0xff, (rhs >> offset) & 0xff);
	}
}  // namespace sprites
