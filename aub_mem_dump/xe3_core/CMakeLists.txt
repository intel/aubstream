#
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(SUPPORT_XE3_CORE)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3_core.h
  )

  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

  foreach(subdir ${SUBDIRS})
    MESSAGE(STATUS "Adding directory: ${subdir}")
    add_subdirectory(${subdir})
  endforeach()
endif()
