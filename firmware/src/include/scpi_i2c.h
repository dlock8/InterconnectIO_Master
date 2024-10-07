/**
 * @file    scpi_i2c.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header file defining constants and macros for the I2C communication.
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef _SCPI_I2C_H_
#define _SCPI_I2C_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @def USER_I2C_SDA_PIN
 * @brief GPIO pin used for I2C SDA (Data line).
 */
#define USER_I2C_SDA_PIN 6

/**
 * @def USER_I2C_SCL_PIN
 * @brief GPIO pin used for I2C SCL (Clock line).
 */
#define USER_I2C_SCL_PIN 7

#define DEF_I2C_USER i2c1   //!< i2c1 is the user communication channel to external
#define DEF_I2C_BAUD 1E5    //!< i2c baudrate is set at 100Khz by default
#define DEF_I2C_ADDR 0      //!< Default device Address
#define DEF_I2C_DATABITS 8  //!< i2c number of bits used for comunication
#define DEF_I2C_STATUS 0    //!< spi is disable by default

/**
 * @brief Error codes for I2C operations.
 */

/** @def NOERR
 *  @brief No error.
 */
#define NOERR 0

/** @def I2C_GENERIC_ERR
 *  @brief Generic I2C error.
 */
#define I2C_GENERIC_ERR -1

/** @def I2C_TIMEOUT_ERR
 *  @brief I2C operation timed out.
 */
#define I2C_TIMEOUT_ERR -2

/** @def I2C_ADDRESS_NACK
 *  @brief No acknowledgment from the specified address.
 */
#define I2C_ADDRESS_NACK -3

/** @def I2C_DATA_NACK
 *  @brief No acknowledgment for the data.
 */
#define I2C_DATA_NACK -4

/** @def I2C_BUS_ERR
 *  @brief Bus error encountered during I2C operation.
 */
#define I2C_BUS_ERR -5

/** @def I2C_MALLOC_FAILURE
 *  @brief Memory allocation failure during I2C operation.
 */
#define I2C_MALLOC_FAILURE 52

/** @def I2C_NOT_ENABLED
 *  @brief I2C interface is not enabled.
 */
#define I2C_NOT_ENABLED 53

  void scpi_i2c_enable(void);
  void scpi_i2c_disable(void);
  bool scpi_i2c_status(void);
  void scpi_i2c_set_baudrate(uint32_t speed);
  uint32_t scpi_i2c_get_baudrate(void);

  uint8_t scpi_i2c_set_databits(uint32_t num);
  uint32_t scpi_i2c_get_databits(void);

  uint8_t scpi_i2c_set_address(uint32_t num);
  uint32_t scpi_i2c_get_address(void);

  int8_t scpi_i2c_wri_read_data(uint8_t* wdata, uint8_t wlen, uint8_t* rdata, uint8_t rlen, bool* wflag);

#ifdef __cplusplus
}
#endif

#endif  // _SCPI_I2C_H_
