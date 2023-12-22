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


#include "pico/stdlib.h"
#include <ctype.h>
#include "pico/binary_info.h"
#include "include/functadv.h"
#include "stdio.h"
#include "string.h"
#include "include/master.h"


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


// function read ADC. Float value is returned
// Command used by SCPI 
float  read_master_adc(uint8_t channel) {
    const float cfactor = ADC_REF / (1 << 12);  // 12 Bits conversion
    uint16_t value;
    float adc_val;

    adc_select_input(channel);
    value = adc_read();  // read ADC
    adc_val = value * cfactor;

    switch (channel) {
        case 0: // ADC channel 0
            fprintf(stdout,"ADC0: Raw value: 0x%03x, voltage: %f V\n", value, adc_val);
            break;
        case 1: // ADC channel 1
            fprintf(stdout,"ADC1: Raw value: 0x%03x, voltage: %f V\n", value, adc_val);
            break;
        case 2:  // Not used as analog channel (only ADC0 and 1)
            adc_val = 0;   
            fprintf(stdout,"ADC2: is not allowed \n");
            break;
        case 3:    // Vsys value
            adc_val = adc_val*3;  // Pico has voltage divider as input
            fprintf(stdout,"Raw value 3: 0x%03x, Vsys  voltage: %f V\n", value, adc_val);
            break;
        case 4:    // Master internal temperature
            adc_val = 27 - (adc_val - 0.706)/0.001721; // from RP2040 Datasheet
            fprintf(stdout,"Raw value 0: 0x%03x, Temperature: %f C\n", value, adc_val);
            break;
    }
    return adc_val;

}



// SCPI function to control the power device IN219
float read_power(uint8_t mode){
  int16_t readv;
  char meas[3] = {0,0,0};
  char rmd[8] = {0,0,0,0,0,0,0,0};

   switch (mode){
    case 0 :
      readv = ina219GetBusVoltage() * 0.001;
      strcpy(meas,"V");
      strcpy(rmd,"BUS V");
      break;

    case 1 :
      readv = ina219GetCurrent_mA();  // read current.
      strcpy(meas,"mA");
      strcpy(rmd,"CURRENT");
      break;

    case 2 :
      readv = ina219GetPower_mW();
      strcpy(meas,"mW");
      strcpy(rmd,"POWER");
      break;
    case 3 :

      readv = ina219GetShuntVoltage() * 10E-3;
      strcpy(meas,"mV");
      strcpy(rmd,"SHUNT");
      break;
  }   

    fprintf(stdout,"IN219,read: %s ,  value: %d %s \n",rmd, readv,meas);
    return readv;
}


// SCPI function to calibrate the current on the power device IN219
void calibrate_power(float actual, float expected){
     bool flg;

        flg = ina219CalibrateCurrent_mA(actual,expected);
        if (flg) {
             fprintf(stdout,"IN219,calibration current, actual value: %.2f, expected value: %.2f \n",actual, expected);
        } else {
             fprintf(stdout,"IN219,calibration not performed, cal factor identical, actual value: %.2f, expected value: %.2f \n",actual, expected);
        }
}

// temporary function
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

// temporary function
// function read ADC internal value . Reading are placed in array
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

// function set DAC.
// Command used by SCPI 
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


// This function check if the eeprom is detected and if the data is valid
// The byte check could be validated to detects if eeprom is empty or not

uint8_t eeprom_data_valid(bool check_data,at24cx_dev_t* eeprom)
{
  at24cx_writedata_t dt;
 

 // register eeprom 24lc32
 at24cx_i2c_device_register(eeprom, EEMODEL, I2C_ADDRESS_AT24CX);
  
  //Check if eeprom is active
 fprintf(stdout,"eeprom is %s\n", (*eeprom).status ? "detected" : "not detected");
 if ((*eeprom).status == false) return EDE;

 if (check_data) {  // if required to check data 
    dt.address = ADD_EEPROM_BASE;
    if (at24cx_i2c_byte_read(*eeprom, &dt) == AT24CX_OK) {
         if (dt.data != EE_CHECK_CHAR) {  // Error Check byte not written
            fprintf(stdout,"Error Check Character do not match, expect: 0x%02X read: 0x%02X \n",EE_CHECK_CHAR,dt.data);
            return ECE;
         } else {
            fprintf(stdout,"EEprom check byte valid: 0x%02X \n",dt.data);
         }
    }else {
        fprintf(stdout,"Device byte read error!\n");
        return EDE;
    }
 }
 return NOERR;
}

// Write parameter to eeprom

uint8_t cfg_eeprom_rw(char mode, uint32_t eeaddr,uint8_t eedatalen, char *data, uint8_t datalen)
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  uint8_t status;

  // EEprom check access and validity
  status = eeprom_data_valid(true, &eeprom_1);
  if (status != NOERR) { return status;}  // if error do not execue read or write on eeprom
 

  if (mode == 'w') {
    fprintf(stdout,"\nWrite Eeprom parameter\n");
    if (datalen > eedatalen) {   // if data is longer than reserved field
        fprintf(stdout,"Error, data to write is too long, field length 0x%02X : Data length 0x%02X \n", eedatalen, datalen);
        return EOOR;   // raise error due to value outside maximum limit
    }  
    for(int i=0;i<eedatalen;i++)
    {
        dt.address = ADD_EEPROM_BASE + eeaddr +i; // physical address on eeprom
        dt.data = data[i];
        dt.data_multi[i] = data[i];  // save value on array to be compare after eeprom 

        if (at24cx_i2c_byte_write(eeprom_1, dt) == AT24CX_OK) {
            fprintf(stdout,"Writing at address 0x%02X: 0x%02X , %c \n", dt.address, dt.data,dt.data);
        }else {
            fprintf(stdout,"EEprom device write byte error! \n");
            return EDE;
        }
    }
  }
    fprintf(stdout,"\nRead eeprom byte test\n");
    for(int i=0;i<eedatalen;i++)
    {
      dt.address = ADD_EEPROM_BASE + eeaddr +i;
      if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK) {

         fprintf(stdout,"Reading at address 0x%02X: 0x%02X , %c \n", dt.address, dt.data,dt.data);
         data[i] = dt.data;   // save value on array
      }else {
        fprintf(stdout,"EEprom device byte read error!\n");
        return EDE;
      }
    }

    if (mode == 'w') {  // if data written on eeprom, compare value.
        fprintf(stdout,"\nCompare EEprom Write and read\n");
        for(int i=0;i<eedatalen;i++)
        {
            dt.address = ADD_EEPROM_BASE + eeaddr +i;
            if (data[i] != dt.data_multi[i]) {
            fprintf(stdout,"Error byte Write-read at address 0x%02X: write value 0x%02X: read value: 0x%02X\n", dt.address, dt.data_multi[i], data[i]);
            return ECE;
            }
        }
        fprintf(stdout,"Eeprom data match\n");
    }

    return NOERR;
}


// Read the entire contents of the configuration eeprom and store data on eeprom config structure
// EEprom structure is global (defined on master.h)

uint8_t cfg_eeprom_read_full()
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  uint32_t datalen;
  uint8_t status;

  // EEprom check access and validity of the check byte
  status = eeprom_data_valid(true, &eeprom_1);
  if (status != NOERR) { return status;}  // if error do not execue read or write on eeprom

  datalen = sizeof(ee.cfg);  // read size of eeprom global structure 
  
  fprintf(stdout,"\n--> Read full eeprom\n");
  for(int i=0;i<datalen;i++)
    {
      dt.address = ADD_EEPROM_BASE +i;  // calculate physical address
      if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK) {
         fprintf(stdout,"Full Eeprom reading at address 0x%02X: 0x%02X \n", dt.address, dt.data);
         ee.data[i] = dt.data;   // save value on eeprom structure
      }else {
        fprintf(stdout,"EEprom read full device byte read error!\n");
        return EDE;
      }
    }
    fprintf(stdout,"\n--> Completed read of full eeprom\n");
    return NOERR;
}



// Write the default values on EEprom. EEprom is written by page
// EEprom structure is global (defined on master.h)

uint8_t cfg_eeprom_write_default()
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  uint32_t datalen;
  uint32_t addr;
  int i;
  uint8_t status;

  eep eed = DEF_EEPROM; // Assign default value to structure eeprom

  // EEprom check access
  status = eeprom_data_valid(false, &eeprom_1);
  if (status != NOERR) { return status;}  // if error do not execue read or write on eeprom

  datalen = sizeof(ee.cfg);  // read size of eeprom global structure 

  fprintf(stdout,"\n--> Write full eeprom\n");
  dt.address = ADD_EEPROM_BASE;  // Set base adress
  i = 0; // index for the location of data

  while (0 < datalen)
  {
    uint32_t pagelen = (dt.address|(EE_PAGESIZE-1)) - dt.address + 1;
    uint32_t writelen = min(datalen, pagelen);

    strncpy(&dt.data_multi[0],&eed.data[i],writelen); // copy data in array

    if (at24cx_i2c_page_write(eeprom_1, dt) == AT24CX_OK) fprintf(stdout,"Page Writing at address 0x%02X\n", dt.address);
    else { fprintf(stdout,"Device page write error!\n"); return EDE; }
 
    dt.address += writelen;
    i  += writelen;
    datalen -= writelen;
  }

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












