#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(SUPPORT_GEN8)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_gen8.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_gen8.h
  )

  target_sources(${PROJECT_NAME}_all_hw
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/enable_gen8.cpp
  )

  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

  foreach(subdir ${SUBDIRS})
    MESSAGE(STATUS "Adding directory: ${subdir}")
    add_subdirectory(${subdir})
  endforeach()
endif()
