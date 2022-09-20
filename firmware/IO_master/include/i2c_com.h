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

#define PICO_PORT_ADDRESS 0x21  // Pico address where generic port A and Port B are used
#define PICO_RELAY1_ADDRESS 0x22
#define PICO_RELAY2_ADDRESS 0x23

#define REG_STATUS 100  // Register used to report Status

#define I2C_BAUDRATE 100000   // 100 k
#define I2C_MASTER_SDA_PIN  6
#define I2C_MASTER_SCL_PIN 7

/*
// GPIO Index for relay position. SE Relay located at index 16 
#define RBK {{0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7}, \
{10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17}, \
{0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7}, \
{10,10,11,11,12,12,13,13,14,14,15,15,16,16,17,17}}
*/

#define RBK {{0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7}, \
{10,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17}, \
{0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7}, \
{10,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17}, }


#define SE_BK1 19   // Relay single ended for bank 1 located at gpio x
#define SE_BK2 18   // Relay single ended for bank 2 located at gpio x
#define SE_BK3 19   // Relay single ended for bank 3 located at gpio x
#define SE_BK4 18   // Relay single ended for bank 4 located at gpio x


// I2C Command code to send to slave

#define MJR_VERSION         01
#define MNR_VERSION         02
#define OPEN_RELAY          10
#define CLOSE_RELAY         11
#define OPEN_RELAY_BANK     12
#define STATE_RELAY         15
#define STATE_BANK          13

// Error codefrom I2C sub

#define I2C_COM_ERROR       125  // I2C communication error
#define RELAY_NUM_ERROR     126  // Relay numbering erro



void setup_master();

bool send_master(uint8_t i2c_add,uint8_t cmd, uint16_t wdata, uint8_t *rback);

bool  relay_execute(uint16_t *list,uint8_t action, uint8_t *answer);

#endif // 