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
 *     Ali Eissa - 2026 version.
 *     Jacob Rosén - STM32F429ZI USB MSC adaptation
 */

#include <stdio.h>
#include "ansi_colors.h"
#include "board_init.h"
#include "tx_api.h"
#include "usb.h"

/* --- ThreadX Resource Definitions --- */

/* 1. Green LED Blinky Thread */
#define GREEN_THREAD_STACK_SIZE 1024
TX_THREAD green_thread;
CHAR green_thread_stack[GREEN_THREAD_STACK_SIZE];


/* 6. printF Mutex */
TX_MUTEX printf_mutex;

/* Thread Function Prototypes */
void green_thread_entry(ULONG thread_input);

/* External console function declaration */
int __io_getchar(void);

/**
  * @brief  Standard C Main entry point.
  * @retval int
  */
int main(void)
{
    board_init();

    printf(ANSI_BOLD ANSI_CYAN "\r\n==========================================\r\n" ANSI_RESET);
    printf(ANSI_BOLD ANSI_CYAN "USBX Device MSC Demo Booted!\r\n");
    printf(ANSI_BOLD ANSI_CYAN "==========================================\r\n\r\n" ANSI_RESET);

    /* Initialize USB */
    usb_phy_init();

    tx_kernel_enter();

    return 0;
}

/**
  * @brief  Define ThreadX resources
  * @param  first_unused_memory: pointer to first free memory block in RAM
  * @retval None
  */
void tx_application_define(void *first_unused_memory)
{
    UINT status;
    (void)first_unused_memory;

    printf("\x1b[38;5;243m[LOG]" ANSI_RESET " " MSG_INFO "Starting initialization of ThreadX resources\r\n" ANSI_RESET);

    /* Thread 1: Blinks Green LED */
    status = tx_thread_create(&green_thread,
                     "Green LED Thread",
                     green_thread_entry,
                     0,
                     green_thread_stack,
                     GREEN_THREAD_STACK_SIZE,
                     15, /* Lower priority */
                     15,
                     TX_NO_TIME_SLICE,
                     TX_AUTO_START);
    /* Check for error.  */
    if (status != TX_SUCCESS) {
        Error_Handler();
    }

    /* Mutex 1: Printf guard */
    tx_mutex_create(&printf_mutex,
                    "Printf Guard",
                    TX_NO_INHERIT);


    printf("\x1b[38;5;243m[LOG]" ANSI_RESET " " MSG_SUCCESS "ThreadX resources initialized successfully!\r\n" ANSI_RESET);
    /* Start filex before USB*/
    start_usbx();
}

/**
  * @brief  Thread 1: Blinks the Green LED (LD1) continuously to show life.
  * @param  thread_input: unused
  * @retval None
  */
void green_thread_entry(ULONG thread_input)
{
    (void)thread_input;

    while (1)
    {
        /* Toggles LD1 (Green) on and off every 250 milliseconds */
        LED1_ON();
        tx_thread_sleep(25); /* 25 ticks = 250 ms at 100 Hz tick rate */

        LED1_OFF();
        tx_thread_sleep(25);
    }
}