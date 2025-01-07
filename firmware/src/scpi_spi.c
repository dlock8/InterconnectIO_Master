/**
 * @file    scpi_spi.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Complete software code library to communicate to the user SPI port using
 *          SCPI command
 *
 * @details This software setup and control the communication of the SPI port.
 *          Each SCPI command related to SPI port is handled by this software.
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "include/scpi_spi.h"

/**
 * @brief structure to contents the parameters of the user SPI
 *
 */
struct user_spi
{
  spi_inst_t* spi_id;  //!< spi id, define as uart0 for user spi
  uint32_t baudrate;   //!< spi baudrate
  uint32_t databits;   //!< spi size of data
  uint8_t cs;          //!< spi Chipselect gpio used
  uint8_t mode;        //!< spi mode to define Cs, cpol,cpha and msb
  bool status;         //!< spi enable or disable.  1 = enable
};

/**
 * @brief Global SPI configuration structure initialized with default values.
 *
 * This structure holds the default configuration for the SPI interface, including
 * settings for baud rate, data bits, chip select, SPI mode, and status.
 */
struct user_spi uspi = {
    DEF_SPI_USER,      //!< Default SPI user instance (e.g., spi0 or spi1).
    DEF_SPI_BAUD,      //!< Default baud rate for SPI communication.
    DEF_SPI_DATABITS,  //!< Number of data bits in SPI communication (typically 8 bits).
    DEF_SPI_CS,        //!< Default chip select pin for SPI communication.
    DEF_SPI_MODE,      //!< SPI mode (0 to 3), defining clock polarity and phase.
    DEF_SPI_STATUS     //!< SPI status, 1 = enabled, 0 = disabled.
};

/**
 * @brief  function to configure the parameters of the user spi based on the data set on the spi structure
 *
 */
void scpi_spi_enable()
{
  // Set GPIO function to SPI
  gpio_set_function(USER_SPI_RX_PIN, GPIO_FUNC_SPI);
  gpio_set_function(USER_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(USER_SPI_TX_PIN, GPIO_FUNC_SPI);

  //!< Configure the gpio who will be used as chipselect
  gpio_init(uspi.cs);
  gpio_set_dir(uspi.cs, GPIO_OUT);
  gpio_put(uspi.cs, 1);  //!< set to high by default

  spi_set_slave(uspi.spi_id, 0);         // spi is master
  spi_init(uspi.spi_id, uspi.baudrate);  // Init SPI and speed
  scpi_spi_set_mode(uspi.mode);          // set mode
  uspi.status = 1;                       // set flag to indicate of SPI is enabled
  fprintf(stdout, "User SPI is enabled\r\n");
}

/**
 * @brief  function to disable the user spi and configure pins reserved by SPI to normal GPIO, set as input.
 *
 */
void scpi_spi_disable()
{
  // Disable.
  spi_deinit(uspi.spi_id);

  // set pins used for uart to GPIO mode
  gpio_set_function(USER_SPI_RX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(USER_SPI_SCK_PIN, GPIO_FUNC_SIO);
  gpio_set_function(USER_SPI_TX_PIN, GPIO_FUNC_SIO);
  // gpio_set_function(USER_SPI_CSN_PIN, GPIO_FUNC_SIO);

  bool mode = 0;                         // define GPIO as input
  gpio_set_dir(USER_SPI_RX_PIN, mode);   // set pins as input if mode = 0
  gpio_set_dir(USER_SPI_SCK_PIN, mode);  // set pins as output if mode = 1
  gpio_set_dir(USER_SPI_TX_PIN, mode);   // set pins as input
  gpio_set_dir(uspi.cs, mode);           // set pins as input

  uspi.status = 0;  // Reset flag to indicate of serial port is disabled
  fprintf(stdout, "User SPI is disabled\r\n");
}

/**
 * @brief Function who return if the user spi is enabled or not
 *
 *  @return  True  if spi is enabled
 */
bool scpi_spi_status()
{
  return uspi.status;
}

/**
 * @brief Function to update the baudrate speed on the structure, Baudrate will be updated with function spi enable
 *
 * @param speed Baudrate to save on structure
 */
void scpi_spi_set_baudrate(uint32_t speed)
{
  if (speed != uspi.baudrate)
  {  // if value changed
    uspi.baudrate = speed;
    if (uspi.status)
    {  // id SPI is enabled, update the baudrate
      spi_init(uspi.spi_id, uspi.baudrate);
    }
  }
}

/**
 * @brief function to return the baudrate
 *
 * @return uint32_t  baudrate
 */
uint32_t scpi_spi_get_baudrate()
{
  return uspi.baudrate;
}

/**
 * @brief Function to update the SPI ChipSelect to be used.
 *
 * @param num  gpio to write
 * @return uint8_t  error if number is not valid baudrate
 */
uint8_t scpi_spi_set_chipselect(uint32_t num)
{
  uint8_t gpio_list[] = {0, 1, 5, 6, 7, 12, 13, 14, 15, 16, 17};  // List of valid gpio to be used as Chipselect on Master pico

  bool valid = false;
  size_t sizeg = sizeof(gpio_list) / sizeof(gpio_list[0]);

  //!< Loop to determine if the gpio required for become new ChipSelect is valid
  for (size_t i = 0; i < sizeg; i++)
  {
    if (num == gpio_list[i])
    {
      valid = true;
      break;
    }
  }

  if (valid == false)
  {
    return SPI_CS_NUM_ERROR;
  }  // if number is not on the list

  if (num != uspi.cs)
  {  // if value changed, configure the new gpio

    uspi.cs = num;  //!< save in structure the new gpio
    //!< Configure the gpio who will be used as chipselect
    gpio_init(uspi.cs);
    gpio_set_dir(uspi.cs, GPIO_OUT);
    gpio_put(uspi.cs, 1);  //!< set to high by default

    fprintf(stdout, "SPI Chipselect gpio updated to:  %d\r\n", num);
  }

  return NOERR;  // no error
}

/**
 * @brief function to return the chipselect gpio used
 *
 * @return uint32_t  gpio
 */
uint32_t scpi_spi_get_chipselect()
{
  return uspi.cs;
}

/**
 * @brief Function to update the value of databits to be used.
 *
 * @param num  Databits to write
 */
uint8_t scpi_spi_set_databits(uint32_t num)
{
  if (num != uspi.databits)
  {
    uspi.databits = num;
    scpi_spi_set_mode(uspi.mode);
  }
  fprintf(stdout, "SPI Parameter databit updated to  %d\r\n", num);
  return NOERR;
}

/**
 * @brief function to return the databits value saved on structure
 *
 * @return uint32_t  databits
 */
uint32_t scpi_spi_get_databits()
{
  return uspi.databits;
}

/**
 * @brief function to set the mode to use for spi communication 
 *
 * @param mode  value of mode to use
 * @return uint8_t error number
 */
uint8_t scpi_spi_set_mode(uint8_t mode)
{
  bool cpol, cpha, msb, cs;

  msb = SPI_MSB_FIRST;  // LSB First is not supported

  switch (mode)
  {
    case 0:
    {
      cs = 0;
      cpol = SPI_CPOL_0;
      cpha = SPI_CPHA_0;
      break;
    }  //!< set value for mode 0-3
    case 1:
    {
      cs = 0;
      cpol = SPI_CPOL_0;
      cpha = SPI_CPHA_1;
      break;
    }
    case 2:
    {
      cs = 0;
      cpol = SPI_CPOL_1;
      cpha = SPI_CPHA_0;
      break;
    }
    case 3:
    {
      cs = 0;
      cpol = SPI_CPOL_1;
      cpha = SPI_CPHA_1;
      break;
    }
    case 4:
    {
      cs = 1;
      cpol = SPI_CPOL_0;
      cpha = SPI_CPHA_0;
      break;
    }  //!< set value for mode 4-7
    case 5:
    {
      cs = 1;
      cpol = SPI_CPOL_0;
      cpha = SPI_CPHA_1;
      break;
    }
    case 6:
    {
      cs = 1;
      cpol = SPI_CPOL_1;
      cpha = SPI_CPHA_0;
      break;
    }
    case 7:
    {
      cs = 1;
      cpol = SPI_CPOL_1;
      cpha = SPI_CPHA_1;
      break;
    }
    default:
    {
      return SPI_MODE_NUM_NOTVALID;
    }
  }
  uspi.mode = mode;                                             //!< save mode used in structure
  spi_set_format(uspi.spi_id, uspi.databits, cpol, cpha, msb);  //!< set format
  uint32_t speed = spi_get_baudrate(uspi.spi_id);
  fprintf(stdout, "SPI Mode=%d, mean: CS=%d, Cpol=%d, Cpha=%d, Msb=%d, Baud=%d, Actual Baud=%d\r\n", mode, cs, cpol, cpha, msb, uspi.baudrate, speed);
  return NOERR;
}

/**
 * @brief function who return the spi mode used in communication.
 *
 * @return uint8_t  mode value
 */
uint8_t scpi_spi_get_mode()
{
  return uspi.mode;  //!< return value saved in structure
}

/**
 * @brief  function who transform an array of bytes to an array of words.
 *
 * @param byte_array  pointer to the array of bytes to transform
 * @param word_array  pointer to the array of the resulting word
 * @param length      length of the byte array
 */
void spi_bytes_to_words(uint8_t* byte_array, uint16_t* word_array, size_t length)
{
  for (size_t i = 0; i < length / 2; i++)
  {
    word_array[i] = byte_array[2 * i] << 8 | (byte_array[2 * i + 1]);  //!< transform 2 bytes in 1 word
                                                                       // fprintf(stdout, "bytes to word # %d, data: %02x\r\n",i,word_array[i]);
  }
}

/**
 * @brief function who transform an array of word to an array of byte
 *
 * @param word_array pointer to the array of words to transform
 * @param byte_array pointer to the array of the resulting bytes
 * @param length length of the word array
 */
void spi_words_to_bytes(uint16_t* word_array, uint8_t* byte_array, size_t length)
{
  for (size_t i = 0; i < length; i++)
  {
    byte_array[2 * i] = word_array[i] & 0xFF;      // Least significant byte
    byte_array[2 * i + 1] = (word_array[i] >> 8);  // Most significant byte
                                                   // fprintf(stdout, "word to bytes # %d, data: %02x\r\n",i,byte_array[i]);
  }
}

/**
 * @brief Global flag indicating whether an SPI timeout has occurred.
 *
 * This variable is set to `true` when an SPI timeout occurs and is used
 * to trigger specific actions, such as disabling the SPI interface.
 */
bool spi_timeout = false;

/**
 * @brief Callback function triggered by an alarm to handle SPI timeout.
 *
 * This function is called when an SPI timeout occurs. It sets the global
 * `spi_timeout` flag to `true`, and if the SPI is enabled, it disables the
 * SPI interface to prevent blocking operations. It can also perform additional
 * tasks, such as configuring a GPIO pin.
 *
 * @param id Alarm ID that triggered the function.
 * @param user_data User-defined data passed to the callback (can be NULL if unused).
 *
 * @return int64_t Return 0 to indicate no further alarms need to be triggered.
 *
 * @note This function is typically called by the system in response to an alarm,
 * and it should return quickly to avoid blocking other operations.
 */
int64_t spi_alert_function(alarm_id_t id, void* user_data)
{
  spi_timeout = true;
  if (uspi.status)
  {                      // if SPI enabled
    scpi_spi_disable();  // disable SPI to get out of blocking function
  }
  // set GPIO pin ...
  return 0;
}

/**
 * @brief function who write/read bytes to defined SPI port
 *
 * @param mode Constant who define the function to perform: write data,write-read data or read data
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen contain length of the read array
 */
static uint8_t spi_bytes(uint8_t mode, uint8_t* wdata, uint8_t wlen, uint8_t* rdata, uint8_t rlen)
{
  uint8_t csdelay = 1;  // 1 us delay
  uint8_t i, j;

  //  If required, add alarm to get out of blocking function
  //  alarm_id_t r = add_alarm_in_us(ALARM_TIMEOUT, &spi_alert_function, NULL, true);
  fprintf(stdout, "On SPI bytes\r\n");
  for (i = 0; i < wlen + rlen; i++)
  {  // loop to write single byte
    if (i >= wlen)
    {
      wdata[i] = 0;
    }                      // send 0 as default value
    gpio_put(uspi.cs, 0);  //!< Active Chipselect
    sleep_us(csdelay);     //!<  wait small delay for let CS goes active
    while (!spi_is_writable(uspi.spi_id))
    {
    }  // Wait until buffer is writable
    //!< based on mode selected, send SPI data, 1 byte at the time because
    //!< chipselect need to be toggle at each byte on some device. (mandatory on SPI slave mode)
    if (mode == SPIW)
    {
      spi_write_blocking(uspi.spi_id, &wdata[i], 1);
    }
    if (mode == SPIWR)
    {
      spi_write_read_blocking(uspi.spi_id, &wdata[i], &rdata[i], 1);
    }
    if (mode == SPIR)
    {
      spi_read_blocking(uspi.spi_id, SPI_DEFAULT_VALUE, &rdata[i], 1);
    }
    if (uspi.mode >= 4)
    {  // If CS need to toggle each byte
      sleep_us(csdelay);
      gpio_put(uspi.cs, 1);  //!< De-active Chipselect
      sleep_us(csdelay);     //!<  wait small delay for let CS goes de-active
    }
  }

  if (uspi.mode <= 3)
  {                        // If CS need to toggle at end of transmission
    gpio_put(uspi.cs, 1);  // De-activate CS
  }

  if (spi_timeout)
  {
    fprintf(stdout, "SPI timeout Occurs\r\n");
    scpi_spi_enable();  // enable SPI
    return SPI_TIMEOUT;
  }

  if (mode == SPIW)
  {  //!<  send message to monitor to help debug
    fprintf(stdout, "SPI write, nb of bytes  written: %d \r\n", wlen);
  }

  if (mode == SPIWR)
  {  //!<  send message to monitor to help debug
    fprintf(stdout, "SPI write-read, nb of bytes to write: %d, Nb of bytes to read: %d\r\n", wlen, rlen);
    for (j = 0; j < wlen + rlen; j++)
    {  // loop to print
      fprintf(stdout, "SPI write-read,# %02d, Write: 0x%x, Read: %02x\r\n", j, wdata[j], rdata[j]);
    }
  }

  if (mode == SPIR)
  {  //!<  send message to monitor to help debug
    fprintf(stdout, "SPI read, Nb of bytes to read: %d\r\n", rlen);
    for (j = 0; j < rlen; j++)
    {  // loop to print
      fprintf(stdout, "SPI read,# %d, Read: %02x\r\n", j, rdata[j]);
    }
  }

  return NOERR;
}

/**
 * @brief function who write/read word to defined SPI port
 *
 * @param mode Constant who define the function to perform: write data,write-read data or read data
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen contain length of the read array
 */
static uint8_t spi_word(uint8_t mode, uint16_t* wdata, uint8_t wlen, uint16_t* rdata, uint8_t rlen)
{
  uint8_t csdelay = 1;  // 1 us delay
  uint8_t i, j;

  // alarm_id_t r = add_alarm_in_us(ALARM_TIMEOUT, &spi_alert_function, NULL, true);

  for (i = 0; i < wlen + rlen; i++)
  {  // loop to write single byte
    if (i >= wlen)
    {
      wdata[i] = 0;
    }  // send 0 as default value
    gpio_put(uspi.cs, 0);
    sleep_us(csdelay);
    while (!spi_is_writable(uspi.spi_id))
    {
    }  // Wait until buffer is writable
    //!< based on mode selected, send SPI data, 1 word at the time because
    //!< chipselect need to be toggle at each word on some device. (mandatory on Pico SPI slave mode)
    if (mode == SPIW)
    {
      spi_write16_blocking(uspi.spi_id, &wdata[i], 1);
    }
    if (mode == SPIWR)
    {
      spi_write16_read16_blocking(uspi.spi_id, &wdata[i], &rdata[i], 1);
    }
    if (mode == SPIR)
    {
      spi_read16_blocking(uspi.spi_id, SPI_DEFAULT_VALUE, &rdata[i], 1);
    }
    if (uspi.mode >= 4)
    {  // If CS need to toggle each word
      sleep_us(csdelay);
      gpio_put(uspi.cs, 1);  //!< De-active Chipselect
      sleep_us(csdelay);     //!<  wait small delay for let CS goes de-active
    }
  }
  if (uspi.mode <= 3)
  {                        // If CS need to toggle at end of transmission
    gpio_put(uspi.cs, 1);  // De-activate CS
  }

  if (spi_timeout)
  {
    fprintf(stdout, "SPI timeout Occurs\r\n");
    scpi_spi_enable();  // enable SPI
    return SPI_TIMEOUT;
  }

  if (mode == SPIW)
  {  //!<  send message to monitor to help debug
    fprintf(stdout, "SPI write, nb of word  written: %d\r\n", wlen);
  }

  if (mode == SPIWR)
  {  //!<  send message to monitor to help debug
    fprintf(stdout, "SPI write-read, nb of word to write: %d, Nb of word to read: %d\r\n", wlen, rlen);
    for (j = 0; j < wlen + rlen; j++)
    {  // loop to print
      fprintf(stdout, "SPI write-read,# %d, Write: 0x%04x, Read: 0x%04x\r\n", j, wdata[j], rdata[j]);
    }
  }

  if (mode == SPIR)
  {  //!<  send message to monitor to help debug
    fprintf(stdout, "SPI read, nb of word to read: %d\r\n", rlen);
    for (j = 0; j < rlen; j++)
    {  // loop to print
      fprintf(stdout, "SPI read,# %d, Read: 0x%04x\r\n", j, rdata[j]);
    }
  }
  return NOERR;
}

/**
 * @brief main function called by SCPI command to send or receive data on SPI port
 *
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen  contain length of the read array
 * @param wflag Flag to define if we process bytes size or word size
 * @return uint8_t error code
 */
uint8_t scpi_spi_wri_read_data(uint8_t* wdata, uint8_t wlen, uint8_t* rdata, uint8_t rlen, bool* wflag)
{
  bool swwd = false;
  bool swrd = false;
  size_t saw, sar;
  size_t i, j;
  volatile uint8_t ret = NOERR;

  uint16_t* wwdata = NULL;  // create pointer to write word data, required if databits = 16
  uint16_t* wrdata = NULL;  // create pointer to read word data, required if databits = 16

  if (uspi.status == 0)
  {
    return SPI_NOT_ENABLED;
  }

  if (uspi.databits > 8)
  {                                        // if we need to write data using word size
    wrdata = (uint16_t*)(uintptr_t)rdata;  // adjust pointer to word data
    wwdata = (uint16_t*)(uintptr_t)wdata;  // adjust pointer to word data
    // swap byte due to endianess
    for (size_t i = 0; i < wlen; ++i)
    {
      wwdata[i] = (wwdata[i] << 8) | (wwdata[i] >> 8);
    }
    wlen = wlen / 2;  // Adjust length to word
  }

  if (wlen > 0 && rlen == 0)
  {  // if we need to perform write only
    if (uspi.databits <= 8)
    {
      ret = spi_bytes(SPIW, wdata, wlen, rdata, rlen);  //!< bytes command
    }
    else
    {
      ret = spi_word(SPIW, wwdata, wlen, wrdata, rlen);  //!< word command
    }
  }

  if (wlen > 0 && rlen > 0)
  {  // if some data to write and read
    if (uspi.databits <= 8)
    {
      ret = spi_bytes(SPIWR, wdata, wlen, rdata, rlen);  //!< bytes command
    }
    else
    {
      ret = spi_word(SPIWR, wwdata, wlen, wrdata, rlen);  //!< word command
    }
  }

  if (wlen == 0 && rlen > 0)
  {  // if some data to  read
    if (uspi.databits <= 8)
    {
      ret = spi_bytes(SPIR, wdata, wlen, rdata, rlen);  //!< bytes command
    }
    else
    {
      ret = spi_word(SPIR, wwdata, wlen, wrdata, rlen);  //!< word command
    }
  }

  //  This section remove from the read array the data read during write
  //  by moving the data read  to the beginning of array
  if (wlen > 0)
  {
    for (size_t i = 0; i < wlen + rlen; ++i)
    {  // Bring data to beginning of array
      if (uspi.databits > 8)
      {                                  // if word size
        wrdata[i] = (wrdata[i + wlen]);  // move word data
      }
      else
      {
        rdata[i] = (rdata[i + wlen]);  // move byte data
      }
    }
  }

  // if (uspi.databits >8 ) {  // if word read, transform to byte and save in rdata
  //     spi_words_to_bytes(wrdata, rdata,rlen*2+wlen );
  //}

  *wflag = (uspi.databits <= 8) ? false : true;  //!< set flag to determine size of data read
                                                 // free(wrdata);
                                                 // free(wwdata);

  return ret;
}
