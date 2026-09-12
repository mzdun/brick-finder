// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module img.reg;

import std;

namespace img {
	namespace {
		std::vector<image_factory*>& factories() {
			static std::vector<image_factory*> storage{};
			return storage;
		}
	}  // namespace

	void register_factory(image_factory* factory) { factories().push_back(factory); }

	std::unique_ptr<image> load_from_registry(std::FILE* infile) noexcept {
		auto const pos = std::ftell(infile);

		for (auto const factory : factories()) {
			auto const valid = factory->is_valid(infile);
			std::fseek(infile, pos, 0 /*SEEK_SET*/);

			if (!valid) continue;

			auto candidate = factory->load(infile);
			if (candidate) {
				return candidate;
			}

			std::fseek(infile, pos, 0 /*SEEK_SET*/);
		}

		return {};
	}

	std::unique_ptr<image> load_from_registry(std::filesystem::path const& path) noexcept {
		auto const ptr = open(path, false);
		if (!ptr) {
			return {};
		}

		return load_from_registry(ptr.get());
	}

	file_ptr open(std::filesystem::path const& path, bool write) {
#ifdef _WIN32
		FILE* result{nullptr};
		if (_wfopen_s(&result, path.c_str(), write ? L"wb" : L"rb")) {
			result = nullptr;
		}
		return file_ptr{result};
#else
		auto const utf8 = path.u8string();
		return file_ptr{std::fopen(path.c_str(), write ? "wb" : "rb")};
#endif
	}
};  // namespace img
