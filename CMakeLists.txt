#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
PROJECT(azure-sphere-rtcore-audio C)
# include
include_directories(${CMAKE_SOURCE_DIR} 
					${CMAKE_SOURCE_DIR}/printf
					${CMAKE_SOURCE_DIR}/mt3620_m4_software/MT3620_M4_Driver/HDL/inc 
					${CMAKE_SOURCE_DIR}/mt3620_m4_software/MT3620_M4_Driver/MHAL/inc 
					${CMAKE_SOURCE_DIR}/mt3620_m4_software/MT3620_M4_Sample_Code/BareMetal/OS_HAL/inc
					${CMAKE_SOURCE_DIR}/mt3620_m4_software/MT3620_M4_BSP/CMSIS/include 
					${CMAKE_SOURCE_DIR}/mt3620_m4_software/MT3620_M4_BSP/mt3620/inc)

# macro
add_compile_definitions(OSAI_BARE_METAL)
add_compile_definitions(AzureSphere_CM4)

# Create executable
ADD_EXECUTABLE(${PROJECT_NAME} main.c Log_Debug.c printf/printf.c 
                               mt3620_m4_software/MT3620_M4_Driver/MHAL/src/mhal_osai.c
	                           mt3620_m4_software/MT3620_M4_BSP/mt3620/src/vector_table.c 
							   mt3620_m4_software/MT3620_M4_BSP/mt3620/src/nvic.c 
							   mt3620_m4_software/MT3620_M4_BSP/mt3620/src/cirq_common.c 
							   mt3620_m4_software/MT3620_M4_Driver/HDL/src/hdl_gpio.c 
							   mt3620_m4_software/MT3620_M4_Driver/MHAL/src/mhal_gpio.c 
							   mt3620_m4_software/MT3620_M4_Sample_Code/BareMetal/OS_HAL/src/os_hal_gpio.c
							   mt3620_m4_software/MT3620_M4_Driver/HDL/src/hdl_gpt.c 
							   mt3620_m4_software/MT3620_M4_Driver/MHAL/src/mhal_gpt.c 
							   mt3620_m4_software/MT3620_M4_Sample_Code/BareMetal/OS_HAL/src/os_hal_gpt.c
							   )
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld)

# Add MakeImage post-build command
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")
