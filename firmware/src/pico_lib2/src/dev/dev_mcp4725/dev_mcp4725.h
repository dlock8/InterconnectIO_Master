/**
 * @file    dev_mcp4725.h
 *
 * 
 * @brief   MCP4725 One Channel 12-bit ADC
 * 
 * @details The MCP4725 is a 12-bit digital to analog converter with one output.
 *           The chip can connected direct to Pico with supply voltage 3.3V.
 *           Maximum output voltage is VDD (3.3V).
 * 
 * @copyright Copyright 2021 Ocean (iiot2k@gmail.com) All rights reserved
 *            Copyright (c) 2024, D.Lockhead. All rights reserved.
 * 
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef _DEV_MCP4725_H_
#define _DEV_MCP4725_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "../../sys/include/sys_i2c.h"


/*! 
    @def VDD
    @brief Source Voltage.
    This value can be calibrated during self-test and saved in EEPROM.
 */
#define VDD 3.3

/*! 
    @brief I2C addresses for the MCP4725.
    
    @details 
    The MCP4725 can have two different I2C addresses based on the connection of the A0 pin:
    - MCP4725_ADDR0: I2C address (0x60) when A0 is connected to GND or left open.
    - MCP4725_ADDR1: I2C address (0x61) when A0 is connected to VDD.
*/
enum {
    MCP4725_ADDR0 = 0x60,  ///< I2C address for MCP4725 when A0 is connected to GND or open.
    MCP4725_ADDR1 = 0x61   ///< I2C address for MCP4725 when A0 is connected to VDD.
};



/*! 
    @brief Power down modes for the MCP4725.
    
    @details 
    The MCP4725 can operate in different power down modes:
    - MCP4725_PD_OFF: Power down mode is off (device is fully operational).
    - MCP4725_PD_1: Power down mode with a 1k ohm resistor.
    - MCP4725_PD_100: Power down mode with a 100k ohm resistor.
    - MCP4725_PD_500: Power down mode with a 500k ohm resistor.
*/
enum {
    MCP4725_PD_OFF = 0,   ///< Power down mode is off (device operational).
    MCP4725_PD_1,        ///< Power down with a 1k ohm resistor.
    MCP4725_PD_100,      ///< Power down with a 100k ohm resistor.
    MCP4725_PD_500       ///< Power down with a 500k ohm resistor.
};

/*! $## **Functions:**
    @--
*/
/*! @brief - Sets power down for channel (active on set or save to dac)
    @brief Default power down is MCP4725_PD_OFF
    @param pd Power down settings MCP4725_PD_xxx
*/
void dev_mcp4725_pd(uint8_t pd);

/*! @brief - Sets adc output
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address MCP4725_ADDRn
    @param value Output value for channnel (0..4095)
    @return true
    @return[error] false
*/
bool dev_mcp4725_set_raw(i2c_inst_t* i2c, uint8_t addr, uint16_t value);


/*! 
    @brief Sets the ADC output for the MCP4725.
    
    @param i2c Pointer to the I2C instance (i2c0 or i2c1).
    @param addr I2C address of the MCP4725 (use MCP4725_ADDRn).
    @param volt Voltage output value for the channel (range: 0 - VDD).
    
    @return true if the operation was successful.
    @return false if there was an error.
*/
bool dev_mcp4725_set(i2c_inst_t* i2c, uint8_t addr, float volt);


/*! @brief - Sets adc output
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address MCP4725_ADDRn
    @return voltage value read
*/
float dev_mcp4725_get(i2c_inst_t* i2c, uint8_t addr);

/*! @brief - Sets adc output and save settings to EEPROM (50ms)
    @param i2c I2C channel i2c0 or i2c1
    @param addr I2C address MCP4725_ADDRn
    @param volt Output value for for channnel (0..4095)
    @return true
    @return[error] false
*/
bool dev_mcp4725_save(i2c_inst_t* i2c, uint8_t addr, float volt);

#ifdef __cplusplus
 }
#endif

#endif // _DEV_MCP4725_H_