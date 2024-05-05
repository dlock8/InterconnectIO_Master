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
    i2c_inst_t *i2c_id;     //!< spi id, define as uart0 for user spi
    uint32_t baudrate;      //!< spi baudrate
    uint32_t databits;      //!< spi size of data
    bool status;            //!< spi enable or disable.  1 = enable
};

//!< Declare and initialize a global struct with default values
struct user_i2c uiic = {DEF_I2C_USER,DEF_I2C_BAUD,DEF_I2C_DATABITS,DEF_I2C_STATUS};


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
  * @brief  function who transform an array of bytes to an array of words.
  * 
  * @param byte_array  pointer to the array of bytes to transform
  * @param word_array  pointer to the array of the resulting word
  * @param length      length of the byte array
  */
void i2c_bytes_to_words(uint8_t *byte_array, uint16_t *word_array, size_t length) {
    for (size_t i = 0; i < length / 2; i++) {
        word_array[i] = byte_array[2*i] << 8 | (byte_array[2*i + 1]); //!< transform 2 bytes in 1 word
        //fprintf(stdout, "bytes to word # %d, data: %02x\r\n",i,word_array[i]);
    }
}

 /**
  * @brief function who transform an array of word to an array of byte
  * 
  * @param word_array pointer to the array of words to transform
  * @param byte_array pointer to the array of the resulting bytes
  * @param length length of the word array
  */
void i2c_words_to_bytes(uint16_t *word_array, uint8_t *byte_array, size_t length) {
    for (size_t i = 0; i < length; i++) {
        byte_array[2*i] = word_array[i] & 0xFF;        // Least significant byte
        byte_array[2*i + 1] = (word_array[i] >> 8);    // Most significant byte
        //fprintf(stdout, "word to bytes # %d, data: %02x\r\n",i,byte_array[i]);
    }
}

/**
 * @brief function who write/read bytes to defined SPI port
 * 
 * @param mode Constant who define the function to perform: write data,write-read data or read data
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen contain length of the read array
 */
 /*
static void i2c_bytes(uint8_t mode,uint8_t* wdata,uint8_t wlen,uint8_t* rdata,uint8_t rlen){
   uint8_t csdelay = 1; // 1 us delay 
   uint8_t i,j;

    for(i =0; i < wlen+rlen; i++) { // loop to write single byte
        if (i >= wlen) { wdata[i]= 0;} // send 0 as default value
        gpio_put (uspi.cs,0);  //!< Active Chipselect
        sleep_us(csdelay);    //!<  wait small delay for let CS goes active
        while (!spi_is_writable(uspi.spi_id)) {} // Wait until buffer is writable
        //!< based on mode selected, send SPI data, 1 byte at the time because
        //!< chipselect need to be toggle at each byte on some device. (mandatory on SPI slave mode)
        if (mode == SPIW) { spi_write_blocking(uspi.spi_id,&wdata[i],1);}
        if (mode == SPIWR) { spi_write_read_blocking(uspi.spi_id,&wdata[i],&rdata[i],1);} 
        if (mode == SPIR) { spi_read_blocking(uspi.spi_id,SPI_DEFAULT_VALUE,&rdata[i],1);} 
        if (uspi.mode >=4 ) { // If CS need to toggle each byte 
              sleep_us(csdelay);
              gpio_put (uspi.cs,1); //!< De-active Chipselect
              sleep_us(csdelay);    //!<  wait small delay for let CS goes de-active
        }
    }

    if (uspi.mode <=3 ) { // If CS need to toggle at end of transmission
          gpio_put (uspi.cs,1); // De-activate CS
    }
 

    if (mode == SPIW) {   //!<  send message to monitor to help debug
              fprintf(stdout, "SPI write, nb of bytes  written: %d \r\n",rlen);
    }

    if (mode == SPIWR) {  //!<  send message to monitor to help debug
        fprintf(stdout, "SPI write-read, nb of bytes to write: %d, Nb of bytes to read: %d\r\n",wlen,rlen);
        for(j =0; j < wlen+rlen; j++) { // loop to print
            fprintf(stdout, "SPI write-read,# %02d, Write: 0x%x, Read: %02x\r\n",j,wdata[j],rdata[j]);
        }
    }

    if (mode == SPIR) { //!<  send message to monitor to help debug
        fprintf(stdout, "SPI read, Nb of bytes to read: %d\r\n",rlen);
        for(j =0; j < rlen; j++) { // loop to print
            fprintf(stdout, "SPI read,# %d, Read: %02x\r\n",j,rdata[j]);
        }
    }

}
*/


/**
 * @brief function who write/read word to defined SPI port
 * 
 * @param mode Constant who define the function to perform: write data,write-read data or read data
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen contain length of the read array
 */

 /*
static void i2c_word(uint8_t mode,uint16_t* wdata,uint8_t wlen,uint16_t* rdata,uint8_t rlen){
   uint8_t csdelay = 1; // 1 us delay 
   uint8_t i,j;

    for(i =0; i < wlen+rlen; i++) { // loop to write single byte
        if (i >= wlen) { wdata[i]= 0;} // send 0 as default value
        gpio_put (uspi.cs,0);
        sleep_us(csdelay);
        while (!spi_is_writable(uspi.spi_id)) {} // Wait until buffer is writable
        //!< based on mode selected, send SPI data, 1 word at the time because
         //!< chipselect need to be toggle at each word on some device. (mandatory on Pico SPI slave mode)
        if (mode == SPIW) { spi_write16_blocking(uspi.spi_id,&wdata[i],1);}
        if (mode == SPIWR) { spi_write16_read16_blocking(uspi.spi_id,&wdata[i],&rdata[i],1);} 
        if (mode == SPIR) { spi_read16_blocking(uspi.spi_id,SPI_DEFAULT_VALUE,&rdata[i],1);} 
        if (uspi.mode >=4 ) { // If CS need to toggle each word 
              sleep_us(csdelay);
              gpio_put (uspi.cs,1); //!< De-active Chipselect
              sleep_us(csdelay);    //!<  wait small delay for let CS goes de-active
        }
    }
    if (uspi.mode <=3 ) { // If CS need to toggle at end of transmission
          gpio_put (uspi.cs,1); // De-activate CS
    }

    if (mode == SPIW) { //!<  send message to monitor to help debug
        fprintf(stdout, "SPI write, nb of word  written: %d\r\n",wlen);
    }

    if (mode == SPIWR) {//!<  send message to monitor to help debug
        fprintf(stdout, "SPI write-read, nb of word to write: %d, Nb of word to read: %d\r\n",wlen,rlen);
        for(j =0; j < wlen+rlen; j++) { // loop to print
            fprintf(stdout, "SPI write-read,# %d, Write: 0x%04x, Read: 0x%04x\r\n",j,wdata[j],rdata[j]);
        }
    }

    if (mode == SPIR) {//!<  send message to monitor to help debug
        fprintf(stdout, "SPI read, nb of word to read: %d\r\n",rlen);
        for(j =0; j < rlen; j++) { // loop to print
            fprintf(stdout, "SPI read,# %d, Read: 0x%04x\r\n",j,rdata[j]);
        }
    }

}
*/


/**
 * @brief main function called by SCPI command to send or receive data on SPI port
 * 
 * @param wdata Pointer to an array of data to write
 * @param wlen  contain length of the write array
 * @param rdata Pointer who will contains the data read
 * @param rlen  contain length of the read array
 * @param wflag Flag to define if we process bytes size or word size
 * @return uint8_t error code
 */
/*
uint8_t sys_i2c_wri_read_data(uint8_t* wdata,uint8_t wlen,uint8_t* rdata,uint8_t rlen, bool* wflag){
  bool swwd = false;
  bool swrd = false;
  size_t saw,sar;
  size_t i,j;

  uint16_t *wwdata = NULL;    // create pointer to write word data, required if databits = 16
  uint16_t *wrdata = NULL;    // create pointer to read word data, required if databits = 16
 

  // data to write is received in byte. Transformaton is required from array of byte to array of
  // word before writing on port
  if (uspi.databits > 8){ // if we need to write data using word size
    saw = wlen/2;  // 2 bytes by word, size divide by two
    sar = rlen;
    if (wlen > 0) {
       //!<  Dynamically allocate memory for wwdata
        wwdata = (uint16_t *)malloc(saw * sizeof(uint16_t));
        if (wwdata == NULL) {
            // Error handling: unable to allocate memory
            fprintf(stdout, "Failed to allocate memory for wwdata\n");
            exit(MALLOC_FAILURE);
        }
        // Convert bytes to words
        bytes_to_words(wdata,wwdata,wlen);
        swwd = true;
    }
    if (rlen>0) {
        // Dynamically allocate memory for wrdata
        wrdata = (uint16_t *)malloc(sar * sizeof(uint16_t));
        if (wrdata == NULL) {
            // Error handling: unable to allocate memory
            fprintf(stderr, "Failed to allocate memory for wrdata\n");
            exit(MALLOC_FAILURE);
        }
    }

  }
  
 //fprintf(stdout, "SPI Info, Databits: %d , Mode %d, Chipselect %d\r\n",uspi.databits,uspi.mode,uspi.cs);

  if (wlen > 0 && rlen== 0 ) { // if we need to perform write only
      if (uspi.databits <=8) {
        spi_bytes(SPIW,wdata,wlen,rdata,rlen); //!< bytes command
     } else {
        spi_word(SPIW,wwdata,saw,wrdata,sar); //!< word command
     }
  }

  if (wlen > 0 && rlen > 0){ // if some data to write and read
     if (uspi.databits <=8) {
         spi_bytes(SPIWR,wdata,wlen,rdata,rlen);  //!< bytes command
     } else {
         spi_word(SPIWR,wwdata,saw,wrdata,sar); //!< word command
     }
  }

  if (wlen == 0 && rlen > 0){ // if some data to  read
     if (uspi.databits <=8) {
         spi_bytes(SPIR,wdata,wlen,rdata,rlen); //!< bytes command
     } else {
        spi_word(SPIR,wwdata,saw,wrdata,sar);   //!< word command
     }
  }

  if (uspi.databits >8) {  // if word read, transform to byte and save in rdata
      words_to_bytes(wrdata, rdata,sar*2+wlen ); 
  }

  *wflag = (uspi.databits <=8)? false:true;  //!< set flag to determine size of data read
 // free(wrdata);
 // free(wwdata);
  return NOERR;
}

*/
