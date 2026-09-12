# Copyright (c) 2026 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

cmake_minimum_required (VERSION 4.0)

if (CMAKE_GENERATOR MATCHES "^Visual Studio")
set(VS_GEN ON)
else()
set(VS_GEN OFF)
endif()

if (CMAKE_VERSION VERSION_LESS 4.0.3)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
elseif(CMAKE_VERSION VERSION_LESS 4.3.0)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
elseif(CMAKE_VERSION VERSION_LESS 4.4.0)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
elseif(CMAKE_VERSION VERSION_LESS 4.5.0)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
endif()

project (brick-finder
  VERSION 0.1.0
  DESCRIPTION "Brick Finder"
  LANGUAGES CXX)

set(PROJECT_VERSION_SHORT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set(PROJECT_VERSION_STABILITY "" # or "-alpha", or "-beta", or "-rc.1", or "-rc.2"
    CACHE STRING "" FORCE)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
  message(FATAL_ERROR "Building in source directory is not supported")
endif()

set(BF_TESTING ON CACHE BOOL "Compile and/or run self-tests")
set(BF_INSTALL ON CACHE BOOL "Install the application")
set(BF_SANITIZE OFF CACHE BOOL "Compile with sanitizers enabled")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT VS_GEN)
set(CMAKE_CXX_MODULE_STD 1)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_BINARY_DIR}/conan" "${PROJECT_SOURCE_DIR}/.flow/cmake")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_BINARY_DIR}/conan")

include(common)
include(output_dirs_setup)

set(CONAN_CMAKE_SILENT_OUTPUT ON)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(fmt REQUIRED)
find_package(GTest REQUIRED)
find_package(mbits-args REQUIRED)
find_package(PNG REQUIRED)

if (BF_TESTING)
  enable_testing()

  find_package(GTest REQUIRED)

  set(TEST_REPORT_DIR "${PROJECT_BINARY_DIR}/test-results")
  set(TEST_SANITIZER_TAG "")
  if (BF_SANITIZE)
    set(TEST_SANITIZER_TAG "-sanitizer")
  endif()
  set(TEST_REPORT_FILE "${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}${TEST_SANITIZER_TAG}.xml")
endif()

include(output_dirs_setup)
old_conan_output_dirs_setup()

# See <https://github.com/lefticus/cppbestpractices/blob/v1.0.0/02-Use_the_Tools_Available.md#compilers>

if (MSVC)
  set(BF_ADDITIONAL_COMPILE_FLAGS
    /D_CRT_SECURE_NO_WARNINGS
    /D_CRT_NONSTDC_NO_WARNINGS
    /utf-8
    /permissive-
    /Zc:__cplusplus
    /W4
    /w14242
    /w14254
    /w14263
    /w14265
    /w14287
    /we4289
    /w14296
    /w14311
    /w14545
    /w14546
    /w14547
    /w14549
    /w14555
    /w14619
    /w14640
    /w14826
    /w14905
    /w14906
    /w14928
    /w14946)
  if (BF_SANITIZE)
    list(APPEND BF_ADDITIONAL_COMPILE_FLAGS
      /fsanitize=address
      /D_DISABLE_VECTOR_ANNOTATION
      /D_DISABLE_STRING_ANNOTATION
    )
  endif()
else()
  set(BF_ADDITIONAL_COMPILE_FLAGS
      -Wall -Wextra
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
  )
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND BF_ADDITIONAL_COMPILE_FLAGS
      -fcolor-diagnostics
      -Wno-braced-scalar-init
    # -Wlifetime
    )
  else()
    list(APPEND BF_ADDITIONAL_COMPILE_FLAGS
      -fdiagnostics-color
      -Wmisleading-indentation
      -Wduplicated-cond
      -Wduplicated-branches
      -Wlogical-op
      -Wuseless-cast
      )
  endif()
  if (BF_SANITIZE)
    list(APPEND BF_ADDITIONAL_COMPILE_FLAGS
      -fsanitize=address
      -fsanitize=undefined

    )
    list(APPEND BF_ADDITIONAL_LINK_FLAGS
      -fsanitize=address
      -fsanitize=undefined
    )
  endif()
endif()

include(${PROJECT_SOURCE_DIR}/.flow/packages/config.cmake)
include(CPack)

configure_file(src/version.in.cc src/version.cc @ONLY)

set(SRCS
  src/main.cc
  src/finder/pool.private.cc
  src/finder/task.private.cc
  src/img/base-impl.cc
  src/img/img-impl.cc
  src/img/png.cc
  src/img/reg-impl.cc
  src/sprites/db.cc
  src/sprites/flood_fill.cc
)

set(MOD_SRCS
  src/bricks/args.cc
  src/bricks/run.cc
  src/bricks/tool.cc
  src/finder/finder.cc
  src/finder/pool.cc
  src/finder/str.cc
  src/finder/task.cc
  src/img/base.cc
  src/img/img.cc
  src/img/png.cc
  src/img/reg.cc
  src/sprites/sprites.cc
)

set(MOD_GEN_SRCS
  "${CMAKE_CURRENT_BINARY_DIR}/src/version.cc"
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${MOD_SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES ${MOD_GEN_SRCS} PREFIX gen)

if (VS_GEN)
add_executable(${PROJECT_NAME} ${SRCS} ${MOD_SRCS} ${MOD_GEN_SRCS})
else()
add_executable(${PROJECT_NAME} ${SRCS})
target_sources(${PROJECT_NAME} PRIVATE FILE_SET CXX_MODULES FILES ${MOD_SRCS} ${MOD_GEN_SRCS})
endif()

add_win32_icon(${PROJECT_NAME} "appicon.ico")
target_compile_options(${PROJECT_NAME} PRIVATE ${BF_ADDITIONAL_COMPILE_FLAGS})
target_link_options(${PROJECT_NAME} PUBLIC ${BF_ADDITIONAL_LINK_FLAGS})
target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(${PROJECT_NAME} fmt::fmt mbits::args PNG::PNG)
set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE /ENTRY:wmainCRTStartup)
    target_compile_options(${PROJECT_NAME} PRIVATE /D_UNICODE /DUNICODE)
endif()

##################################################################
##  INSTALL
##################################################################

if (BF_INSTALL)
  install(TARGETS ${PROJECT_NAME})
endif()

##################################################################
##  TESTING
##################################################################

if (BF_TESTING)
  file(GLOB EXCLUDES_TEST_SRCS_CC tests/*.cc)
  file(GLOB EXCLUDES_TEST_SRCS_CPP tests/*.cpp)
  file(GLOB EXCLUDES_TEST_SRCS_CXX tests/*.cxx)
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES
    ${EXCLUDES_TEST_SRCS_CC}
    ${EXCLUDES_TEST_SRCS_CPP}
    ${EXCLUDES_TEST_SRCS_CXX})

  add_project_test(${PROJECT_NAME} ${EXCLUDES_TEST_SRCS_CC} ${EXCLUDES_TEST_SRCS_CPP} ${EXCLUDES_TEST_SRCS_CXX})
  target_link_libraries(${PROJECT_NAME}-test PUBLIC GTest::gmock_main fmt::fmt)

  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}-test "--gtest_output=xml:${TEST_REPORT_DIR}/${PROJECT_NAME}/${TEST_REPORT_FILE}")
endif()
