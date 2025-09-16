#
# Copyright (C) 2022-2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_XE_HPG_CORE)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_xe_hpg_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/hardware_context_xe_hpg_tests.cpp
  )

  aub_stream_add_subdirectories("*")
endif()
