// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module;

#include <args/parser.hpp>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

export module bricks.run;

import bricks.tool;
import std;

#ifdef _WIN32
static std::string ut8f_str(wchar_t const* arg) {
	if (!arg) return {};

	auto size = WideCharToMultiByte(CP_UTF8, 0, arg, -1, nullptr, 0, nullptr, nullptr);
	auto out = std::make_unique<char[]>(size + 1);
	WideCharToMultiByte(CP_UTF8, 0, arg, -1, out.get(), size + 1, nullptr, nullptr);
	return {out.get(), static_cast<std::size_t>(size)};
}

std::vector<std::string> wide_char_to_utf8(int argc, wchar_t* argv[]) {
	std::vector<std::string> result{};
	result.resize(argc);
	std::transform(argv, argv + argc, result.begin(), ut8f_str);
	return result;
}

export int run(int argc, wchar_t* argv[]) {
	auto utf8 = wide_char_to_utf8(argc, argv);
	std::vector<char*> args{};
	args.resize(utf8.size() + 1);
	std::transform(utf8.begin(), utf8.end(), args.begin(), [](auto& s) { return s.data(); });
	args[argc] = nullptr;

	SetConsoleOutputCP(CP_UTF8);

	return bricks::tool(::args::from_main(static_cast<int>(args.size() - 1), args.data()));
}
#else
export int run(int argc, char* argv[]) { return bricks::tool(args::from_main(argc, argv)); }
#endif
