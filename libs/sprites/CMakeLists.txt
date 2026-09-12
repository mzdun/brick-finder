# Copyright (c) 2026 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
  src/db.cc
  src/flood_fill.cc
)

set(MOD_SRCS
  src/sprites.cc
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS} ${MOD_SRCS})
add_module_library(sprites ${SRCS} EXPORTS ${MOD_SRCS} LINK mod-img)

##################################################################
##  TESTING
##################################################################

if (BF_TESTING)
  set(TEST_SRCS
    tests/test.cc
  )
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/tests FILES ${TEST_SRCS})

  add_module_test(sprites ${TEST_SRCS})
endif()
