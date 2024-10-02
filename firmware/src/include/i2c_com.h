/**
 * @file    i2c_com.h
 * @author  Daniel Lockhead
 * @date    2024
 * 
 * @brief   Header file defining constants and macros for internal i2c bus
 *   
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */
 

#ifndef _I2C_COM_H_
#define _I2C_COM_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hardware/i2c.h"

/**
 * @brief I2C addresses and configuration for Pico devices.
 */
#define PICO_MASTER_ADDRESS       0x00       /**< I2C address for Pico Master. */
#define PICO_PORT_ADDRESS         0x21       /**< I2C address for Pico Slave #1. */
#define PICO_RELAY1_ADDRESS       0x22       /**< I2C address for Pico Slave #2. */
#define PICO_RELAY2_ADDRESS       0x23       /**< I2C address for Pico Slave #3. */
#define PICO_SELFTEST_ADDRESS     0x20       /**< I2C address for Pico Selftest. */

#define REG_STATUS                100          /**< Register used to report status. */

#define I2C_BAUDRATE              100000       /**< I2C baud rate (100 kHz). */
#define I2C_MASTER_SDA_PIN        20           /**< GPIO used for I2C SDA. */
#define I2C_MASTER_SCL_PIN        21           /**< GPIO used for I2C SCL. */


/**
 * @brief GPIO configuration for relay actuation and digital ports.
 */

/** 
 * @def RBK
 * @brief Array of relay indices assigned to Pico GPIOs for relay actuation.
 * 
 * Each sub-array corresponds to a bank of relays with assigned GPIO pins.
 */
#define RBK {{0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7},\
{0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7}, \
{10,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17}, \
{10,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17}, }

/** 
 * @def DIGP
 * @brief Array of digital bit indices assigned to Pico GPx for Port0 and Port1. 
 */
#define DIGP {{0,1,2,3,4,5,6,7}, \
{10,11,12,13,14,15,16,17}}

/** @name Single-ended Relay GPIOs
 *  @{
 */
#define SE_BK1 19  /**< GPIO for single-ended relay of bank 1. */
#define SE_BK2 18  /**< GPIO for single-ended relay of bank 2. */
#define SE_BK3 19  /**< GPIO for single-ended relay of bank 3. */
#define SE_BK4 18  /**< GPIO for single-ended relay of bank 4. */
/** @} */



/** I2C Command Codes for executing actions. */
#define MJR_VERSION         01     //!< Major version of the protocol
#define MIN_VERSION         02     //!< Minor version of the protocol
#define OPEN_RELAY          10     //!< Command to open a relay
#define CLOSE_RELAY         11     //!< Command to close a relay
#define OPEN_RELAY_BANK     12     //!< Command to open a specific relay bank
#define STATE_RELAY         15     //!< Command to get the state of a relay
#define STATE_BANK          13     //!< Command to get the state of a relay bank
#define DIG_DIR_MASK        80     //!< Digital direction mask command
#define DIG_OUT             81     //!< Command to set digital output
#define DIG_IN              85     //!< Command to read digital input
#define DIR_GP_OUT          20     //!< Command to set GPIO direction to output
#define DIR_GP_IN           21     //!< Command to set GPIO direction to input
#define DIR_GP_READ         25     //!< Command to read GPIO direction
#define DIG_GP_OUT_CLEAR    10     //!< Command to clear digital GPIO output
#define DIG_GP_OUT_SET      11     //!< Command to set digital GPIO output
#define DIG_GP_IN           15     //!< Command to read digital GPIO input
#define GP_PAD_VALUE        60     //!< Command to get GPIO pad value
#define GP_PAD_SET          61     //!< Command to set GPIO pad configuration
#define GP_PAD_READ         65     //!< Command to read GPIO pad configuration
#define GP_FUNCTION         75     //!< Command to set or get GPIO function
#define SL_DEV_STATUS       100    //!< Command to get the status of the slave device
#define ENABLE_UART         101    //!< Command to enable UART communication
#define DISABLE_UART        102    //!< Command to disable UART communication
#define SET_UART_PROT       103    //!< Command to set UART protocol configuration
#define GET_UART_CFG        105    //!< Command to get UART configuration
#define ENABLE_SPI          111    //!< Command to enable SPI communication
#define DISABLE_SPI         112    //!< Command to disable SPI communication
#define SET_SPI_CFG         113    //!< Command to set SPI configuration
#define GET_SPI_CFG         115    //!< Command to get SPI configuration


void setup_master();
bool send_master(i2c_inst_t* i2c,uint8_t i2c_add,uint8_t cmd, uint16_t wdata, uint16_t *rback);
bool relay_execute(uint16_t *list,uint8_t action, uint16_t *answer);
bool digital_execute(uint8_t action, uint8_t port, uint8_t bit, uint8_t value, uint16_t *answer);
bool gpio_execute(uint8_t action, uint8_t device, uint8_t gpio, uint8_t value, uint16_t *answer);
bool system_execute(uint8_t action, uint16_t *answer);


#endif // 