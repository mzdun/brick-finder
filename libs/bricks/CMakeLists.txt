# Copyright (c) 2026 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
)

set(MOD_SRCS
  src/args.cc
  src/run.cc
  src/tool.cc
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${MOD_SRCS})
add_module_library(bricks ${SRCS} EXPORTS ${MOD_SRCS} LINK mbits::args mod-finder)

##################################################################
##  TESTING
##################################################################

if (BF_TESTING)
  set(TEST_SRCS
    tests/test.cc
  )
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${TEST_SRCS})

  add_module_test(bricks ${TEST_SRCS})
endif()
