#
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_XE3_CORE)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/page_table_tests_xe3_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_xe3_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/hardware_context_xe3_tests.cpp
  )

  aub_stream_add_subdirectories("*")
endif()
