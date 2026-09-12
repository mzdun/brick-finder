// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

export module finder.str;

import std;

export {
	inline std::string_view as_sv(std::u8string_view v) { return {reinterpret_cast<char const*>(v.data()), v.size()}; }
}
