/*
 * Copyright (c) 2026 Eclipse ThreadX contributors
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the MIT license which is available at
 *  https://opensource.org/license/mit.
 *
 *  SPDX-License-Identifier: MIT
 *
 *  Contributors:
 *     Jacob Rosén - Initial version.
 */

/* This files includes public headers for USB */

#ifndef _USB_PHY_H
#define _USB_PHY_H

#include "stm32f4xx_hal.h"

/* Global USB Handle */
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Function Prototypes */
void usb_phy_init(void);

void start_usbx(void);

#endif // _USB_PHY_H
