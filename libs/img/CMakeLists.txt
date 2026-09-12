# Copyright (c) 2026 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
  src/base-impl.cc
  src/base-impl-copy.cc
  src/img-impl.cc
  src/png.cc
  src/reg-impl.cc
  src/stream.cc
)

set(MOD_SRCS
  src/base.cc
  src/img.cc
  src/png.cc
  src/reg.cc
  src/str.cc
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${MOD_SRCS})
add_module_library(img ${SRCS} EXPORTS ${MOD_SRCS} LINK PNG::PNG)

##################################################################
##  TESTING
##################################################################

if (BF_TESTING)
  set(TEST_SRCS
    tests/test.cc
  )
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${TEST_SRCS})

  add_module_test(img ${TEST_SRCS})
endif()
