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




bool test_ioboard(); 
bool test_selftest(); 
bool test_eeprom();
bool test_ina219();
bool test_adc();
bool test_dac(float);
 
 