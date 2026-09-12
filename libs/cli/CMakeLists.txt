# Copyright (c) 2026 Marcin Zdun
# This file is licensed under MIT license (see LICENSE for details)

set(SRCS
  src/main.cc
)


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
add_executable(${PROJECT_NAME} ${SRCS})

add_win32_icon(${PROJECT_NAME} "appicon.ico")
target_compile_options(${PROJECT_NAME} PRIVATE ${BF_ADDITIONAL_COMPILE_FLAGS})
target_link_options(${PROJECT_NAME} PUBLIC ${BF_ADDITIONAL_LINK_FLAGS})
target_link_libraries(${PROJECT_NAME} mod-bricks)
set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE /ENTRY:wmainCRTStartup)
endif()

##################################################################
##  INSTALL
##################################################################

if (BF_INSTALL)
  install(TARGETS ${PROJECT_NAME})
endif()
