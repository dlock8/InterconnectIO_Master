/**
 * @file    scpi_spi.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header file defining constants and macros for the SPI communication.
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef _SCPI_SPI_H_
#define _SCPI_SPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief User-defined SPI pin configurations.
 */
#define USER_SPI_SCK_PIN 2 /**< SPI Clock Pin. */
#define USER_SPI_TX_PIN 3  /**< SPI Transmit Pin (MOSI). */
#define USER_SPI_RX_PIN 4  /**< SPI Receive Pin (MISO). */
#define USER_SPI_CSN_PIN 5 /**< SPI Chip Select Pin. */

/**
 * @brief Buffer length for SPI transactions.
 */
#define SPI_BUF_LEN 8 /**< Length of the SPI buffer in bytes. */

#define DEF_SPI_USER spi0      //!< spi0 is the user communication channel to external
#define DEF_SPI_BAUD 1E5       //!< spi baudrate is set at 100Khz by default
#define DEF_SPI_DATABITS 8     //!< spi number of bits used for comunication
#define DEF_SPI_CS 5           //!< spi gpio used to perform chipselect
#define DEF_SPI_MODE 0         //!< define mode parameters: cs,cpol,cpha and MSB
#define DEF_SPI_STATUS 0       //!< spi is disable by default
#define ALARM_TIMEOUT 1000000  //!< Timeout value

#define SPI_DEFAULT_VALUE 0  //!< spi data to write when we perform read

#define SPIW 0   //!< mode = SPI write
#define SPIWR 1  //!< mode = SPI write-read
#define SPIR 2   //!< mode = SPI read

/**
 * @brief Error code definitions for SPI communication and other errors.
 */
#define NOERR 0                  /**< No error. */
#define SPI_MODE_NUM_NOTVALID 40 /**< Invalid SPI mode number. */
#define SPI_CS_NUM_ERROR 41      /**< Invalid Chip Select (CS) number for SPI. */
#define MALLOC_FAILURE 42        /**< Memory allocation failure. */
#define SPI_TIMEOUT 43           /**< SPI communication timeout. */
#define SPI_NOT_ENABLED 44       /**< SPI not enabled error. */

  void scpi_spi_enable(void);
  void scpi_spi_disable(void);
  bool scpi_spi_status(void);
  void scpi_spi_set_baudrate(uint32_t speed);
  uint32_t scpi_spi_get_baudrate(void);
  uint8_t scpi_spi_set_chipselect(uint32_t num);
  uint32_t scpi_spi_get_chipselect(void);

  uint8_t scpi_spi_set_databits(uint32_t num);
  uint32_t scpi_spi_get_databits(void);
  uint8_t scpi_spi_set_mode(uint8_t mode);
  uint8_t scpi_spi_get_mode(void);

  uint8_t scpi_spi_wri_read_data(uint8_t* wdata, uint8_t wlen, uint8_t* rdata, uint8_t rlen, bool* wflag);
  void spi_bytes_to_words(uint8_t* byte_array, uint16_t* word_array, size_t length);
  void spi_words_to_bytes(uint16_t* word_array, uint8_t* byte_array, size_t length);

#ifdef __cplusplus
}
#endif

#endif  // _SYS_SPI_H_
