#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe_hpg_core.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe_hpg_core.h
)

target_sources(${PROJECT_NAME}_all_hw
               PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/enable_xe_hpg_core.cpp
)

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

foreach(subdir ${SUBDIRS})
  MESSAGE(STATUS "Adding directory: ${subdir}")
  add_subdirectory(${subdir})
endforeach()
