#
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_XE2_HPG_CORE)
  set(TEST_TRAITS_SOURCES
      ${TEST_TRAITS_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_bmg.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_lnl.cpp
  )

  set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

  # 1x2x3: non-zero value for unit tests
  GENERATE_CONFIG(BMG 0x100020003)
  GENERATE_CONFIG(LNL 0x100020003)
endif()
