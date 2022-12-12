#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TEST_TRAITS_SOURCES
    ${TEST_TRAITS_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_dg2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_mtl.cpp
)

set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

# 1x2x3: non-zero value for unit tests
GENERATE_CONFIG(DG2 0x10002003)
GENERATE_CONFIG(MTL 0x10002003)