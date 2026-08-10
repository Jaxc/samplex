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

message(STATUS "STM32F4xx library check: " ${STM32Cube_DIR})

AUX_SOURCE_DIRECTORY(${STM32Cube_DIR}/Drivers/Middleware/filex_stm32_drivers/ ST_FILEX_MIDDLEWARE_SOURCES)

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ST_FILEX_MIDDLEWARE DEFAULT_MSG ST_FILEX_MIDDLEWARE_SOURCES)