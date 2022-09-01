#
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TEST_TRAITS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits.h
)

set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

function(GENERATE_CONFIG Product DefaultConfig)
  set(VALUE "${DefaultConfig}")

  if(DEFINED AUB_STREAM_${Product}_CONFIG)
    set(VALUE "${AUB_STREAM_${Product}_CONFIG}")
  endif()

  get_property(HasProperty GLOBAL PROPERTY TEST_TRAITS_MACROS_property)
  string(FIND "${HasProperty}" "${Product}_CONFIG" ConfigFound)

  if(ConfigFound EQUAL -1)
    set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_MACROS_property "${Product}_CONFIG=${VALUE}")
  endif()
endfunction()

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*")

foreach(subdir ${SUBDIRS})
  add_subdirectory(${subdir})
endforeach()

