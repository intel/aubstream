#
# Copyright (C) 2022-2023 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_MTL OR TESTS_ARL)
  target_sources(${TARGET_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/gpu_tests_mtl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_stream_helper_tests_mtl.cpp
  )
endif()
