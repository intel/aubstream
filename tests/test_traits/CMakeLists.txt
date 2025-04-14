#
# Copyright (C) 2022-2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

set(TEST_TRAITS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_traits.h
)

set_property(GLOBAL APPEND PROPERTY TEST_TRAITS_SOURCES_property "${TEST_TRAITS_SOURCES}")

set(AUBCONFIG_PATH "${AUB_STREAM_DIR}/../aub_configs/CMakeLists.txt")
if(EXISTS ${AUBCONFIG_PATH})
  include(${AUBCONFIG_PATH})
else()
  message(WARNING "aub_configs NOT FOUND. Using 1x2x3 as default config.")
  set(DEFAULT_CONFIG 0x100020003)
endif()

function(GENERATE_CONFIG Product)
  if(DEFINED ${Product}_CONFIG_HEX)
    set(VALUE "${${Product}_CONFIG_HEX}")
  else()
    if(DEFINED DEFAULT_CONFIG)
      set(VALUE "${DEFAULT_CONFIG}")
    else()
      message(FATAL_ERROR "${Product} config not defined in aub_configs")
    endif()
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
