/**
 * @file    scpi_uart.h
 * @author  Daniel Lockhead
 * @date    2024
 * 
 * @brief   Header file defining constants and macros for the uart communication.
 *   
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */
 

#ifndef _SCPI_UART_H_
#define _SCPI_UART_H_

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @brief Pin definitions for user communication.
 * 
 * These macros define the GPIO pins used for 
 * user serial communication.
 */
#define USER_TX_PIN 12  //!< Transmit pin for UART communication
#define USER_RX_PIN 13  //!< Receive pin for UART communication
#define USER_CTS_PIN 14  //!< Clear To Send pin (flow control)
#define USER_RTS_PIN 15  //!< Request To Send pin (flow control)

#define  USER_UART uart0 //!< uart for the user communication

/**
 * @brief Default configuration values for user UART.
 * 
 * These macros define the default settings for UART communication.
 */
#define DEF_USER_BAUD      115200    //!< Default baud rate for UART communication
#define DEF_USER_PARITY    UART_PARITY_NONE //!< Default parity setting (no parity)
#define DEF_USER_BITS      8          //!< Default number of data bits
#define DEF_USER_STOP      1          //!< Default number of stop bits
#define DEF_USER_HANDSHAKE 0          //!< Default handshake setting (no handshake)
#define DEF_USER_STATUS    0          //!< Default status (disabled)
#define DEF_TIMEOUT_MS     1000       //!< Default timeout duration in milliseconds
#define DEF_LASTCHAR       '\n'       //!< Default last character for received strings

/**
 * @brief Error codes for UART communication.
 * 
 * This set of defines represents various error conditions that may arise during UART operations.
 */
#define UART_PROT_NUM_NOTVALID      31   //!< Error code indicating an invalid protocol number
#define UART_PROT_LETTER_NOVALID    32   //!< Error code indicating a non-valid protocol letter
#define UART_PROT_MISSING           33   //!< Error code indicating a missing protocol
#define NOCERR                       0    //!< No communication error (success)
#define UART_RX_TIMEOUT_MS          34   //!< Error code indicating a timeout during RX
#define UART_LASTCHAR_TIMEOUT_MS    35   //!< Error code indicating a timeout waiting for the last character
#define UART_BUFFER_FULL            36   //!< Error code indicating the UART buffer is full
#define UART_NOT_ENABLED            37   //!< Error code indicating that UART is not enabled

void scpi_uart_enable(void);
void scpi_uart_disable(void);
bool scpi_uart_status(void);
void scpi_uart_set_baudrate(uint32_t speed);
uint32_t scpi_uart_get_baudrate(void); 
void scpi_uart_set_timeout(uint32_t time);
uint32_t scpi_uart_get_timeout(void); 
void scpi_uart_set_handshake(bool val);
bool scpi_uart_get_handshake(void);
uint8_t scpi_uart_set_protocol(char* str);
const char* scpi_uart_get_protocol(void);
uint8_t scpi_uart_write_data(char* dwrt);
uint8_t scpi_uart_write_read_data(char* strd, char* dpr,size_t rsize); // write data, expect answer


#ifdef __cplusplus
}
#endif

#endif   // _SCPI_UART_H_