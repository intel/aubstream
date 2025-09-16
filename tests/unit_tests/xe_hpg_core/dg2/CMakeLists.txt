#
# Copyright (C) 2022-2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_DG2)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_dg2.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/page_table_tests_dg2.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_dg2.cpp
  )
endif()
