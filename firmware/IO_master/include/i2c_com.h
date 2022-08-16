/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _I2C_COM_H_
#define _I2C_COM_H_


#ifdef __cplusplus
extern "C" {
#endif

#define PICO_PORT_ADDRESS 0x22  // Pico address where genierci port A and Port B are used
#define REG_STATUS 100  // Register used to report Status

#define I2C_BAUDRATE 100000   // 100 k
#define I2C_MASTER_SDA_PIN  6
#define I2C_MASTER_SCL_PIN 7


// GPIO Index for relay position. SE Relay located at index 16 
# define relay {0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,0,0,0,10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18}

void setup_master();

void send_master(uint8_t i2c_add,uint8_t cmd, uint16_t wdata);

#endif // 