#
# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_XE3P_CORE)
  set(TEST_TRAITS_SOURCES
      ${TEST_TRAITS_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_cri.cpp
  )

  set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")
  aub_stream_add_subdirectories("*")

  GENERATE_CONFIG(CRI)
endif()
