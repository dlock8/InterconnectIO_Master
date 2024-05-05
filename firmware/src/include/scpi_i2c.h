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
 #ifndef _SCPI_I2C_H_
#define _SCPI_I2C_H_

#ifdef __cplusplus
 extern "C" {
#endif


#define USER_I2C_SDA_PIN   6    // GPIO used to I2C SDA
#define USER_I2C_SCL_PIN   7    // GPIO used to I2C SCL


#define DEF_I2C_USER        i2c1     //!< i2c1 is the user communication channel to external
#define DEF_I2C_BAUD        1E5      //!< i2c baudrate is set at 100Khz by default
#define DEF_I2C_DATABITS    8        //!< i2c number of bits used for comunication
#define DEF_I2C_STATUS      0        //!< spi is disable by default

#define NOERR                     0
//#define SPI_MODE_NUM_NOTVALID     40     //!< spi error
//#define SPI_CS_NUM_ERROR          41
#define I2C_MALLOC_FAILURE          52



void scpi_i2c_enable(void);
void scpi_i2c_disable(void);
bool scpi_i2c_status(void);
void scpi_i2c_set_baudrate(uint32_t speed);
uint32_t scpi_i2c_get_baudrate(void);

uint8_t scpi_i2c_set_databits(uint32_t num);
uint32_t scpi_i2c_get_databits(void); 

uint8_t scpi_i2c_wri_read_data(uint8_t* wdata,uint8_t wlen,uint8_t* rdata,uint8_t rlen, bool* wflag);
void i2c_bytes_to_words(uint8_t *byte_array, uint16_t *word_array, size_t length);
void I2C_words_to_bytes(uint16_t *word_array, uint8_t *byte_array, size_t length);

#ifdef __cplusplus
}
#endif

#endif // _SYS_SPI_H_

