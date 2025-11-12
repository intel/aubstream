#
# Copyright (C) 2025 Intel Corporation
#
# SPDX-License-Identifier: MIT
#

if(AUBSTREAM_SUPPORT_CRI)
  target_sources(${PROJECT_NAME}
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
                 ${CMAKE_CURRENT_SOURCE_DIR}/command_streamer_helper_xe3p_core_cri.cpp
  )

  target_sources(${PROJECT_NAME}_all_hw
                 PRIVATE
                 ${CMAKE_CURRENT_SOURCE_DIR}/enable_product_cri.cpp
  )

endif()
