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
    X(SCPI_RELAYS_LIST_ERROR,           -185,  "Relays channel list error. single dimension only")   \
    X(RELAY_NUMBERING_ERROR,           -186,  "A Relay channel is invalid and raise an error")   \
    X(I2C_COMMUNICATION_ERROR,      -301,  "I2C internal communication error, run selftest")            \
    X(VSYS_OUT_LIMITS,              -302,  "Voltage VSYS of Master Pico is out limit, run selftest")   \
    X(TEMP_MASTER_HIGH,             -303,  "Internal Temperature of Master Pico is too High, run selftest")   \
    X(WATCHDOG_TRIG,                -304,  "Master Pico Watchdog Trig")   \
    X(SELFTEST_FAIL,                -305,  "Self-test failed")   \
    X(EEPROM_NOT_DETECTED,          -601,  "Cfg EEprom not detected")   \
    X(EEPROM_NUMBER_CHECK,          -602,  "Cfg EEprom data corrupt")   \
    X(EEPROM_BYTE_ERROR,            -603,  "Cfg EEprom byte error")   \
    X(EEPROM_BYTE_WRITE_ERROR,      -604,  "Cfg EEprom byte write error at address 0")   \
    X(EEPROM_BYTE_READ_ERROR,       -605,  "Cfg EEprom byte read error at address 0")   \
    X(EEPROM_BYTE_WR_ERROR,         -606,  "Cfg EEprom error, byte write is not equal to byte read at address 0")   \
    X(PICO_STATUS_SLAVE1,           -607,  "Pico Slave1 device status returned an error")   \
    X(PICO_I2C_SLAVE1,              -608,  "Pico Slave1 communication I2C error")   \
    X(PICO_STATUS_SLAVE2,           -609,  "Pico Slave2 device status returned an error")   \
    X(PICO_I2C_SLAVE2,              -610,  "Pico Slave2 communication I2C error")   \
    X(PICO_STATUS_SLAVE3,           -611,  "Pico Slave3 device status returned an error")   \
    X(PICO_I2C_SLAVE3,              -612,  "Pico Slave3 communication I2C error")   \
    X(PICO_UNKNOW_ERROR,            -613,  "Pico Slave unknow error with com")   \
    X(PWR_I2C_ERROR,                -614,  "I2C com error with CURRENT MONITOR module (INA219)")   \
    X(PWR_DEFAULT_ERROR,            -615,  "CURRENT MONITOR module (INA219) default value error")   \

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
