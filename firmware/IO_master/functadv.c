/*-
 * BSD 2-Clause License
 *
 * Copyright (c) 2022, Daniel Lockhead 
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   functadv.c
 * @date   Dec 16, 2022
 *
 * @brief  InterconnectIO Board
 *
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "include/functadv.h"

#include "hardware/adc.h"
#include "include/i2c_com.h"
#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
//#include "pico_lib2/src/sys/include/sys_adc.h"


//#include "include/fts_scpi.h"
//#include "userconfig.h"


// Initlialize ADC function  if enable=1, else pin will be GPIO
void setup_ADC(bool enable) {
   
    if (enable) {  // ADC function

        adc_gpio_init(ADC0);
        adc_gpio_init(ADC1);

    
    } else { // ADC0 and ADC1 are defined as GPIO function
       gpio_init(ADC0);
       gpio_init(ADC1);

       gpio_set_dir(ADC0, GPIO_IN);
       gpio_set_dir(ADC1, GPIO_IN);
    }
}

// function read ADC. Reading are placed in array
void  read_ADC(float *adc_val) {
    const float cfactor = ADC_REF / (1 << 12);  // 12 Bits conversion
    uint16_t value;

    adc_select_input(0);
    value = adc_read();  // read ADC0
    adc_val[0] = value * cfactor;
    fprintf(stdout,"Raw value 0: 0x%03x, ADC0  voltage: %f V\n", value, adc_val[0]);
    adc_select_input(1);
    value = adc_read();  // read ADC1
    adc_val[1] = value * cfactor;
    fprintf(stdout,"Raw value 0: 0x%03x, ADC1  voltage: %f V\n", value, adc_val[1]);
}

// function read ADC. Reading are placed in array
void  read_int_ADC(float *adc_val) {
    const float cfactor = ADC_REF / (1 << 12);  // 12 Bits conversion
    uint16_t value;
    float  conval;
    float sread;

    adc_select_input(3);
    //sleep_ms(100);
    value = adc_read();  // read VSYS
    conval = value * cfactor;
    adc_val[0] = conval*3;  // Pico has voltage divider as input
    fprintf(stdout,"Raw value 0: 0x%03x, Vsys  voltage: %f V\n", value, adc_val[0]);

    adc_select_input(4);
    //sleep_ms(100);
    value = adc_read();  // read internal temperature
    conval = value * cfactor;
    adc_val[1] = 27 - (conval - 0.706)/0.001721; // from RP2040 Datasheet
    fprintf(stdout,"Raw value 0: 0x%03x, Temperature: %f C\n", value, adc_val[1]);

}


uint8_t dac_set(float value, bool save){
    uint16_t error;
    float ovalue;
    bool flag;

    ovalue = value;
    error = NOERR;
   

    // float value;

    if (value > MAXDACVOLT) { 
        value = MAXDACVOLT;
        error = EOOR;   // raise error due to value outside maximum limit
    }

    if (value < MINDACVOLT) { 
        value = MINDACVOLT;
        error = EOOR;
     }

    if (save) {  
        flag = dev_mcp4725_save(i2c0, MCP4725_ADDR0, value);
    } else {  
        flag = dev_mcp4725_set(i2c0, MCP4725_ADDR0, value);
    }

    if (!flag) {
        fprintf(stdout,"DAC Error on set MCP4725\n"); 
        error = EDE;
    } else {
        fprintf(stdout,"DAC voltage set to: %2.3f V\n", value);
       
    }
  return error; 

}

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Sweep through all 7-bit I2C addresses, to see if any slaves are present on
// the I2C bus. Print out a table that looks like this:
//
// I2C Bus Scan
//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
// 0
// 1       @
// 2
// 3             @
// 4
// 5
// 6
// 7
//
// E.g. if slave addresses 0x12 and 0x34 were acknowledged.

//#include <stdio.h>
//#include "pico/stdlib.h"
//#include "pico/binary_info.h"
#include "hardware/i2c.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan_i2c_bus() {

    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "*");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
    //return 0;
}












