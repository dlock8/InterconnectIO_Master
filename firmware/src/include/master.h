/**
 * @file    master.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header file defining constants and macros used on the main file
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef _MASTER_H_
#define _MASTER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief maximum time before watchdog trig
 *
 */
#define WATCHDOG_TIMEOUT 5000 /**< Watchdog set for 5 seconds. */

/**
 * @brief UART configuration and default settings.
 *
 * This group contains default values and pin assignments for UART configuration.
 */
#define UART_ID uart1           /**< UART instance used. */
#define BAUD_RATE 115200        /**< Baud rate for UART communication. */
#define DATA_BITS 8             /**< Number of data bits. */
#define STOP_BITS 1             /**< Number of stop bits. */
#define PARITY UART_PARITY_NONE /**< Parity setting (None). */

/**
 * @brief UART GPIO pin configuration.
 *
 * We are using GPIO pins 0 and 1 for UART transmission and reception, but other pins can be used as described in the datasheet.
 */
#define UART_TX_PIN 8 /**< GPIO pin for UART TX (Transmit). */
#define UART_RX_PIN 9 /**< GPIO pin for UART RX (Receive). */

/**
 * @brief Redefine default UART pin configuration for PICO.
 *
 * During development, printf output is sent to both USB and serial port. In normal operation, the serial port is used for SCPI communication only.
 */
#undef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 9 /**< Default RX pin for UART on PICO. */

#undef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 8 /**< Default TX pin for UART on PICO. */

#undef PICO_DEFAULT_UART_BAUD_RATE
#define PICO_DEFAULT_UART_BAUD_RATE 115200 /**< Default baud rate for UART on PICO. */

/**
 * @brief EEPROM parameters supported by SCPI commands.
 *
 * These parameters can be accessed and modified via SCPI commands.
 */
#define CHECK "CHECK"                 /**< EEPROM Check character. */
#define PARTNUMBER "PARTNUMBER"       /**< EEPROM Part number. */
#define SERIALNUMBER "SERIALNUMBER"   /**< EEPROM Serial number. */
#define MOD_OPTION "MOD_OPTION"       /**< EEPROM Module options. */
#define COM_SER_SPEED "COM_SER_SPEED" /**< EEPROM Communication serial speed. */
#define PSLAVE_RUN "PICO_SLAVES_RUN"  /**< EEPROM Pico slaves run status. */
#define TESTBOARD_NUM "TESTBOARD_NUM" /**< EEPROM Test board part number. */

  /**
   * @brief EEPROM configuration structure.
   *
   * This structure contains the default values and settings that are stored in EEPROM.
   * New parameters should be added with proper size allocation, and always include
   * a null terminator (`+1`) for strings.
   */
  typedef struct eecfg_t
  {
    char check[1];           /**< Proof character to validate EEPROM read. */
    char partnumber[13];     /**< Part number stored in EEPROM. */
    char serialnumber[6];    /**< Serial number stored in EEPROM. */
    char mod_option[15];     /**< Module options stored in EEPROM. */
    char com_ser_speed[7];   /**< Communication serial speed. */
    char slave_force_run[2]; /**< Force master RUN_EN = 1, useful for debugging slave Pico devices. */
    char testboard_num[13];  /**< Part number of the self-test board. */
  } cfg;

  /**
   * @brief Union that combines the EEPROM configuration structure with a raw data array.
   */
  typedef union eef
  {
    struct eecfg_t cfg;     /**< EEPROM configuration structure. */
    char data[sizeof(cfg)]; /**< Raw data representation of the configuration. */
  } eep;                    /**< Instance of the eef union used for EEPROM operations. */

  /**
   * @var ee
   * @brief Instance of the EEPROM configuration union.
   *
   * This is a global instance of the `eep` union, providing access to the EEPROM configuration.
   * It can be used to read or write EEPROM data either by individual fields (via `cfg`) or as
   * a raw byte array (via `data[]`).
   */
  extern eep ee; /**< External declaration of EEPROM union for global use. */

/**
 * @brief Default value for EEPROM structure.
 *
 * This section defines the default values assigned to the EEPROM structure. Each parameter has a specific role in the system configuration.
 */
#define EE_CHECK_CHAR '#'               /**< EEPROM check character */
#define EE_PARTNUMBER "500-1000-010"    /**< Part number of the device */
#define EE_SERIALNUMBER "00001"         /**< Serial number of the device */
#define EE_MOD_OPTION "DAC,PWR"         /**< Module options (e.g., DAC, Power) */
#define EE_CSER_SPEED "115200"          /**< Communication serial speed (e.g., baud rate) */
#define EE_SLAVE_RUN "0"                /**< Slave run status (0 = Disable, 1 = Enable) */
#define EE_TESTBOARD_NUM "500-1010-020" /**< Part number of the self-test board */

/**
 * @brief Default values for the EEPROM structure.
 *
 * This macro assigns default values to the EEPROM structure.
 * New parameters can be added at the end of the list as needed.
 */
#define DEF_EEPROM                                                                                                                                   \
  {                                                                                                                                                  \
    EE_CHECK_CHAR,       /**< EEPROM Check Character */                                                                                              \
        EE_PARTNUMBER,   /**< EEPROM Part Number */                                                                                                  \
        EE_SERIALNUMBER, /**< EEPROM Serial Number */                                                                                                \
        EE_MOD_OPTION,   /**< Module Option */                                                                                                       \
        EE_CSER_SPEED,   /**< Communication Serial Speed */                                                                                          \
        EE_SLAVE_RUN,    /**< Slave Run Flag */                                                                                                      \
        EE_TESTBOARD_NUM /**< Test Board Number */                                                                                                   \
  }

  //!< Add parameter on struct ParamInfo  file fts_scpi.c  around line 990

#define BEEP_I2C_FAIL 3   //!< Define burst of Beep
#define BEEP_EEP_FAIL 4   //!< Define burst of Beep
#define BEEP_VSYS_OUT 2   //!< Define burst of Beep
#define BEEP_TEMP_HIGH 1  //!< Define burst of Beep
#define BEEP_WATCHDOG 5   //!< Define burst of Beep

#define MAX_PICO_TEMP 60  //!< Maximum temperature acceptable for pico
#define MAX_VSYS_VOLT 6   //!< Maximum voltage to Pico Master Vsys
#define MIN_VSYS_VOLT 4   //!< Minimum voltage to Pico Master Vsys

#define DEFAULT_PWR_VAL 0xc1f  //!< Value of configuration register after ina219_init

  void Hardware_Default_Setting();

#endif  //!<