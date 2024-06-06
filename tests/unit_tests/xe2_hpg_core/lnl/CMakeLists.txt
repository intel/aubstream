#
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_LNL)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_lnl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_lnl.cpp
  )
endif()
