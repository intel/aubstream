#
# Copyright (C) 2022-2025 Intel Corporation
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

  GENERATE_CONFIG(ADLS)
  GENERATE_CONFIG(DG1)
  GENERATE_CONFIG(TGLLP)
  GENERATE_CONFIG(ADLN)
  GENERATE_CONFIG(ADLP)
endif()
