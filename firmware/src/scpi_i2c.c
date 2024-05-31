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

 /**
 * @file   scpi_i2c.c
 * @date   Jan08, 2024
 *
 * @brief  Assembly of function to handle the SCPI command relative to I2C
 * @brief  I2C adress need to be set before using read-write command
 * @brief  Data coule be reported in byte or word according to the value of databits
 * @brief  Working example of SCPI code to use is availabe
 *
 */


#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico_lib2/src/sys/include/sys_i2c.h"
#include "scpi_i2c.h"


 /**
 * @brief structure to contents the parameter of the user I2c
 * 
 */
struct user_i2c {
    i2c_inst_t *i2c_id;     //!< i2c id
    uint8_t address;           //!< device address baudrate
    uint32_t baudrate;      //!< i2c baudrate
    uint32_t databits;      //!< i2c size of data. 8 for byte and 16 for word
    bool status;            //!< i2c enable or disable.  1 = enable
};

//!< Declare and initialize a global struct with default values
struct user_i2c uiic = {DEF_I2C_USER,DEF_I2C_ADDR,DEF_I2C_BAUD,DEF_I2C_DATABITS,DEF_I2C_STATUS};


/**
 * @brief  function to configure the parameters of the user I2C based on the data set on the I2C structure
 * 
 */
void scpi_i2c_enable(){

    gpio_init(USER_I2C_SDA_PIN);
    gpio_set_function(USER_I2C_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(USER_I2C_SDA_PIN);

    gpio_init(USER_I2C_SCL_PIN);
    gpio_set_function(USER_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(USER_I2C_SCL_PIN);

    i2c_init(uiic.i2c_id, uiic.baudrate);
    uiic.status = 1;  // set flag to indicate of I2C is enabled
    fprintf(stdout, "User I2C is enabled\r\n");

}


/**
 * @brief  function to disable the user spi and configure pins reserved by SPI to normal GPIO, set as input.
 * 
 */
void scpi_i2c_disable(){

    // Disable.
    i2c_deinit(uiic.i2c_id);

    // set pins used for i2c to GPIO mode
    gpio_set_function(USER_I2C_SDA_PIN, GPIO_FUNC_SIO);
    gpio_set_function(USER_I2C_SCL_PIN, GPIO_FUNC_SIO);
 

    bool mode = 0;  // define GPIO as input
    gpio_set_dir (USER_I2C_SDA_PIN,mode); // set pins as input if mode = 0
    gpio_set_dir (USER_I2C_SCL_PIN,mode); // set pins as output if mode = 1
 

    uiic.status = 0;  // Reset flag to indicate of I2C port is disabled
    fprintf(stdout, "User I2C is disabled\r\n");
}

/**
 * @brief Function who return if the user I2C is enabled or not
 * 
 *  @return  True  if I2C is enabled
 */
bool scpi_i2c_status(){
    return uiic.status;
}

/**
 * @brief Function to update the device address to be used. 
 *       
 * 
 * @param num  Device Address 
 */
uint8_t scpi_i2c_set_address(uint32_t num) {

    uiic.address = num;  
    fprintf(stdout, "I2C Device address updated to  %d\r\n", num);
    return NOERR;
}

/**
 * @brief function to return the device address value saved on structure
 * 
 * @return uint32_t  Device address
 */
uint32_t scpi_i2c_get_address() {
   return uiic.address;
}


/**
 * @brief Function to update the baudrate speed on the structure, Baudrate will be updated with function spi enable
 * 
 * @param speed Baudrate to save on structure
 */
void scpi_i2c_set_baudrate(uint32_t speed) {
      
    if (speed != uiic.baudrate) {  // if value changed
        uiic.baudrate = speed;
        if (uiic.status) {                              // id SPI is enabled, update the baudrate
            i2c_init(uiic.i2c_id,uiic.baudrate);
        }
    }
}

/**
 * @brief function to return the actual baudrate
 * 
 * @return uint32_t  baudrate
 */
uint32_t scpi_i2c_get_baudrate() {
   return (uiic.baudrate);
}

/**
 * @brief Function to update the number of databits to be used. Necessary to determine if we
 *        doing communication using byte or word
 * 
 * @param num  Databits to write 
 */
uint8_t scpi_i2c_set_databits(uint32_t num) {

    uiic.databits = num;  
    fprintf(stdout, "I2C Parameter databit updated to  %d\r\n", num);
    
   return NOERR;
}

/**
 * @brief function to return the databits value saved on structure
 * 
 * @return uint32_t  databits
 */
uint32_t scpi_i2c_get_databits() {
   return uiic.databits;
}

 

/**
 * @brief main function called by SCPI command to send or receive data on I2C port
 * 
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen  contain length of the read array
 * @param wflag Flag to define if we process bytes size or word size
 * @return uint8_t error code
 */
int8_t scpi_i2c_wri_read_data(bool regflag, uint8_t* wdata,uint8_t wlen,uint8_t* rdata,uint8_t rlen, bool* wflag)
{
  size_t j;
  int32_t ret;

  if (wlen > 0 && rlen== 0 ) { // if we need to perform write only
        ret = sys_i2c_wbuf(uiic.i2c_id,uiic.address,wdata,wlen);
        for(j =0; j < wlen; j++) { // loop to print
            fprintf(stdout, "I2C write buffer byte, data: 0x%02x\r\n",wdata[j]);
        }
  }

  if (wlen > 0 && rlen> 0 ) { // if we need to perform write data and read data
        if (uiic.databits > 8){ rlen = rlen *2;} // multiply to get number of byte to read
        ret = sys_i2c_wbuf_rbuf(uiic.i2c_id,uiic.address,wdata,wlen,rdata,rlen);

        for(j =0; j < wlen; j++) { // loop to print
            fprintf(stdout, "I2C write buffer byte, data: 0x%02x\r\n",wdata[j]);
        }
        for(j =0; j < rlen; j++) { // loop to print
             if (uiic.databits > 8){
                fprintf(stdout, "I2C read after write, buffer word, data: 0x%02x%02x\r\n",rdata[j],rdata[j+1]);
                j++;
            }else {
                fprintf(stdout, "I2C read after write, buffer byte, data: 0x%02x\r\n",rdata[j]);
            }
        }
  }

  if (wlen == 0 && rlen > 0 ) {// if we need to perform read only
    if (uiic.databits > 8){ rlen = rlen *2;} // multiply nb of word by 2 to get number of byte to read
        ret = sys_i2c_rbuf(uiic.i2c_id,uiic.address,rdata,rlen);

        for(j =0; j < rlen; j++) { // loop to print
            if (uiic.databits > 8){
                fprintf(stdout, "I2C read buffer word, data: 0x%02x%02x\r\n",rdata[j],rdata[j+1]);
                j++;
            }else {
                fprintf(stdout, "I2C read buffer byte, data: 0x%02x\r\n",rdata[j]);
            }
        }

  }

  *wflag = (uiic.databits <=8)? false:true;  //!< set flag to return the size of data (byte or word)

//  return error if value is negative or return number of bytes read or write
 if (ret >= 0) { 
    return NOERR;
 } else {
    fprintf(stdout, "I2C Error return:  %d,\r\n",ret);
    return (int8_t) ret;
 }
  
}

