//
   
/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _MASTER_H_
#define _MASTER_H_


#ifdef __cplusplus
extern "C" {
#endif

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1




//PICO Definition
#define BANK {2,3,4,5,6,7,8,9,0}  // GPIO Relay position, could be 4 dimension
#define SE {4}   // could be for diemnsion
#define DIGIO {12,13,14,15,16,17,18,19}  // Digital IO position, could be 4 dimension


#endif // 