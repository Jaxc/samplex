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

 /* This file contains all USB related SW, to separate different SW and make
  * the sample SW easier to follow. */
#include "usb.h"
#include "board_init.h"
#include "tx_api.h"
#include "usb_phy.h"
#include "ux_api.h"
#include "ux_device_class_storage.h"
#include "ux_device_descriptors.h"



static ULONG storage_interface_number;
static ULONG storage_configuration_number;

#define RAM_STORE_SIZE              (64*1024) /* 64k was the smallest disk I managed to format in Linux */
#define RAM_STORE_BLOCK_LEN         (512u) /* 512 seems to be the smallest number supported by Linux*/
#define N_LBA                       (RAM_STORE_SIZE/RAM_STORE_BLOCK_LEN - 1)
uint8_t ram_store[RAM_STORE_SIZE];

/* ThreadX/USBX related functionality. */

void usb_device_thread_entry(ULONG thread_input);

/* USBX device thread*/
#define USB_DEVICE_THREAD_STACK_SIZE UX_THREAD_STACK_SIZE
TX_THREAD usb_device_thread;
CHAR usb_device_thread_stack[USB_DEVICE_THREAD_STACK_SIZE];

/* USB data */
#define USB_MEMORY_SIZE (16*1024)
CHAR usb_buffer[USB_MEMORY_SIZE];

/* Storeage callbacks*/
UINT media_read(
    VOID *storage,
    ULONG lun,
    UCHAR *data_pointer,
    ULONG number_blocks,
    ULONG lba,
    ULONG *media_status);

UINT media_write(
    VOID *storage,
    ULONG lun,
    UCHAR *data_pointer,
    ULONG number_blocks,
    ULONG lba,
    ULONG *media_status);

UINT media_status(
    VOID *storage,
    ULONG lun,
    ULONG media_id,
    ULONG *media_status);

UINT media_flush (VOID *storage, ULONG lun, ULONG number_blocks, ULONG lba, ULONG *media_status);

void  tx_demo_thread_device_simulation_entry(ULONG arg);

void start_usbx(void) {
    /* Initialize USB */
    UINT status;

    /* This call should be as early as possible in tx_application_define(). As a
    * compromise it is placed first in this function. */
    status = ux_system_initialize(usb_buffer, USB_MEMORY_SIZE, UX_NULL, 0);
    /* Check for error.  */
    if (status != UX_SUCCESS) {
        Error_Handler();
    }

    memset(ram_store, 0, sizeof(ram_store));

    /* Get_Device_Framework_High_Speed and get the length */
    ULONG device_framework_hs_length;
    uint8_t *device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED,
                                &device_framework_hs_length);

    /* Get_Device_Framework_Full_Speed and get the length */
    ULONG device_framework_fs_length;
    uint8_t *device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED,
                                    &device_framework_fs_length);

    /* Get_String_Framework and get the length */
    ULONG string_framework_length;
    uint8_t *string_framework = USBD_Get_String_Framework(&string_framework_length);

    /* Get_Language_Id_Framework and get the length */
    ULONG languge_id_framework_length;
    uint8_t *language_id_framework = USBD_Get_Language_Id_Framework(&languge_id_framework_length);

    /* The code below is required for installing the device portion of USBX */
    status =  ux_device_stack_initialize(
                    device_framework_high_speed,
                    device_framework_hs_length,
                    device_framework_full_speed,
                    device_framework_fs_length,
                    string_framework,
                    string_framework_length,
                    language_id_framework,
                    languge_id_framework_length,
                    UX_NULL);
    /* Check for error.  */
    if (status != UX_SUCCESS) {
        Error_Handler();
    }

    /* Create Storage Class */
    UX_SLAVE_CLASS_STORAGE_PARAMETER storage_parameter;
    memset((void*)&storage_parameter, 0, sizeof(UX_SLAVE_CLASS_STORAGE_PARAMETER));

    storage_parameter.ux_slave_class_storage_parameter_number_lun = 1;

    /* Initialize the storage class parameters for reading/writing to the Flash Disk. */
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba = N_LBA;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length = RAM_STORE_BLOCK_LEN;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type = 0;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag = 0x80;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read = media_read;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write = media_write;
    storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status = media_status;
    //storage_parameter.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_flush = media_flush;


    char *device_name = "Storage";

    /* Get storage configuration number */
    storage_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_MSC, 0);

    /* Find storage interface number */
    storage_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_MSC, 0);

    /* Initialize the device storage class. The class is connected with interface 0 */
    status = ux_device_stack_class_register((UCHAR*)device_name, ux_device_class_storage_entry,
        storage_configuration_number, storage_interface_number, (VOID *)&storage_parameter);
    /* Check for error.  */
    if (status != UX_SUCCESS) {
        Error_Handler();
    }


    usb_phy_setup_device();

    /* Thread 5: USB device thread */
    status = tx_thread_create(
                    &usb_device_thread,
                    "USB thread",
                    tx_demo_thread_device_simulation_entry,
                    0,
                    usb_device_thread_stack,
                    USB_DEVICE_THREAD_STACK_SIZE,
                    20,
                    20,
                    TX_NO_TIME_SLICE,
                    TX_AUTO_START);

    /* Check for error.  */
    if (status != TX_SUCCESS) {
        Error_Handler();
    }
}

void  tx_demo_thread_device_simulation_entry(ULONG arg)
{

    UX_PARAMETER_NOT_USED(arg);
#if 0
UINT    status;
ULONG   actual_length;


    UX_PARAMETER_NOT_USED(arg);

    while(1)
    {

        /* Ensure the dpump class on the device is still alive.  */
        while (dpump_device != UX_NULL)
        {

            /* Increment thread counter.  */
            thread_1_counter++;

            /* Read from the device data pump.  */
            status =  _ux_device_class_dpump_read(dpump_device, device_buffer, UX_HOST_CLASS_DPUMP_PACKET_SIZE, &actual_length);

            /* Verify that the status and the amount of data is correct.  */
            if ((status != UX_SUCCESS) || actual_length != UX_HOST_CLASS_DPUMP_PACKET_SIZE)
                error_handler();

            /* Now write to the device data pump.  */
            status =  _ux_device_class_dpump_write(dpump_device, device_buffer, UX_HOST_CLASS_DPUMP_PACKET_SIZE, &actual_length);

            /* Verify that the status and the amount of data is correct.  */
            if ((status != UX_SUCCESS) || actual_length != UX_HOST_CLASS_DPUMP_PACKET_SIZE)
                error_handler();
        }

        /* Relinquish to other thread.  */
        tx_thread_relinquish();
    }
#endif
}

void  usb_device_thread_entry(ULONG arg)
{

//UINT    status;
//ULONG   actual_length;


    UX_PARAMETER_NOT_USED(arg);
    //UINT status;
    while(1)
    {
#if 0
        status = ux_system_tasks_run();

        /* Check for error.  */
        if (status != TX_SUCCESS) {
            Error_Handler();
        }


        /* Ensure the dpump class on the device is still alive.  */
        while (dpump_device != UX_NULL)
        {

            /* Increment thread counter.  */
            thread_1_counter++;

            /* Read from the device data pump.  */
            status =  _ux_device_class_dpump_read(dpump_device, device_buffer, UX_HOST_CLASS_DPUMP_PACKET_SIZE, &actual_length);

            /* Verify that the status and the amount of data is correct.  */
            if ((status != UX_SUCCESS) || actual_length != UX_HOST_CLASS_DPUMP_PACKET_SIZE)
                error_handler();

            /* Now write to the device data pump.  */
            status =  _ux_device_class_dpump_write(dpump_device, device_buffer, UX_HOST_CLASS_DPUMP_PACKET_SIZE, &actual_length);

            /* Verify that the status and the amount of data is correct.  */
            if ((status != UX_SUCCESS) || actual_length != UX_HOST_CLASS_DPUMP_PACKET_SIZE)
                error_handler();
        }

        /* Relinquish to other thread.  */
        tx_thread_relinquish();
#endif
    tx_thread_relinquish();
    }

}

UINT media_read(
    VOID *storage,
    ULONG lun,
    UCHAR *data_pointer,
    ULONG number_blocks,
    ULONG lba,
    ULONG *media_status)
{
    UX_PARAMETER_NOT_USED(storage);
    UX_PARAMETER_NOT_USED(lun);
    UX_PARAMETER_NOT_USED(data_pointer);
    UX_PARAMETER_NOT_USED(number_blocks);
    UX_PARAMETER_NOT_USED(lba);
    UX_PARAMETER_NOT_USED(media_status);

    ULONG read_len = number_blocks*RAM_STORE_BLOCK_LEN;
    UCHAR *read_start_addr = lba*RAM_STORE_BLOCK_LEN + ram_store;
    UCHAR *read_end_addr = read_start_addr + read_len;

    if (read_end_addr > ram_store+sizeof(ram_store)){
        __BKPT();
        return UX_INVALID_PARAMETER;
    }

    memcpy(data_pointer, read_start_addr, read_len);

    return UX_SUCCESS;
}

UINT media_write(
    VOID *storage,
    ULONG lun,
    UCHAR *data_pointer,
    ULONG number_blocks,
    ULONG lba,
    ULONG *media_status)
{
    UX_PARAMETER_NOT_USED(storage);
    UX_PARAMETER_NOT_USED(lun);
    UX_PARAMETER_NOT_USED(data_pointer);
    UX_PARAMETER_NOT_USED(number_blocks);
    UX_PARAMETER_NOT_USED(lba);
    UX_PARAMETER_NOT_USED(media_status);

    ULONG write_len = number_blocks*RAM_STORE_BLOCK_LEN;
    UCHAR *write_start_addr = lba*RAM_STORE_BLOCK_LEN + ram_store;
    UCHAR *write_end_addr = write_start_addr + write_len;

    if (write_end_addr > ram_store+sizeof(ram_store)){
        __BKPT();
        return UX_INVALID_PARAMETER;
    }

    memcpy(write_start_addr, data_pointer, write_len);

    return UX_SUCCESS;

    return UX_SUCCESS;
}

UINT media_status(
    VOID *storage,
    ULONG lun,
    ULONG media_id,
    ULONG *media_status)
{

    //*media_status = UX_SLAVE_CLASS_STORAGE_SENSE_KEY_NO_SENSE;
    UX_PARAMETER_NOT_USED(storage);
    UX_PARAMETER_NOT_USED(lun);
    UX_PARAMETER_NOT_USED(media_id);
    UX_PARAMETER_NOT_USED(media_status);

    return UX_SUCCESS;
}

UINT media_flush (VOID *storage, ULONG lun, ULONG number_blocks, ULONG lba, ULONG *media_status) {
    UX_PARAMETER_NOT_USED(storage);
    UX_PARAMETER_NOT_USED(lun);
    UX_PARAMETER_NOT_USED(number_blocks);
    UX_PARAMETER_NOT_USED(lba);
    UX_PARAMETER_NOT_USED(media_status);

    return UX_SUCCESS;
}