#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

target_sources(${TARGET_NAME}
               PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
               ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_dg2.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/page_table_tests_dg2.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_dg2.cpp
)
