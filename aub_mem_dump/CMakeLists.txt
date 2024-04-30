#
# Copyright (C) 2022-2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_library(${PROJECT_NAME} STATIC
            ${SOURCES}
            ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_file_stream.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_file_stream.h
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_header.h
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_manager_imp.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_manager_imp.h
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_stream.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_stream.h
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_tbx_stream.h
            ${CMAKE_CURRENT_SOURCE_DIR}/aub_shm_stream.h
            ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper.h
            ${CMAKE_CURRENT_SOURCE_DIR}/family_mapper.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/family_mapper.h
            ${CMAKE_CURRENT_SOURCE_DIR}/gpu.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/gpu.h
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware_context_imp.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/hardware_context_imp.h
            ${CMAKE_CURRENT_SOURCE_DIR}/memory_banks.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/memory_banks.h
            ${CMAKE_CURRENT_SOURCE_DIR}/memory_bank_helper.h
            ${CMAKE_CURRENT_SOURCE_DIR}/null_hardware_context.h
            ${CMAKE_CURRENT_SOURCE_DIR}/options.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/options.h
            ${CMAKE_CURRENT_SOURCE_DIR}/page_table.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/page_table.h
            ${CMAKE_CURRENT_SOURCE_DIR}/page_table_walker.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/page_table_walker.h
            ${CMAKE_CURRENT_SOURCE_DIR}/page_table_entry_bits.h
            ${CMAKE_CURRENT_SOURCE_DIR}/page_table_pml5.h
            ${CMAKE_CURRENT_SOURCE_DIR}/physical_address_allocator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/physical_address_allocator.h
            ${CMAKE_CURRENT_SOURCE_DIR}/settings.h
            ${CMAKE_CURRENT_SOURCE_DIR}/settings_reader.h
            ${CMAKE_CURRENT_SOURCE_DIR}/settings_reader.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_proto.h
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_sockets.h
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_sockets_imp.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_sockets_imp.h
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_stream.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_shm_stream.h
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_shm_stream.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tbx_stream.h
            ${CMAKE_CURRENT_SOURCE_DIR}/alloc_tools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/align_helpers.h
            ${CMAKE_CURRENT_SOURCE_DIR}/misc_helpers.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/includes/aub_services.h
            ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/includes/gfx_core_family.h
            ${CMAKE_CURRENT_SOURCE_DIR}/os_interface/os_calls.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/includes/setting_vars.inl
            ${CMAKE_CURRENT_SOURCE_DIR}/includes/setting_vars_base.inl

)

target_include_directories(${PROJECT_NAME} INTERFACE ${INTERFACE_HEADERS_DIRECTORY})

target_include_directories(${PROJECT_NAME}
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/includes
)

# Generate an object library that enables all valid platforms
add_library(${PROJECT_NAME}_all_hw
            OBJECT
            ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)
target_compile_definitions(${PROJECT_NAME}_all_hw PRIVATE ${SUPPORTED_CORE_FLAGS_DEFINITONS})

target_include_directories(${PROJECT_NAME}_all_hw
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/${BRANCH_TYPE}/includes
)

set_target_properties(${PROJECT_NAME}_all_hw PROPERTIES LINKER_LANGUAGE CXX)

macro(subdirlist result curdir dirmask)
  file(GLOB children RELATIVE ${curdir} ${curdir}/${dirmask})
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "gen*")

foreach(subdir ${SUBDIRS})
  MESSAGE(STATUS "Adding directory: ${subdir}")
  add_subdirectory(${subdir})
endforeach()

subdirlist(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR} "*_core")

foreach(subdir ${SUBDIRS})
  MESSAGE(STATUS "Adding directory: ${subdir}")
  add_subdirectory(${subdir})
endforeach()

if(NOT ("${BRANCH_TYPE}" STREQUAL ""))
  add_subdirectory("${BRANCH_TYPE}")
endif()

# Object library target may not link to anything for CMake versions < 3.12
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
  target_link_libraries(${PROJECT_NAME}_all_hw PUBLIC ${PROJECT_NAME})
endif()

aub_stream_create_source_tree(${PROJECT_NAME})
aub_stream_create_source_tree(${PROJECT_NAME}_all_hw)
