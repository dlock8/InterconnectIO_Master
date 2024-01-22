//
   
/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: MIT
 */
#define V 1     // Volts
#define I 2     // mA
#define P 3     // mW
#define S 4     // mV

char output_buffer[1024];
size_t output_buffer_pos;


bool test_ioboard(); 
bool test_selftest(); 
bool test_eeprom();
void test_ina219(void);
bool test_adc();
bool test_dac(float);
void test_command(void);
static void output_buffer_clear(void);

 
 