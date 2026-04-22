#
# Copyright (C) 2025-2026 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_SUPPORT_XE3P_CORE)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3p_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3p_core.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/command_streamer_helper_xe3p_core_additional.cpp
  )

  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

  foreach(subdir ${SUBDIRS})
    MESSAGE(STATUS "Adding directory: ${subdir}")
    add_subdirectory(${subdir})
  endforeach()
endif()
