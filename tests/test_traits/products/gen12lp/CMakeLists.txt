#
# Copyright (C) 2022-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(TESTS_GEN12LP)
  set(TEST_TRAITS_SOURCES
      ${TEST_TRAITS_SOURCES}
      ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_adln.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_adlp.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_adls.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_dg1.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_tgllp.cpp
  )

  set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

  GENERATE_CONFIG(ADLS 0x100060010)
  GENERATE_CONFIG(DG1 0x100060010)
  GENERATE_CONFIG(TGLLP 0x100060010)
  # 1x2x3: non-zero value for unit tests
  GENERATE_CONFIG(ADLN 0x10002003)
  GENERATE_CONFIG(ADLP 0x10002003)
endif()
