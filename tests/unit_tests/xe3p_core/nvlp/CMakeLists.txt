#
# Copyright (C) 2026 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_NVLP)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_nvlp.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_nvlp.cpp
  )
endif()
