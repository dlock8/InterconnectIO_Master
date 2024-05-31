/*-
 * Copyright (c) 2012-2013 Lutz Hoerl, Thorlabs GmbH
 *
 * All Rights Reserved
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   scpi_user_config.h
 * @date   Wed Aug 05 10:00:00 UTC 2015
 * 
 * @brief  User resp. device dependent SCPI Configuration
 * 
 * 
 */

#ifndef __SCPI_USER_CONFIG_H_
#define __SCPI_USER_CONFIG_H_

#ifdef   __cplusplus
extern "C" {
#endif

// #define SCPI_LINE_ENDING    "\r\n"    /*   use <CR><LF> carriage return + line feed as termination charcters */

#ifndef  USE_FULL_ERROR_LIST 
#define USE_FULL_ERROR_LIST  1
#endif

//#ifdef  SCPI_USER_CONFIG 
//#define SCPI_USER_CONFIG   1

// Added to use the custom list of errors
#ifndef  USE_USER_ERROR_LIST 
#define USE_USER_ERROR_LIST 1
#endif


#define LIST_OF_USER_ERRORS \
    X(SCPI_RELAYS_LIST_ERROR,   -185,  "Relays channel list error. single dimension only")   \
    X(RELAY_NUMBERING_ERROR,    -186,  "A Relay channel is invalid and raise an error")   \
    X(I2C_COMMUNICATION_ERROR,  -106,  "I2C internal communication error, run selftest")            \
    X(VSYS_OUT_LIMITS,          -302,  "Voltage VSYS of Master Pico is out limit, run selftest")   \
    X(TEMP_MASTER_HIGH,         -303,  "Internal Temperature of Master Pico is too High, run selftest")   \
    X(WATCHDOG_TRIG,            -304,  "Master Pico Watchdog Trig")   \
    X(SELFTEST_FAIL,            -305,  "Self-test failed")   \
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
    X(SPI_MODE_ERROR,           -384,  "SPI mode number is invalid, expect Mode value between 0 and 3") \
    X(SPI_CS_ERROR,             -385,  "SPI chipselect gpio number is invalid." ) \
    X(MEMORY_ALLOCATION_ERROR,  -386,  "Internal error, memory allocation raise error." ) \
    X(ARB_WORD_FORMAT_ERROR,    -387,  "Arbitrary block data length is odd, expect even number for word size." ) \
    X(I2C_GENERIC_ERROR,        -388,  "I2C Generic Error (Error not defined)." ) \
    X(I2C_TIMEOUT_ERROR,        -389,  "I2C Operation timed out." ) \
    X(I2C_ADDRESS_NACK_ERROR,   -390,  "I2C No acknowledgment from the addressed device (Address NACK)." ) \
    X(I2C_DATA_NACK_ERROR,      -391,  "I2C No acknowledgment after sending data.(Data NACK)" ) \
    X(I2C_BUS_ERROR,            -392,  "An error occurred on the I2C bus.(Bus Error)" ) \
 


// Definition of each bit of the Operation Condition Event Register (QER) 
#define OPER_BOOT_FAIL           0   // <1>
#define OPER_SELFTEST_FAIL       1   // <2>



// Definition of each bit of the Questionable Condition Register (QCN) 
#define QCR_EEP_READ_ERROR    0     // <1>
#define QCR_I2C_COM           1     // <2>
#define QCR_MTEMP_HIGH        2     // <4>
#define QCR_VSYS_OUTLIMIT     3     // <8>
#define QCR_WATCHDOG          4     // <16>





#ifdef   __cplusplus
}
#endif

#endif   /* #define __SCPI_USER_CONFIG_H_ */
