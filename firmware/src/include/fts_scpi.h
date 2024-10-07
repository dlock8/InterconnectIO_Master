/**
 * @file    fts_scpi.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header file defining constants and macros used for SCPI interpreter.
 *
 *
 * @copyright Copyright (c) 2021 Valentin Milea <valentin.milea@gmail.com> SPDX-License-Identifier: MIT
 *            Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef _FTS_SCPI_H_
#define _FTS_SCPI_H_

#include "scpi/scpi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Size of the SCPI input buffer.
 */
#define SCPI_INPUT_BUFFER_SIZE 256 /**< Size of the SCPI input buffer. */

/**
 * @brief Size of the SCPI error queue.
 */
#define SCPI_ERROR_QUEUE_SIZE 20 /**< Size of the SCPI error queue. */

/**
 * @brief "IDN?" fields. Their meanings are "suggestions" in the standard.
 */
#define SCPI_IDN1 "FirstTestStation" /**< Manufacturer. */
#define SCPI_IDN2 "InterconnectIO"   /**< Model. */
#define SCPI_IDN3 "2022A"            /**< Design Date. */
#define SCPI_IDN4 "1.0"              /**< Design Version. */

/**
 * @brief Maximum number of rows for relay matrix.
 */
#define MAXROW 36 /**< Maximum number of rows for relay matrix. */

/**
 * @brief Maximum number of columns.
 */
#define MAXCOL 1 /**< Maximum number of columns. */

/**
 * @brief Maximum number of dimensions.
 */
#define MAXDIM 1 /**< Maximum number of dimensions. */

#define RCLOSE 1   //!< Close relay tag
#define RCLEX 2    //!< Close Exclusive relay tag
#define ROPEN 3    //!< Open relay tag
#define ROPALL 4   //!< Open all relay tag
#define RSTATE 5   //!< Read relay state tag
#define BSTATE 6   //!< Read relay bank state tag
#define SESTATE 7  //!< Read relay single ended or reverse relay state tag
#define SECLOSE 8  //!< Close REVerse relay tag
#define SEOPEN 9   //!< Open REVerse relay tag

#define SDIR 10     //!< Set direction of digital IO
#define SBDIR 11    //!< Set direction of digital IO Bit
#define SOUT 12     //!< Set port digital Output
#define SBOUT 13    //!< Set bit on port digital Output
#define RIN 14      //!< Read port digital input
#define RBIN 15     //!< Read port bit  digital input
#define RDIR 16     //!< Read direction of digital IO port
#define RBDIR 17    //!< Read direction for a bit on a port
#define GPSDIR 18   //!< Set direction for a single GPIO on a particular device
#define GPRDIR 19   //!< Read direction for a single GPIO on a particular device
#define GPOUT 20    //!< Set output for a single gpio on a particular device
#define GPIN 21     //!< Read a single gpio on a particular device
#define GPSPAD 22   //!< Set digital IO PAD parameter
#define GPGPAD 23   //!< Read digital IO PAD parameter
#define PWCLOSE 24  //!< CLose power relay
#define PWOPEN 25   //!< OPen power relay
#define PWSTATE 26  //!< Read state of power relay
#define OCCLOSE 27  //!< Active Open Collector
#define OCOPEN 28   //!< Desactive Open Collector
#define OCSTATE 29  //!< Read state of Open Collector

#define SBEEP 50  //!< Send Beep pulse
#define SVER 51   //!< Read version of Pico Master and slave
#define SLERR 52  //!< Control of error led
#define SRUN 53   //!< Pico SLaves ON/OFF
#define SOE 54    //!< Digital Output Enable ON/OFF
#define GLERR 55  //!< Read error led
#define GRUN 56   //!< Read Pico SLaves RUN status
#define GOE 57    //!< Read status Output Enable ON/OFF
#define GSTA 58   //!< Read Slave Device status byte
#define STBR 59   //!< Run complete test with selftest board connected

#define SDAC 63   //!< Set DAC Voltage
#define WDAC 64   //!< Set DAC Voltage and save as default value
#define RADC0 65  //!< Read ADC0 Voltage
#define RADC1 66  //!< Read ADC1 Voltage
#define RADC3 67  //!< Read Vsys Voltage
#define RADC4 68  //!< Read Master Temperature

#define RPV 70  //!< Read Power monitor Voltage
#define RPS 71  //!< Read Power Shunt Voltage
#define RPI 72  //!< Read Power current in milliAmpere
#define RPP 73  //!< Read Power in milliWatt
#define CPI 74  //!< Calibrate current on power device

#define WEEP 78  //!< Write to eeprom
#define REEP 79  //!< Read from eeprom
#define WDEF 80  //!< Write default value to EEprom
#define RFUL 81  //!< Read complete data on Eeprom

#define W1W 84  //!< Write on 1-Wire devices
#define R1W 85  //!< Read on 1-Wire devices
#define C1W 86  //!< Check on 1-Wire devices

#define CIE 88  //!< Enable communication protocol
#define CID 89  //!< Disable communication protocol, set pin as GPIO
#define CRI 90  //!< Read status communication protocol

#define CSWD 100  //!< Write Data on user serial port, no answer
#define CSRD 101  //!< Write Data on user serial port and wait for the answer
#define CSWB 102  //!< Write user serial baudrate
#define CSRB 103  //!< Read user serial baudrate
#define CSWP 104  //!< Write user serial Protocol
#define CSRP 105  //!< Read user serial Protocol
#define CSWH 106  //!< Write user serial Handshake (RTS-CTS)
#define CSRH 107  //!< Read user serial Handshake
#define CSWT 108  //!< Write user serial Timeout
#define CSRT 109  //!< Read user serial Timeout

#define SPWD 111   //!< Write Data on SPI port, read data
#define SPRD 112   //!< Read Data  on SPI port
#define SPWF 113   //!< Write user Spi baudrate
#define SPRF 114   //!< Read user Spi baudrate
#define SPWDB 115  //!< Write user Spi databits
#define SPRDB 116  //!< Read user Spi databits
#define SPWM 117   //!< Write user Spi mode
#define SPRM 118   //!< Read user Spi mode
#define SPWCS 119  //!< Set gpio used for SPI chipselect
#define SPRCS 120  //!< Read Gpio used for SPI chipselect

#define ICWD 131   //!< Write Data on I2C port
#define ICRD 132   //!< Read Data  on I2C port
#define ICWA 133   //!< Set Device Address to use I2C port
#define ICRA 134   //!< Get Device Address to use I2C port
#define ICWF 135   //!< Write user I2C baudrate
#define ICRF 136   //!< Read user I2C baudrate
#define ICWDB 137  //!< Write user I2C databits
#define ICRDB 138  //!< Read user I2C databits

#define SCPI_BANK1 1     //!< Open BK1 relay tag
#define SCPI_BANK2 2     //!< Open BK2 relay tag
#define SCPI_BANK3 3     //!< Open BK3 relay tag
#define SCPI_BANK4 4     //!< Open BK4 relay tag
#define SCPI_BANK_ALL 5  //!< Open all relay tag
#define SCPI_LPR1 6      //!< Power relay
#define SCPI_LPR2 7      //!< Power relay
#define SCPI_HPR1 8      //!< Power relay
#define SCPI_SSR1 9      //!< Power Solid State relay
#define SCPI_OC1 10      //!< Open collector
#define SCPI_OC2 11      //!< Open collector
#define SCPI_OC3 12      //!< Open collector

#define SCPI_SERIAL 15  //!< Uart serial communication
#define SCPI_SPI 16     //!< SPI communication
#define SCPI_I2C 17     //!< I2C communication

#define GPIO_LPR1 8  //!< Gpio number to control Power relay
#define GPIO_LPR2 9  //!< Gpio number to control Power relay
#define GPIO_HPR1 8  //!< Gpio number to control Power relay
#define GPIO_SSR1 9  //!< Gpio number to control Power relay

#define GPIO_OC1 28  //!< Gpio number to open collector 1
#define GPIO_OC2 28  //!< Gpio number to open collector 2
#define GPIO_OC3 28  //!< Gpio number to open collector 3

#define GPIO_BEEP 11  //!< Gpio number used to drive the beeper
#define GPIO_RUN 18   //!< Gpio number used to control RUN of the slave
#define GPIO_LED 19   //!< Gpio number used to led monitor green
#define GPIO_SYNC 22  //!< Gpio number used to sync all module
#define GPIO_OE 28    //!< Gpio number to signal Ouput enable

#define BEEP_TIME 10  //!< Beep time in mS

#define ESR_USER_BIT 6  //!< User Request Bit position on ESR
#define ESR_PON_BIT 7   //!< Power ON Bit position on ESR

//!< Error
#define ARB_ODD_ERR 1  //!< Error on  Arbitrary block for word defintion

  /* Reset Block for master

  [Block to reset]	[Bit]
      USB	            24
      UART 1	        23
      UART 0	        22
      Timer	        21
      TB Manager	    20
      SysInfo	        19
      System Config	18
      SPI 1	        17
      SPI 0	        16
      RTC	            15
      PWM	            14
      PLL USB	        13
      PLL System	    12
      PIO 1	        11
      PIO 0	        10
      Pads - QSPI	    9
      Pads - bank 0	8
      JTAG	        7
      IO Bank 1	    6
      IO Bank 0	    5
      I2C 1	        4
      I2C 0	        3
      DMA	            2
      Bus Control	    1
      ADC 0	        0
  */

#define RESETS_MASTER 0b000000000000000111111111  //!<  Bitmask to reset the master.

  /**
   * @brief Enumeration for error indices used to set or clear errors in registers.
   *
   * This enumeration defines various error conditions that can be monitored
   * by the system, allowing for appropriate error handling.
   */
  typedef enum _reg_info_index_t
  {
    BOOT_I2C = 0,      /**< Boot I2C Error */
    VSYS_OUT,          /**< Vsys out of limits */
    MTEMP_HIGH,        /**< Master temperature too high */
    WATCH_TRIG,        /**< Watchdog trigger */
    EEPROM_ERROR       /**< EEPROM access error or corruption */
  } reg_info_index_t;  ///< Typedef for the enumeration

  /**
   * @typedef reg_info_index_t
   * @brief Typedef for the enumeration _reg_info_index_t.
   */
  typedef enum _reg_info_index_t reg_info_index_t;  ///< Typedef for the enumeration

  extern scpi_t scpi_context;
  extern char scpi_input_buffer[];
  extern scpi_error_t scpi_error_queue[];
  extern scpi_command_t scpi_commands[];

  // size_t write_scpi(scpi_t *context, const char *data, size_t len);
  scpi_result_t Relay_Chanlst(scpi_t* context, uint16_t* array);
  void init_scpi();
  void ErrorBeep(uint8_t nbeep);
  void RegBitHdwrErr(reg_info_index_t index, bool scbit);
  static size_t output_buffer_write(const char* data, size_t len);

#endif  //!<