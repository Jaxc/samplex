#  Copyright (c) Microsoft
#  Copyright (c) 2026 Eclipse ThreadX contributors
#
#  This program and the accompanying materials are made available
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
#
#  SPDX-License-Identifier: MIT
#
#  Contributors:
#     Microsoft         - Initial version
#     Frédéric Desbiens - 2024 version.
#     Ali Eissa         - 2026 version.

set(ST_USBX_MIDDLEWARE_DEVICE_HEADERS
    ux_dcd_stm32.h
)

set(ST_USBX_MIDDLEWARE_HOST_HEADERS
    ux_hcd_stm32.h
)

message(STATUS "STM32F4xx library check: " ${STM32Cube_DIR})

find_path(ST_USBX_MIDDLEWARE_DEVICE_INCLUDE_DIR ${ST_USBX_MIDDLEWARE_DEVICE_HEADERS}
    HINTS ${STM32Cube_DIR}/Drivers/Middleware/usbx_stm32_device_controllers/
    CMAKE_FIND_ROOT_PATH_BOTH
)

find_path(ST_USBX_MIDDLEWARE_HOST_INCLUDE_DIR ${ST_USBX_MIDDLEWARE_HOST_HEADERS}
    HINTS ${STM32Cube_DIR}/Drivers/Middleware/usbx_stm32_host_controllers/
    CMAKE_FIND_ROOT_PATH_BOTH
)


set(ST_USBX_MIDDLEWARE_INCLUDE_DIRS
    ${ST_USBX_MIDDLEWARE_DEVICE_INCLUDE_DIR}
    ${ST_USBX_MIDDLEWARE_HOST_INCLUDE_DIR}
)

AUX_SOURCE_DIRECTORY(${STM32Cube_DIR}/Drivers/Middleware/usbx_stm32_device_controllers/ ST_USBX_MIDDLEWARE_DEVICE_SOURCES)
AUX_SOURCE_DIRECTORY(${STM32Cube_DIR}/Drivers/Middleware/usbx_stm32_host_controllers/ ST_USBX_MIDDLEWARE_HOST_SOURCES)

set(ST_USBX_MIDDLEWARE_SOURCES
    ${ST_USBX_MIDDLEWARE_DEVICE_SOURCES}
    ${ST_USBX_MIDDLEWARE_HOST_SOURCES}
)

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ST_USBX_MIDDLEWARE DEFAULT_MSG ST_USBX_MIDDLEWARE_INCLUDE_DIRS ST_USBX_MIDDLEWARE_SOURCES)