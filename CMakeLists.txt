#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
PROJECT(azure-sphere-rtcore-audio C)

# include
include_directories(${CMAKE_SOURCE_DIR} 
					${CMAKE_SOURCE_DIR}/printf
					${CMAKE_SOURCE_DIR}/MT3620_M4_Driver/HDL/inc ${CMAKE_SOURCE_DIR}/MT3620_M4_Driver/MHAL/inc ${CMAKE_SOURCE_DIR}/MT3620_M4_Driver/OS_HAL/inc
					${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/CMSIS/include ${CMAKE_SOURCE_DIR}/MT3620_M4_BSP/mt3620/inc)

# macro
add_compile_definitions(OSAI_BARE_METAL)
add_compile_definitions(AzureSphere_CM4)


# Create executable
ADD_EXECUTABLE(${PROJECT_NAME} main.c Log_Debug.c printf/printf.c MT3620_M4_Driver/MHAL/src/mhal_osai.c
	                           MT3620_M4_BSP/mt3620/src/vector_table.c MT3620_M4_BSP/mt3620/src/nvic.c MT3620_M4_BSP/mt3620/src/cirq_common.c 
							   MT3620_M4_Driver/HDL/src/hdl_gpio.c MT3620_M4_Driver/MHAL/src/mhal_gpio.c MT3620_M4_Driver/OS_HAL/src/os_hal_gpio.c
							   MT3620_M4_Driver/HDL/src/hdl_gpt.c MT3620_M4_Driver/MHAL/src/mhal_gpt.c MT3620_M4_Driver/OS_HAL/src/os_hal_gpt.c
							   )
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)

# Add MakeImage post-build command
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")
