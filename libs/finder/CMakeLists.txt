# Copyright (c) 2026 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

configure_file(src/version.in.cc src/version.cc @ONLY)

set(SRCS
  src/pool.private.cc
  src/task.private.cc
)

set(MOD_SRCS
  src/finder.cc
  src/pool.cc
  src/task.cc
)

set(MOD_GEN_SRCS
  "${CMAKE_CURRENT_BINARY_DIR}/src/version.cc"
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${MOD_SRCS})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES ${MOD_GEN_SRCS} PREFIX gen)

add_module_library(finder ${SRCS} EXPORTS ${MOD_SRCS} ${MOD_GEN_SRCS} LINK mod-sprites)

##################################################################
##  TESTING
##################################################################

if (BF_TESTING)
  set(TEST_SRCS
    tests/test.cc
  )
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${TEST_SRCS})

  add_module_test(finder ${TEST_SRCS})
endif()
