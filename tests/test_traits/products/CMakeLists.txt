#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TEST_TRAITS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)

set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

foreach(subdir ${SUBDIRS})
  add_subdirectory(${subdir})
endforeach()