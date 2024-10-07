/**
 * @file    scpi_i2c.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Complete software code library to communicate to the user I2C port using
 *          SCPI command
 *
 * @details This software setup and control the communication of the user I2C port.
 *          Each SCPI command related to I2C port is handled by this software.
 *
 * @copyright  Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico_lib2/src/sys/include/sys_i2c.h"
#include "scpi_i2c.h"

/**
 * @brief Structure to contain the parameters of the user I2C configuration.
 *
 * This structure defines the configuration parameters for an I2C interface,
 * including the I2C instance, device address, baud rate, data bits, and status.
 */
struct user_i2c
{
  i2c_inst_t* i2c_id;  //!< I2C instance (e.g., i2c0 or i2c1).
  uint8_t address;     //!< I2C device address.
  uint32_t baudrate;   //!< Baud rate for I2C communication.
  uint32_t databits;   //!< Data size: 8 for byte and 16 for word.
  bool status;         //!< I2C status, true = enabled, false = disabled.
};

/**
 * @var uiic
 * @brief Global variable to hold the user I2C configuration.
 *
 * This variable is initialized with default settings for the I2C interface,
 * including the I2C instance, device address, baud rate, data bits, and status.
 */
struct user_i2c uiic = {
    DEF_I2C_USER,      //!< Default I2C instance.
    DEF_I2C_ADDR,      //!< Default I2C device address.
    DEF_I2C_BAUD,      //!< Default I2C baud rate.
    DEF_I2C_DATABITS,  //!< Default number of data bits (8 or 16).
    DEF_I2C_STATUS     //!< Default I2C status (enabled/disabled).
};

/**
 * @brief  function to configure the parameters of the user I2C based on the data set on the I2C structure
 *
 */
void scpi_i2c_enable()
{
  gpio_init(USER_I2C_SDA_PIN);
  gpio_set_function(USER_I2C_SDA_PIN, GPIO_FUNC_I2C);
  // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
  gpio_pull_up(USER_I2C_SDA_PIN);

  gpio_init(USER_I2C_SCL_PIN);
  gpio_set_function(USER_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(USER_I2C_SCL_PIN);

  i2c_init(uiic.i2c_id, uiic.baudrate);
  uiic.status = 1;  // set flag to indicate of I2C is enabled
  fprintf(stdout, "User I2C is enabled\r\n");
}

/**
 * @brief  function to disable the user spi and configure pins reserved by SPI to normal GPIO, set as input.
 *
 */
void scpi_i2c_disable()
{
  // Disable.
  i2c_deinit(uiic.i2c_id);

  // set pins used for i2c to GPIO mode
  gpio_set_function(USER_I2C_SDA_PIN, GPIO_FUNC_SIO);
  gpio_set_function(USER_I2C_SCL_PIN, GPIO_FUNC_SIO);

  bool mode = 0;                         // define GPIO as input
  gpio_set_dir(USER_I2C_SDA_PIN, mode);  // set pins as input if mode = 0
  gpio_set_dir(USER_I2C_SCL_PIN, mode);  // set pins as output if mode = 1

  uiic.status = 0;  // Reset flag to indicate of I2C port is disabled
  fprintf(stdout, "User I2C is disabled\r\n");
}

/**
 * @brief Function who return if the user I2C is enabled or not
 *
 *  @return  True  if I2C is enabled
 */
bool scpi_i2c_status()
{
  return uiic.status;
}

/**
 * @brief Function to update the device address to be used.
 *
 *
 * @param num  Device Address
 */
uint8_t scpi_i2c_set_address(uint32_t num)
{
  uiic.address = num;
  fprintf(stdout, "I2C Device address updated to  %d\r\n", num);
  return NOERR;
}

/**
 * @brief function to return the device address value saved on structure
 *
 * @return uint32_t  Device address
 */
uint32_t scpi_i2c_get_address()
{
  return uiic.address;
}

/**
 * @brief Function to update the baudrate speed on the structure, Baudrate will be updated with function spi enable
 *
 * @param speed Baudrate to save on structure
 */
void scpi_i2c_set_baudrate(uint32_t speed)
{
  if (speed != uiic.baudrate)
  {  // if value changed
    uiic.baudrate = speed;
    if (uiic.status)
    {  // id SPI is enabled, update the baudrate
      i2c_init(uiic.i2c_id, uiic.baudrate);
    }
  }
}

/**
 * @brief function to return the actual baudrate
 *
 * @return uint32_t  baudrate
 */
uint32_t scpi_i2c_get_baudrate()
{
  return (uiic.baudrate);
}

/**
 * @brief Function to update the number of databits to be used. Necessary to determine if we
 *        doing communication using byte or word
 *
 * @param num  Databits to write
 */
uint8_t scpi_i2c_set_databits(uint32_t num)
{
  uiic.databits = num;
  fprintf(stdout, "I2C Parameter databit updated to  %d\r\n", num);

  return NOERR;
}

/**
 * @brief function to return the databits value saved on structure
 *
 * @return uint32_t  databits
 */
uint32_t scpi_i2c_get_databits()
{
  return uiic.databits;
}

/**
 * @brief Main function called by SCPI command to send or receive data on I2C port.
 *
 * This function handles I2C communication by writing data to the I2C bus and reading
 * the response back. It supports both byte and word transfers depending on the flag.
 *
 * @param wdata Pointer to the data array to be written to the I2C device.
 * @param wlen  Length of the write array (number of bytes/words to write).
 * @param rdata Pointer to the array where the received data will be stored.
 * @param rlen  Length of the read array (number of bytes/words expected to read).
 * @param wflag Flag to specify data size: `true` for word transfers, `false` for byte transfers.
 *
 * @return int8_t Error code. Returns a negative value if an error occurs.
 */
int8_t scpi_i2c_wri_read_data(uint8_t* wdata, uint8_t wlen, uint8_t* rdata, uint8_t rlen, bool* wflag)
{
  size_t j;
  int32_t ret;

  if (uiic.status == 0)
  {
    return I2C_NOT_ENABLED;
  }

  if (wlen > 0 && rlen == 0)
  {  // if we need to perform write only
    ret = sys_i2c_wbuf(uiic.i2c_id, uiic.address, wdata, wlen);
    for (j = 0; j < wlen; j++)
    {  // loop to print
      fprintf(stdout, "I2C write buffer byte, data: 0x%02x\r\n", wdata[j]);
    }
  }

  if (wlen > 0 && rlen > 0)
  {  // if we need to perform write data and read data
    if (uiic.databits > 8)
    {
      rlen = rlen * 2;
    }  // multiply to get number of byte to read
    ret = sys_i2c_wbuf_rbuf(uiic.i2c_id, uiic.address, wdata, wlen, rdata, rlen);

    for (j = 0; j < wlen; j++)
    {  // loop to print
      fprintf(stdout, "I2C write buffer byte, data: 0x%02x\r\n", wdata[j]);
    }
    for (j = 0; j < rlen; j++)
    {  // loop to print
      if (uiic.databits > 8)
      {
        fprintf(stdout, "I2C read after write, buffer word, data: 0x%02x%02x\r\n", rdata[j], rdata[j + 1]);
        j++;
      }
      else
      {
        fprintf(stdout, "I2C read after write, buffer byte, data: 0x%02x\r\n", rdata[j]);
      }
    }
  }

  if (wlen == 0 && rlen > 0)
  {  // if we need to perform read only
    if (uiic.databits > 8)
    {
      rlen = rlen * 2;
    }  // multiply nb of word by 2 to get number of byte to read
    ret = sys_i2c_rbuf(uiic.i2c_id, uiic.address, rdata, rlen);

    for (j = 0; j < rlen; j++)
    {  // loop to print
      if (uiic.databits > 8)
      {
        fprintf(stdout, "I2C read buffer word, data: 0x%02x%02x\r\n", rdata[j], rdata[j + 1]);
        j++;
      }
      else
      {
        fprintf(stdout, "I2C read buffer byte, data: 0x%02x\r\n", rdata[j]);
      }
    }
  }

  *wflag = (uiic.databits <= 8) ? false : true;  //!< set flag to return the size of data (byte or word)

  if (*wflag)
  {  // if databits is word (16 bits)
    // swap byte due to endianess of memory
    for (size_t i = 0; i < rlen; i += 2)
    {
      uint8_t temp = rdata[i];
      rdata[i] = rdata[i + 1];
      rdata[i + 1] = temp;
    }
  }

  //  return error if value is negative or return number of bytes read or write
  if (ret >= 0)
  {
    return NOERR;
  }
  else
  {
    fprintf(stdout, "I2C Error return:  %d,\r\n", ret);
    return (int8_t)ret;
  }
}
