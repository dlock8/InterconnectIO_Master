/*
 * Copyright (c) 2023 Daniel Lockhead
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SCPI_UART_H_
#define _SCPI_UART_H_

#ifdef __cplusplus
 extern "C" {
#endif


#define  USER_TX_PIN 12
#define  USER_RX_PIN 13
#define  USER_CTS_PIN 14
#define  USER_RTS_PIN 15

#define  USER_UART uart0

//!< Default value used for user uart
#define  DEF_USER_BAUD      115200
#define  DEF_USER_PARITY    UART_PARITY_NONE
#define  DEF_USER_BITS      8 
#define  DEF_USER_STOP      1
#define  DEF_USER_HANDSHAKE 0    // No handshake
#define  DEF_USER_STATUS    0    // disabled by default
#define  DEF_TIMEOUT_MS     1000   // default timeout time
#define  DEF_LASTCHAR       '\n'  // default last character of a string


//!< Error list for uart
#define UART_PROT_NUM_NOTVALID      31
#define UART_PROT_LETTER_NOVALID    32
#define UART_PROT_MISSING           33
#define NOCERR                      0
#define UART_RX_TIMEOUT_MS          34
#define UART_LASTCHAR_TIMEOUT_MS    35
#define UART_BUFFER_FULL            36
#define UART_NOT_ENABLED            37


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

#endif   // _SYS_UART_H_