// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module img.reg;

import img.str;
import std;

namespace img {
	namespace {
		std::vector<image_factory*>& factories() {
			static std::vector<image_factory*> storage{};
			return storage;
		}

		std::unique_ptr<image> load_from_registry(stream& instream, std::size_t pos, image_factory& factory) {
			auto const valid = factory.is_valid(instream);
			instream.seek(pos);

			if (!valid) return {};

			auto candidate = factory.load(instream);
			if (!candidate) {
				instream.seek(pos);
			}

			return candidate;
		}
	}  // namespace

	void register_factory(image_factory* factory) { factories().push_back(factory); }

	std::unique_ptr<image> load_from_registry(stream& instream, std::string_view ext_hint) noexcept {
		auto const pos = instream.tell();

		if (!ext_hint.empty()) {
			for (auto const factory : factories()) {
				auto matching = false;
				for (auto const ext : factory->extensions()) {
					if (ext == ext_hint) {
						matching = true;
						break;
					}
				}

				if (matching) {
					auto candidate = load_from_registry(instream, pos, *factory);
					if (candidate) {
						return candidate;
					}
				}
			}
		}

		for (auto const factory : factories()) {
			auto candidate = load_from_registry(instream, pos, *factory);
			if (candidate) {
				return candidate;
			}
		}

		return {};
	}

	std::unique_ptr<image> load_from_registry(std::filesystem::path const& path) noexcept {
		auto const ptr = stream::open(path, stream::mode::read);
		if (!ptr) {
			return {};
		}

		return load_from_registry(*ptr, as_sv(path.extension().u8string()));
	}

	bool store_using_registry(view const& source,
	                          stream& outstream,
	                          std::string_view ext_hint,
	                          store_format format) noexcept {
		if (!ext_hint.empty()) {
			for (auto const factory : factories()) {
				for (auto const ext : factory->extensions()) {
					if (ext == ext_hint) {
						factory->store(source, outstream, format);
						return true;
					}
				}
			}
			return false;
		}

		for (auto const factory : factories()) {
			factory->store(source, outstream, format);
			return true;
		}

		return false;
	}

	bool store_using_registry(view const& source, std::filesystem::path const& path, store_format format) noexcept {
		auto ptr = stream::open(path, stream::mode::write);
		if (!ptr) {
			return {};
		}

		if (!store_using_registry(source, *ptr, as_sv(path.extension().u8string()), format)) {
			ptr.reset();
			std::error_code ignore{};
			std::filesystem::remove(path, ignore);
			return false;
		}

		return true;
	}
};  // namespace img
