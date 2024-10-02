/**
 * @file    sys_i2c.h
 *
 * @brief   I2C Functions Module
 * 
 * @details This module provides functions for I2C communication, including:
 *   - Initialization.
 *   - Reading data from I2C devices.
 *   - Writing data to I2C devices.
 * 
 * @copyright Copyright 2021 Ocean (iiot2k@gmail.com) All rights reserved
 *            Copyright (c) 2024, D.Lockhead. All rights reserved.
 * 
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */


/**************************************************************************/
/*! 
    @file     sys_i2c.h
    @author   Daniel Lockhead
    @date     2024
    
    @brief    Modifications on file 
    
    @section I2C MODIFICATIONS

    - fix documentation errors found by Doxygen
**************************************************************************/


#ifndef _SYS_I2C_H_
#define _SYS_I2C_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

/*! $## **Constants:**
    @--
*/

/*!
    @def I2C_TIMEOUT_CHAR
    @brief I2C timeout per character.
    @details This defines the timeout duration for I2C operations per character in microseconds.
    @n
*/
#define I2C_TIMEOUT_CHAR 500


/*!
    @def SYS_SDA0
    @brief Default I2C0 SDA pin.
    @details This defines the SDA pin used for I2C0 communication.
    @n
*/
#define SYS_SDA0 8

/*!
    @def SYS_SCL0
    @brief Default I2C0 SCL pin.
    @details This defines the SCL pin used for I2C0 communication.
    @n
*/
#define SYS_SCL0 9

/*!
    @def SYS_SDA1
    @brief Default I2C1 SDA pin.
    @details This defines the SDA pin used for I2C1 communication.
    @n
*/
#define SYS_SDA1 6

/*!
    @def SYS_SCL1
    @brief Default I2C1 SCL pin.
    @details This defines the SCL pin used for I2C1 communication.
    @n
*/
#define SYS_SCL1 7


/*! $## **I2C Setup:** 
    @--
    @n
*/

/*! @brief - Set i2c baudrate
    @param i2c I2C channel i2c0 or i2c1
    @param baudrate Baudrate in Hz
*/
void sys_i2c_setbaudrate(i2c_inst_t* i2c, uint32_t baudrate);

/*! @brief - Init i2c with user assignment pins
    @param i2c I2C channel i2c0 or i2c1
    @param sda Pin number for sda
    @param scl Pin number for scl
    @param baudrate Baudrate in Hz
    @param pullup If true connect internal pullup resistor
*/
void sys_i2c_init(i2c_inst_t* i2c, uint32_t sda, uint32_t scl, uint32_t baudrate, bool pullup);

/*! @brief - Init default i2c
    @param i2c I2C channel i2c0 or i2c1
    @param baudrate Baudrate in Hz
    @param pullup If true connect internal pullup resistor
*/
void sys_i2c_init_def(i2c_inst_t* i2c, uint32_t baudrate, bool pullup);

/*! $## **Byte functions:**
    @--
    @n
*/

/*! @brief - Read i2c byte
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param rb Byte to read
    @return Bytes read
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_rbyte(i2c_inst_t* i2c, uint8_t addr, uint8_t* rb);


/*! @brief - Write i2c byte
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param wb Byte to write
    @return Bytes written
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_wbyte(i2c_inst_t* i2c, uint8_t addr, uint8_t wb);

/*! @brief - Read i2c byte register
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param reg Register address
    @param rb Byte to read
    @return Bytes read
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_rbyte_reg(i2c_inst_t* i2c, uint8_t addr, uint8_t reg, uint8_t* rb);

/*! @brief - Write i2c byte register
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param reg Register address
    @param wb Byte to write
    @return Bytes written
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_wbyte_reg(i2c_inst_t* i2c, uint8_t addr, uint8_t reg, uint8_t wb);


/*! @brief - Read i2c buffer by byte
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param pBuf Buffer to read
    @param len Length to read
    @return Bytes read
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_rbuf(i2c_inst_t* i2c, uint8_t addr, uint8_t* pBuf, uint32_t len);

/*! @brief - Write i2c buffer
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param pBuf Buffer to write
    @param len Length to write
    @return Bytes written
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_wbuf(i2c_inst_t* i2c, uint8_t addr, const uint8_t* pBuf, uint32_t len);


/*! @brief - Read i2c buffer register using byte
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param reg Register address
    @param pBuf Buffer to read
    @param len Length to read
    @return Bytes read
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_rbuf_reg(i2c_inst_t* i2c, uint8_t addr, uint8_t reg, uint8_t* pBuf, uint32_t len);


/*! @brief - Write i2c buffer register and byte data
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param reg Register address
    @param pBuf Buffer to read
    @param len Length to read
    @return Bytes written
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_wbuf_reg(i2c_inst_t* i2c, uint8_t addr, uint8_t reg, uint8_t* pBuf, uint32_t len);

/*! @brief - Write i2c buffer and read byte data
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param wBuf Write Buffer
    @param wlen Length to write
    @param rBuf Buffer to read
    @param rlen Length to read
    @return Bytes read
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_wbuf_rbuf(i2c_inst_t* i2c, uint8_t addr, uint8_t* wBuf, uint32_t wlen,uint8_t* rBuf, uint32_t rlen);

/*! @brief - Read i2c byte on eeprom  at word address
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address
    @param ee_address Word address in array
    @param pBuf Buffer to write
    @param len Length to write
    @return Bytes read
    @return[error] PICO_ERROR_GENERIC On error
    @return[error] PICO_ERROR_TIMEOUT On timeout
*/
int32_t sys_i2c_rbyte_eeprom(i2c_inst_t* i2c, uint8_t addr, uint8_t* ee_address, uint8_t* pBuf, uint32_t len);


#ifdef __cplusplus
}
#endif

#endif   // _SYS_I2C_H_