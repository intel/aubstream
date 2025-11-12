#
# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_CRI)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_cri.cpp
  )
endif()
