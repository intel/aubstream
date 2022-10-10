#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_DG1)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_dg1.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_dg1.cpp
  )
endif()
