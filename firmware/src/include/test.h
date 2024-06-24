//
   
/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef test_H
#define test_H

#define BUFFER_SIZE 32      // maximum deep size of circular buffer
#define MESSAGE_LENGTH 120  // Maximum number of character by string



#define V 1     // Volts
#define I 2     // mA
#define P 3     // mW
#define S 4     // mV


extern char out_buffer[1024];
extern size_t out_buffer_pos;



bool test_ioboard(); 
bool test_selftest(); 
bool test_eeprom();
void test_ina219(void);
bool test_adc();
bool test_dac(float);
void test_command(void);
static void output_buffer_clear(void);
void test_design(void);
int onewiretest(void);
void test_spi_adx(void);
void test_i2c_adx(void);
void test_i2c_ina219(void);
void test_com_command(void);
int spi_test(void);
int testt(void);

 
#endif /* BUFFER_H */