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


#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "sys_spi.h"


 /**
 * @brief structure to contents the paramaeter of the user SPI
 * 
 */
struct user_spi {
    spi_inst_t *spi_id;     //!< spi id, define as uart0 for user
    uint32_t baudrate;          //!< spi baudrate
    uint32_t databits;          //!< spi size of data
    uint8_t cs;                 //!< spi Chipselect gpio used 
    uint8_t mode;              //!< spi mode to define cpol,cpha and msb
    bool status;              //!< spi enable or disable.  1 = enable
};

//!< Declare and initialize a global struct with default values
struct user_spi uspi = {DEF_SPI_USER,DEF_SPI_BAUD,DEF_SPI_DATABITS,DEF_SPI_CS,DEF_SPI_MODE,DEF_SPI_STATUS};


/**
 * @brief  function to configure the parameters of the user spi based on the data set on the spi structure
 * 
 */
void sys_spi_enable(){

  // Set GPIO function to SPI
  gpio_set_function(USER_SPI_RX_PIN, GPIO_FUNC_SPI);
  gpio_set_function(USER_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(USER_SPI_TX_PIN, GPIO_FUNC_SPI);

  gpio_init(uspi.cs);
  gpio_set_dir(uspi.cs, GPIO_OUT);
  gpio_put(uspi.cs, 1);

  //gpio_set_function(USER_SPI_CSN_PIN, GPIO_FUNC_SPI);

  spi_set_slave(uspi.spi_id,0);  // spi is master
  spi_init(uspi.spi_id,uspi.baudrate);
  sys_spi_set_mode(uspi.mode);
  uspi.status = 1;  // set flag to indicate of SPI is enabled
  fprintf(stdout, "User SPI is enabled\r\n");
}

/**
 * @brief  function to disable the user spi and configure pins reserved by SPI to normal GPIO, set as input.
 * 
 */
void sys_spi_disable(){

    // Disable.
    spi_deinit(uspi.spi_id);

    // set pins used for uart to GPIO mode
    gpio_set_function(USER_SPI_RX_PIN, GPIO_FUNC_SIO);
    gpio_set_function(USER_SPI_SCK_PIN, GPIO_FUNC_SIO);
    gpio_set_function(USER_SPI_TX_PIN, GPIO_FUNC_SIO);
    //gpio_set_function(USER_SPI_CSN_PIN, GPIO_FUNC_SIO);

    bool mode = 0;  // define GPIO as input
    gpio_set_dir (USER_SPI_RX_PIN,mode); // set pins as input if mode = 0
    gpio_set_dir (USER_SPI_SCK_PIN,mode); // set pins as output if mode = 1
    gpio_set_dir (USER_SPI_TX_PIN,mode); // set pins as input
    gpio_set_dir (uspi.cs,mode); // set pins as input

    uspi.status = 0;  // Reset flag to indicate of serial port is disabled
    fprintf(stdout, "User SPI is disabled\r\n");
}

/**
 * @brief Function who return if the user spi is enabled or not
 * 
 *  @return  True  if spi is enabled
 */
bool sys_spi_status(){
    return uspi.status;
}


/**
 * @brief Function to update the baudrate speed on the structure, Baudrate will be updated with function spi enable
 * 
 * @param speed Baudrate to save on structure
 */
void sys_spi_set_baudrate(uint32_t speed) {
      
    if (speed != uspi.baudrate) {  // if value changed
        uspi.baudrate = speed;
        if (uspi.status) {                              // id SPI is enabled, update the baudrate
            spi_init(uspi.spi_id,uspi.baudrate);
        }
    }
}

/**
 * @brief function to return the actual baudrate
 * 
 * @return uint32_t  baudrate
 */
uint32_t sys_spi_get_baudrate() {
   return spi_get_baudrate(uspi.spi_id);
}

/**
 * @brief Function to update the SPI chipselectto be used. 
 * 
 * @param num  gpio to write 
 * @return uint8_t  error if number is not validbaudrate
 */
uint8_t sys_spi_set_chipselect(uint32_t num) {

   uint8_t gpio_list[] = {0,1,5,6,7,12,13,14,15,16,17}; // List of valid gpio to be used as chipselect on Master pico
   
   bool valid = false;
   size_t sizeg = sizeof(gpio_list) / sizeof(gpio_list[0]);
 
  for (size_t i=0; i< sizeg; i++) { 
    if (num == gpio_list[i]) {
      valid = true;
      break;
    }
  }
   
  if (valid == false) { return SPI_CS_NUM_ERROR;} 

   if (num != uspi.cs) {  // if value changed
        uspi.cs = num;
        fprintf(stdout, "SPI Chipselect gpio updated to:  %d\r\n", num); 
   }
  
  return 0;   // no error
}

/**
 * @brief function to return the chipselect gpio used
 * 
 * @return uint32_t  gpio
 */
uint32_t sys_spi_get_chipselect() {
   return uspi.cs;
}


/**
 * @brief Function to update the number of databits to be used. 
 * 
 * @param num  Databits to write 
 */
uint8_t sys_spi_set_databits(uint32_t num) {

   if (num != uspi.databits) {  // if value changed
        uspi.databits = num;
        if (uspi.status) {                              // id SPI is enabled, update the baudrate
            //spi_set_format(uspi.spi_id,uspi.databits,uspi.cpol,uspi.cpha,uspi.msb);
            fprintf(stdout, "SPI Parameter databit updated to  %d\r\n", num);
        }
   }
}

/**
 * @brief function to return the databits value saved on structure
 * 
 * @return uint32_t  databits
 */
uint32_t sys_spi_get_databits() {
   return uspi.databits;
}


/**
 * @brief function to set the mode to use for spi communication
 * 
 * @param mode  value of mode to use
 * @return uint8_t error number
 */
uint8_t sys_spi_set_mode(uint8_t mode){
  uint8_t ecode = 0;
  bool cpol,cpha,msb;

  msb = SPI_MSB_FIRST; // LSB First is not supported

  switch (mode) {
      case 0: { cpol = SPI_CPOL_0; cpha = SPI_CPHA_0;}
      case 1: { cpol = SPI_CPOL_0; cpha = SPI_CPHA_1;}
      case 2: { cpol = SPI_CPOL_1; cpha = SPI_CPHA_0;}
      case 3: { cpol = SPI_CPOL_1; cpha = SPI_CPHA_1;}
      default: {ecode = SPI_MODE_NUM_NOTVALID; }
  }

  if (ecode == 0) {
      uspi.mode = mode;
      spi_set_format(uspi.spi_id,uspi.databits,cpol,cpha,msb);
      fprintf(stdout, "SPI Mode = %d, define: Cpol = %d, Cpha = %d, Msb = %d\r\n", mode,cpol,cpha,msb );
  }
  return ecode;
}


/**
 * @brief function who return the spi mode used in communication.
 * 
 * @return uint8_t  mode value
 */
uint8_t sys_spi_get_mode(){
  return uspi.mode;
}



/**
 * @brief function to send data to spi port
 * 
 * @param dwrt   string to write on the spi port
  */
void sys_spi_write_byte(uint8_t* dwrt){ // write data only
   int tctr = 0; // Counter for transmit characters
   do {
      if (dwrt[tctr] != '\0') {  // if string is not empty
        if (uspi.databits <=8) {  
             // spi_write_blocking(spi_port, &dwrt, 1)


        }
        //send_char(dwrt[tctr]);
        fprintf(stdout,"Sent char #%d: 0x%x, %c\n",tctr, dwrt[tctr], dwrt[tctr]);
        tctr++; // increment char pointer
      }
    } while (dwrt[tctr] != '\0');  // loop until end of string

   //ucom.lastchr = dwrt[tctr-1];  // identify the last valid character of string, expect CR or LF
   fprintf(stdout,"lastchar Tx only: 0x%x\n", dwrt[tctr-1]);
}


