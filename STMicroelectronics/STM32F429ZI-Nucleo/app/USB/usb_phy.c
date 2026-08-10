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

#include "main.h"
#include "usb_phy.h"
#include "ux_api.h"
#include "ux_dcd_stm32.h"

typedef struct {
    uint16_t total_length;
    uint16_t rem_length;
    uint16_t max_packet_length;
} usbd_endpoint_t;


typedef struct {
    volatile uint32_t current_state;
    volatile uint32_t restore_state;
    uint32_t remote_wake_armed;
    uint8_t address;
    uint8_t current_config;
    void *dev_data;
    void *class_data;
    usbd_endpoint_t ep0_tx;
    usbd_endpoint_t ep0_rx;
    uint8_t ep0_state;
} usbd_context_t;


static usbd_context_t usbd_ctx = { 0 };
/* Phy related functionality */

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/**
  * @brief  Initialize USB as Full Speed device.
  *
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void);

void usb_phy_init(void)
{
    MX_USB_OTG_FS_PCD_Init();
}


/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{
    hpcd_USB_OTG_FS.pData = &usbd_ctx;

    hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
    hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
    hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
    hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
    hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
    if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
    {
        Error_Handler();
    }
}




void usb_phy_setup_device(void) {
    /* Set up endpoints */
    //usbd_setup_endpoints();


    HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x100);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x100);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x100);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, 0x100);

    ux_dcd_stm32_initialize((ULONG)USB_OTG_FS, (ULONG)&hpcd_USB_OTG_FS);


    //USB_OTG_FS->GINTMSK = 0xFFffFFff;
    //USB_MASK_INTERRUPT(USB_OTG_FS,0xFFffFFff);

    /* Start USB */
    HAL_PCD_Start(&hpcd_USB_OTG_FS);
}

#if 0
static void usbd_setup_endpoints(void)
{
    if (HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, 0x81, 0x40, EP_TYPE_BULK) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, 0x01, 0x40, EP_TYPE_BULK) != HAL_OK)
    {
        Error_Handler();
    }

}

void usbd_setup(usbd_context_t *ctx, usb_setup_packet_t *setup)
{

    ctx->ep0_state = USBD_EP0_SETUP;

    setup->wValue = SWAPBYTE(&(setup->wValue));
    setup->wIndex = SWAPBYTE(&(setup->wIndex));
    setup->wLength = SWAPBYTE(&(setup->wLength));

    switch (setup->bmRequestType.recipient)
    {
    case USB_REQ_RECIPIENT_DEVICE:
        usbd_std_dev_request(ctx, setup);
        break;
    case USB_REQ_RECIPIENT_INTERFACE:
        usbd_std_if_request(ctx, setup);
        break;
    case USB_REQ_RECIPIENT_ENDPOINT:
        usbd_std_ep_request(ctx, setup);
        break;
    default:
        usbd_ep_stall(ctx, setup->bmRequestType.direction ? 0x80 : 0x00);
        break;
    }
}
#endif

void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
