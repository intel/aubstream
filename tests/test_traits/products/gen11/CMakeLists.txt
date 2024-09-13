#
# Copyright (C) 2022-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_GEN11)
  set(TEST_TRAITS_SOURCES
      ${TEST_TRAITS_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_icllp.cpp
  )

  set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

  GENERATE_CONFIG(ICLLP 0x100080008)
endif()
