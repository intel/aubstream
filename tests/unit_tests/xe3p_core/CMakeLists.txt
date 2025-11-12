#
# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_XE3P_CORE)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/page_table_tests_xe3p_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_xe3p_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3p_tests.cpp
  )

  aub_stream_add_subdirectories("*")
endif()
