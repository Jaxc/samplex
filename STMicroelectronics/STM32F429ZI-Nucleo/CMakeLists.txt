#  Copyright (c) 2026 Eclipse ThreadX contributors
#
#  This program and the accompanying materials are made available
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
#
#  SPDX-License-Identifier: MIT
#
#  Contributors:
#     Ali Eissa - 2026 version.

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)

# Set the toolchain if not defined
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/arm-gcc-cortex-m4.cmake")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(utilities)

# Define the Project
project(stm32f429_threadx C CXX ASM)

# Define ThreadX User Configurations
set(TX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/lib/threadx/tx_user.h" CACHE STRING "Enable TX user configuration")
set(TX_USER_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/threadx")

# Define ThreadX User Configurations
set(NX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/lib/threadx/nx_user.h" CACHE STRING "Enable NX user configuration")
set(NX_USER_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/threadx")

# Define ThreadX User Configurations
set(FX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/lib/threadx/fx_user.h" CACHE STRING "Enable FX user configuration")
set(FX_USER_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/threadx")

# Define ThreadX User Configurations
set(UX_USER_FILE "${CMAKE_CURRENT_LIST_DIR}/lib/threadx/ux_user.h" CACHE STRING "Enable UX user configuration")
set(UX_USER_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/threadx")

# Set up standard paths for find modules
set(STM32_FAMILY "F4")
set(STM32Cube_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/stm32cubef4")

# Find CMSIS and HAL driver packages
find_package(CMSIS REQUIRED)
find_package(STM32HAL REQUIRED COMPONENTS cortex pwr rcc gpio uart dma eth hcd pcd tim ll_usb)
find_package(ST_USBX_MIDDLEWARE REQUIRED)
find_package(ST_FILEX_MIDDLEWARE REQUIRED)

# Compile the STM32F4xx HAL Driver Library as an Object Library
set(HAL_TARGET stm32cubef4)
add_library(${HAL_TARGET} OBJECT ${STM32HAL_SOURCES})

target_compile_definitions(${HAL_TARGET}
    PUBLIC
        STM32F429xx
        USE_HAL_DRIVER
        STM32F4
)

target_include_directories(${HAL_TARGET}
    PUBLIC
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/app
)

# Compile ThreadX Kernel from root shared libs submodule
set(THREADX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../libs/threadx")
add_subdirectory(${THREADX_DIR} threadx)

# Compile NetX Duo Kernel from root shared libs submodule
set(NETX_DUO_DIR "${CMAKE_CURRENT_LIST_DIR}/../../libs/netxduo")
add_subdirectory(${NETX_DUO_DIR} netxduo)

# Compile FileX Kernel from root shared libs submodule
set(FILEX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../libs/filex")
add_subdirectory(${FILEX_DIR} filex)

# Compile UsbX Kernel from root shared libs submodule
set(USBX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../libs/usbx")
add_subdirectory(${USBX_DIR} usbx)

# Compile ST USBX Middleware
set(USBX_MIDDLEWARE_TARGET st-usbx-middlware)
add_library(${USBX_MIDDLEWARE_TARGET} OBJECT ${ST_USBX_MIDDLEWARE_SOURCES})

target_link_libraries(${USBX_MIDDLEWARE_TARGET}
    PRIVATE
        usbx
        stm32cubef4
)

# Compile ST FileX Middleware
set(FILEX_MIDDLEWARE_TARGET st-filex-middlware)
add_library(${FILEX_MIDDLEWARE_TARGET} OBJECT ${ST_FILEX_MIDDLEWARE_SOURCES})

target_link_libraries(${FILEX_MIDDLEWARE_TARGET}
    PRIVATE
        filex
        stm32cubef4
)
target_include_directories(${FILEX_MIDDLEWARE_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/app/filex
)


# Create the Main Executable
set(EXE_TARGET stm32f429_threadx)

add_executable(${EXE_TARGET}
    app/startup/startup_stm32f429xx.s
    app/startup/system_stm32f4xx.c
    app/startup/tx_initialize_low_level.S
    app/board_init.c
    app/console.c
    app/ethernet_phy.c
    app/interrupts.c
    app/main.c
    app/stm32f4xx_hal_msp.c
    app/stm32f4xx_hal_timebase_tim.c
    app/sysmem.c
    app/syscalls.c
    app/USB/usb.c
    app/USB/ux_device_descriptors.c
    app/USB/usb_phy.c
    app/filex/app_filex.c

)

# Set compile definitions for our executable
target_compile_definitions(${EXE_TARGET}
    PRIVATE
        STM32F429xx
        USE_HAL_DRIVER
        STM32F4
)

# Include paths
target_include_directories(${EXE_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/app
        ${CMAKE_CURRENT_LIST_DIR}/app/USB
        ${CMAKE_CURRENT_LIST_DIR}/app/filex
        ${CMSIS_INCLUDE_DIRS}
        ${STM32HAL_INCLUDE_DIR}
        ${ST_USBX_MIDDLEWARE_INCLUDE_DIRS}
        ${TX_USER_FILE_DIR}
        ${NX_USER_FILE_DIR}
        ${FX_USER_FILE_DIR}
        ${UX_USER_FILE_DIR}
)

# Link libraries (includes ThreadX kernel and HAL object libraries)
target_link_libraries(${EXE_TARGET}
    PRIVATE
        threadx
        netxduo
        filex
        usbx
        st-usbx-middlware
        st-filex-middlware
        stm32cubef4
)

# Apply GCC linker script and print memory usage (utilities.cmake function)
set_target_linker(${EXE_TARGET} "${CMAKE_CURRENT_LIST_DIR}/app/startup/STM32F429xx_FLASH.ld")

# Post-build commands to generate raw .bin and .hex files
post_build(${EXE_TARGET})
