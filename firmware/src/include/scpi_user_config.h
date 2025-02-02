/**
 * @file    scpi_user_config.h
 * @author  Daniel Lockhead
 * @date    2024
 * 
 * @brief   User responsible device dependent SCPI Configuration
 *   
 *
 * @copyright Copyright (c) 2012-2013 Lutz Hoerl, Thorlabs GmbH
 *            Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef __SCPI_USER_CONFIG_H_
#define __SCPI_USER_CONFIG_H_

#ifdef   __cplusplus
extern "C" {
#endif

#define SCPI_LINE_ENDING    LINE_ENDING_LF  /*   use <CR><LF> carriage return + line feed as termination characters */



#ifndef  USE_FULL_ERROR_LIST 
#define USE_FULL_ERROR_LIST  1  ///< Enables the full list of error codes.
#endif

#ifndef  SCPI_USER_CONFIG 
#define SCPI_USER_CONFIG   1
#endif

// Added to use the custom list of errors
#ifndef  USE_USER_ERROR_LIST 
#define USE_USER_ERROR_LIST 1 ///< Enables the user list of error codes.
#endif


/**
 * @brief Macro to define user error codes.
 * 
 * Each error code consists of an error identifier, 
 * an associated error number, and a descriptive message.
 */
#define LIST_OF_USER_ERRORS \
    X(SCPI_RELAYS_LIST_ERROR,   -185,  "Relays channel list error. single dimension only")   \
    X(RELAY_NUMBERING_ERROR,    -186,  "A Relay channel is invalid and raise an error")   \
    X(I2C_COMMUNICATION_ERROR,  -106,  "I2C internal communication error, run selftest")            \
    X(VSYS_OUT_LIMITS,          -302,  "Voltage VSYS of Master Pico is out limit, run selftest")   \
    X(TEMP_MASTER_HIGH,         -303,  "Internal Temperature of Master Pico is too High, run selftest")   \
    X(WATCHDOG_TRIG,            -304,  "Master Pico Watchdog Trig")   \
    X(SELFTEST_FAIL,            -305,  "Selftest failed")   \
    X(NB_ONEWIRE,               -371,  "Number of Onewire devices is different of expected number")  \
    X(STR_ONEWIRE,              -372,  "The Write data is different of the Read data on Onewire devices")  \
    X(WR_ONEWIRE,               -373,  "Error raise during Write - Read on Onewire devices")  \
    X(WRITE_ONEWIRE,            -374,  "Error raise during Write action on Onewire devices")  \
    X(NO_ONEWIRES,              -375,  "Presence of Onewire devices has not been detected") \
    X(READ_ONEWIRES,            -376,  "Error raise during read of Onewire devices") \
    X(HEX_VALIDID,              -377,  "Onewire device ID contains illegal character (hexadecimal only)") \
    X(UART_NUMBER_ERROR,        -378,  "Uart protocol number is invalid, check value for Data_bits or Stop_bits") \
    X(UART_LETTER_ERROR,        -379,  "Uart protocol letter is invalid, check letter used to define parity (expected: O,N,E)") \
    X(UART_PROTOCOL_ERROR,      -380,  "Uart protocol value is invalid, expect 3 characters on any orders ex: 8N1,7O1") \
    X(UART_RX_ERROR,            -381,  "Uart serial communication error, Timeout occur on waiting to receive char") \
    X(UART_LASTCHAR_ERROR,      -382,  "Uart Timeout occur on waiting to receive last character of string (last char is the same of Tx)") \
    X(UART_RXBUFFER_ERROR,      -383,  "Uart Receiver buffer overrun. String received too long") \
    X(UART_ENABLE_ERROR,        -384,  "Uart Serial communication not enabled, send command to enable SERIAL" ) \
    X(SPI_MODE_ERROR,           -385,  "SPI mode number is invalid, expect Mode value between 0 and 7") \
    X(SPI_CS_ERROR,             -386,  "SPI chipselect gpio number is invalid." ) \
    X(SPI_TIMEOUT_ERROR,        -387,  "SPI timeout error (no device?)" ) \
    X(SPI_ENABLE_ERROR,         -388,  "SPI master not enabled, send command to enable SPI" ) \
    X(MEMORY_ALLOCATION_ERROR,  -389,  "Internal error, memory allocation raise error." ) \
    X(ARB_WORD_FORMAT_ERROR,    -390,  "Arbitrary block data length is odd, expect even number for word size." ) \
    X(I2C_GENERIC_ERROR,        -391,  "I2C Generic Error (Error not defined)." ) \
    X(I2C_TIMEOUT_ERROR,        -392,  "I2C Operation timed out." ) \
    X(I2C_ADDRESS_NACK_ERROR,   -393,  "I2C No acknowledgment from the addressed device (Address NACK)." ) \
    X(I2C_DATA_NACK_ERROR,      -394,  "I2C No acknowledgment after sending data.(Data NACK)" ) \
    X(I2C_BUS_ERROR,            -395,  "An error occurred on the I2C bus.(Bus Error)" ) \
    X(I2C_ENABLE_ERROR,         -396,  "I2C master not enabled, send command to enable I2C" ) \


// Definition of each bit of the Operation Condition Event Register (QER) 
#define OPER_BOOT_FAIL           0  ///< Bit 0: Indicates a boot failure.
#define OPER_SELFTEST_FAIL       1  ///< Bit 1: Indicates a self-test failure.

// Definition of each bit of the Questionable Condition Register (QCN) 
#define QCR_EEP_READ_ERROR      0  ///< Bit 0: Indicates an EEPROM read error.
#define QCR_I2C_COM             1  ///< Bit 1: Indicates an I2C communication error.
#define QCR_MTEMP_HIGH          2  ///< Bit 2: Indicates high temperature condition.
#define QCR_VSYS_OUTLIMIT       3  ///< Bit 3: Indicates voltage supply is out of limits.
#define QCR_WATCHDOG            4  ///< Bit 4: Indicates watchdog timer event.




#ifdef   __cplusplus
}
#endif

#endif   /* #define __SCPI_USER_CONFIG_H_ */
