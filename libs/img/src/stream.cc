// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

module;

#ifdef _WIN32
#include <wchar.h>
#endif

module img.reg;

namespace img {
	namespace {
		std::FILE* open_file(std::filesystem::path const& path, stream::mode rw) {
#ifdef _WIN32
			std::FILE* result{nullptr};
			if (_wfopen_s(&result, path.c_str(), rw == stream::mode::write ? L"wb" : L"rb")) {
				result = nullptr;
			}
			return result;
#else
			return std::fopen(path.c_str(), rw == stream::mode::write ? "wb" : "rb");
#endif
		}

		class file_stream : public stream {
		public:
			file_stream(std::filesystem::path const filename, mode rw) : file_{open_file(filename, rw)} {}
			~file_stream() override { std::fclose(file_); }
			bool is_eof() const noexcept override { return !!std::feof(file_); }
			std::size_t read(void* buffer, std::size_t length) override { return std::fread(buffer, 1, length, file_); }
			std::size_t write(void const* buffer, std::size_t length) override {
				return std::fwrite(buffer, 1, length, file_);
			}
			std::size_t tell() const override { return static_cast<std::size_t>(std::max(0l, std::ftell(file_))); }
			void seek(std::size_t pos) override {
				static constexpr auto SEEK_SET = 0;
				static constexpr auto SEEK_CUR = 1;
				static constexpr auto max_seek = static_cast<std::size_t>(std::numeric_limits<long>::max());
				if (pos > max_seek) {
					std::fseek(file_, 0, SEEK_SET);
					while (pos) {
						auto const partial = std::min(pos, max_seek);
						std::fseek(file_, static_cast<long>(partial), SEEK_CUR);
						pos -= partial;
					}
					return;
				}
				std::fseek(file_, static_cast<long>(pos), SEEK_SET);
			}
			void flush() override { std::fflush(file_); }

		private:
			std::FILE* file_;
		};
	}  // namespace

	std::unique_ptr<stream> stream::open(std::filesystem::path const& filename, mode rw) {
		return std::make_unique<file_stream>(filename, rw);
	}
}  // namespace img
