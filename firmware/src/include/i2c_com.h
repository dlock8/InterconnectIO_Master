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

#include "hardware/i2c.h"

#define PICO_MASTER_ADDRESS 0       // Pico Master
#define PICO_PORT_ADDRESS   0x21    // Pico Slave #1
#define PICO_RELAY1_ADDRESS 0x22    // Pico Slave #2
#define PICO_RELAY2_ADDRESS 0x23    // Pico Slave #3
#define PICO_SELFTEST_ADDRESS 0x20  // Pico Selftest


#define REG_STATUS 100  // Register used to report Status

#define I2C_BAUDRATE 100000   // 100k
#define I2C_MASTER_SDA_PIN  20 // GPIO used to I2C SDA
#define I2C_MASTER_SCL_PIN  21 // GPIO used to I2C SCL


// Array of relay index to pico gpio assigned to control the relay actuation
#define RBK {{0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7},\
{0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7}, \
{10,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17}, \
{10,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17}, }

// Array of digital bit index to pico GPx. Usedby Port0 and Port1 
#define DIGP {{0,1,2,3,4,5,6,7}, \
{10,11,12,13,14,15,16,17},}




#define SE_BK1 19   // Relay single ended for bank 1 located at gpio x
#define SE_BK2 18   // Relay single ended for bank 2 located at gpio x
#define SE_BK3 19   // Relay single ended for bank 3 located at gpio x
#define SE_BK4 18   // Relay single ended for bank 4 located at gpio x


// I2C Command code to execute action

#define MJR_VERSION         01
#define MIN_VERSION         02
#define OPEN_RELAY          10
#define CLOSE_RELAY         11
#define OPEN_RELAY_BANK     12
#define STATE_RELAY         15
#define STATE_BANK          13
#define DIG_DIR_MASK        80
#define DIG_OUT             81
#define DIG_IN              85
#define DIR_GP_OUT          20
#define DIR_GP_IN           21
#define DIR_GP_READ         25
#define DIG_GP_OUT_CLEAR    10
#define DIG_GP_OUT_SET      11
#define DIG_GP_IN           15
#define GP_PAD_VALUE        60
#define GP_PAD_SET          61
#define GP_PAD_READ         65
#define GP_FUNCTION         75
#define SL_DEV_STATUS       100
#define ENABLE_UART         101
#define DISABLE_UART        102
#define SET_UART_PROT       103
#define GET_UART_CFG        105
#define ENABLE_SPI          111
#define DISABLE_SPI         112
#define SET_SPI_CFG         113
#define GET_SPI_CFG         115



void setup_master();

bool send_master(i2c_inst_t* i2c,uint8_t i2c_add,uint8_t cmd, uint16_t wdata, uint16_t *rback);
bool relay_execute(uint16_t *list,uint8_t action, uint16_t *answer);
bool digital_execute(uint8_t action, uint8_t port, uint8_t bit, uint8_t value, uint16_t *answer);
bool gpio_execute(uint8_t action, uint8_t device, uint8_t gpio, uint8_t value, uint16_t *answer);
bool system_execute(uint8_t action, uint16_t *answer);


#endif // 