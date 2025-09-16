#
# Copyright (C) 2022-2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_XE_HPC_CORE)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_xe_hpc_core_tests.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/page_table_tests_xe_hpc_core.cpp
  )

  aub_stream_add_subdirectories("*")
endif()
