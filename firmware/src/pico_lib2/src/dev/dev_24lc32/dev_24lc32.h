/*
 * Copyright (c) 2022, Mezael Docoy
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

#ifndef AT24CX_I2C
#define AT24CX_I2C

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "../../sys/include/sys_i2c.h"


/**
 * @brief Error codes for the AT24CX series EEPROM operations.
 */
typedef enum {
    AT24CX_ERR = -1,                 /**< General error. */
    AT24CX_OK,                       /**< Operation successful. */
    AT24CX_NOT_DETECTED,             /**< Device not detected on the I2C bus. */
    AT24CX_INVALID_ADDRESS,           /**< Invalid address provided for access. */
    AT24CX_INVALID_PAGEWRITE_ADDRESS  /**< Invalid address for page write operation. */
} at24cx_err_t;

/**
 * @brief Structure for writing data to the AT24CX EEPROM.
 */
typedef struct {
    uint8_t data;                    /**< Single byte of data to write. */
    uint8_t data_multi[128];         /**< Array for multiple bytes of data to write. */
    uint16_t address;                /**< Address in the EEPROM where data will be written. */
} at24cx_writedata_t;

/**
 * @brief Structure representing the AT24CX EEPROM device characteristics.
 */
typedef struct {
    uint8_t status : 1;              /**< Status of the device (e.g., ready or busy). */
    uint32_t byte_size;              /**< Total byte size of the EEPROM device. */
    uint16_t page_write_size;        /**< Size of each page that can be written to. */
    uint8_t i2c_addres;             /**< I2C address of the EEPROM device. */
    uint16_t dev_chip;               /**< Chip identifier for the EEPROM device. */
} at24cx_dev_t;



/**
 * @brief AT24CX device address.
 * @details AT24CX I2C slave address.
 */
#define I2C_ADDRESS_AT24CX      0x50

/**
 * @brief AT24CX device write delay.
 * @details AT24CX self-time write cycle.
 */
#define AT24CX_WRITE_CYCLE_DELAY    5

/**
 * @brief Register device.
 * @details Register device based on specification.
*/
void at24cx_i2c_device_register(at24cx_dev_t *dev, uint16_t _dev_chip, uint8_t _i2c_addres);

/**
 * @brief Write word to device.
 * @details Read word to AT24CX.
*/
at24cx_err_t at24cx_i2c_byte_write(at24cx_dev_t dev, at24cx_writedata_t dt);

/**
 * @brief Write multi word to device.
 * @details Write 128 bytes to AT24CX.
*/
at24cx_err_t at24cx_i2c_page_write(at24cx_dev_t dev, at24cx_writedata_t dt);

/**
 * @brief Read word from device.
 * @details Read word from AT24CX.
*/
at24cx_err_t at24cx_i2c_byte_read(at24cx_dev_t dev, at24cx_writedata_t *dt);

/**
 * @brief Read from device.
 * @details Read word from current address of AT24CX.
*/
at24cx_err_t at24cx_i2c_current_address_read(at24cx_dev_t dev, at24cx_writedata_t *dt);

#ifdef __cplusplus
}
#endif

#endif /* AT24CX_I2C */
