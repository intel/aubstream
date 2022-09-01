#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TEST_TRAITS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)

set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

if(NOT ("${BRANCH_TYPE}" STREQUAL ""))
  subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "${BRANCH_TYPE}")

  foreach(subdir ${SUBDIRS})
    add_subdirectory(${subdir})
  endforeach()
endif()

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "gen*")

foreach(subdir ${SUBDIRS})
  add_subdirectory(${subdir})
endforeach()
subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*_core")

foreach(subdir ${SUBDIRS})
  add_subdirectory(${subdir})
endforeach()