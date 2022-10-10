#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_XE_HP_CORE)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_xe_hp_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_xe_hp_core.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/hardware_context_tests_xe_hp_core.cpp
  )

  aub_stream_add_subdirectories("*")
endif()
