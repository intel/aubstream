#
# Copyright (C) 2024-2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_TESTS_XE2_HPG_CORE)
  set(TEST_TRAITS_SOURCES
      ${TEST_TRAITS_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_bmg.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_lnl.cpp
  )

  set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

  GENERATE_CONFIG(BMG)
  GENERATE_CONFIG(LNL)
endif()
