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


#ifndef _SCPI_SPI_H_
#define _SCPI_SPI_H_

#ifdef __cplusplus
 extern "C" {
#endif


#define USER_SPI_SCK_PIN   2
#define USER_SPI_TX_PIN    3
#define USER_SPI_RX_PIN    4
#define USER_SPI_CSN_PIN   5



#define SPI_BUF_LEN 8

#define DEF_SPI_USER        spi0     //!< spi0 is the user communication channel to external
#define DEF_SPI_BAUD        1E5      //!< spi baudrate is set at 100Khz by default
#define DEF_SPI_DATABITS    8        //!< spi number of bits used for comunication
#define DEF_SPI_CS          5        //!< spi gpio used to perform chipselect
#define DEF_SPI_MODE        0        //!< define mode paramaters: cs,cpol,cpha and MSB
#define DEF_SPI_STATUS      0        //!< spi is disable by default

#define SPI_DEFAULT_VALUE   0       //!< spi data to write when we perform read

#define SPIW                0       // mode = SPI write
#define SPIWR               1       // mode = SPI write-read
#define SPIR                2       // mode = SPI read

#define NOERR                     0
#define SPI_MODE_NUM_NOTVALID     40     //!< spi error
#define SPI_CS_NUM_ERROR          41
#define MALLOC_FAILURE            42



void scpi_spi_enable(void);
void scpi_spi_disable(void);
bool scpi_spi_status(void);
void scpi_spi_set_baudrate(uint32_t speed);
uint32_t scpi_spi_get_baudrate(void);
uint8_t scpi_spi_set_chipselect(uint32_t num);
uint32_t scpi_spi_get_chipselect(void); 

uint8_t scpi_spi_set_databits(uint32_t num);
uint32_t scpi_spi_get_databits(void); 
void scpi_spi_set_mode(uint8_t mode);
uint8_t scpi_spi_get_mode(void);

uint8_t scpi_spi_wri_read_data(uint8_t* wdata,uint8_t wlen,uint8_t* rdata,uint8_t rlen, bool* wflag);
void spi_bytes_to_words(uint8_t *byte_array, uint16_t *word_array, size_t length);
void spi_words_to_bytes(uint16_t *word_array, uint8_t *byte_array, size_t length);

#ifdef __cplusplus
}
#endif

#endif // _SYS_SPI_H_

