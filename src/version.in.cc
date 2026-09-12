// DO NOT MODIFY, THIS FILE IS GENERATED FROM version.in.cc
//
// @PROJECT_NAME@/@PROJECT_VERSION@@PROJECT_VERSION_STABILITY@
//
// Copyright (c) 2026 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)
// clang-format off

export module version;

export {
	struct version {
		static constexpr char program[] = "@PROJECT_NAME@";
		static constexpr char string[] = "@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@.@PROJECT_VERSION_PATCH@";  // NOLINT build/include_what_you_use and whitespace/line_length
		static constexpr char string_short[] = "@PROJECT_VERSION_MAJOR@.@PROJECT_VERSION_MINOR@";
		static constexpr char stability[] = "@PROJECT_VERSION_STABILITY@";  // or "-alpha.5", "-beta", "-rc.3", "", ...
		static constexpr char build_meta[] = "@PROJECT_VERSION_BUILD_META@";
		static constexpr char ui[] = "@PROJECT_VERSION@@PROJECT_VERSION_STABILITY@@PROJECT_VERSION_BUILD_META@";

		static constexpr unsigned major = @PROJECT_VERSION_MAJOR@;
		static constexpr unsigned minor = @PROJECT_VERSION_MINOR@;
		static constexpr unsigned patch = @PROJECT_VERSION_PATCH@;
	};
}

// clang-format on
