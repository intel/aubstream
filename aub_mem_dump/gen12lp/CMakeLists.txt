#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

target_sources(${PROJECT_NAME}
               PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_gen12lp.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_gen12lp.h
)

target_sources(${PROJECT_NAME}_all_hw
               PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/enable_gen12lp.cpp
)

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

foreach(subdir ${SUBDIRS})
  MESSAGE(STATUS "Adding directory: ${subdir}")
  add_subdirectory(${subdir})
endforeach()
