#
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(SUPPORT_PTL)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3_core_ptl.cpp
  )

  target_sources(${PROJECT_NAME}_all_hw
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/enable_product_ptl.cpp
  )

endif()
