#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TEST_TRAITS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits_bdw.cpp
)

set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

GENERATE_CONFIG(BDW 0x100030008)