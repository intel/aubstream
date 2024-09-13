#
# Copyright (C) 2022-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(SUPPORT_XE_HPC_CORE)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe_hpc_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe_hpc_core.h
  )

  target_sources(${PROJECT_NAME}_all_hw
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/enable_xe_hpc_core.cpp
  )

  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

  foreach(subdir ${SUBDIRS})
    MESSAGE(STATUS "Adding directory: xe_hpc_core.${subdir}")
    add_subdirectory(${subdir})
  endforeach()
endif()
