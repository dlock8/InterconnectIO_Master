/**
 * @file    test.h
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Header file defining constants and macros for buffer and message management.
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#ifndef test_H
#define test_H

/**
 * @def BUFFER_SIZE
 * @brief Maximum depth size of the circular buffer.
 */
#define BUFFER_SIZE 32

/**
 * @def MESSAGE_LENGTH
 * @brief Maximum number of characters per string.
 */
#define MESSAGE_LENGTH 120

/**
 * @def V
 * @brief Constant for volts.
 */
#define V 1  ///< Volts

/**
 * @def I
 * @brief Constant for milliAmps.
 */
#define I 2  ///< mA

/**
 * @def P
 * @brief Constant for milliWatts.
 */
#define P 3  ///< mW

/**
 * @def S
 * @brief Constant for milliVolts.
 */
#define S 4  ///< mV

extern char out_buffer[1024];
extern size_t out_buffer_pos;

bool test_ioboard();
uint8_t test_selftest(const char* testboard_num, uint8_t run);
void internal_test_sequence(const char* testboard_num, uint8_t run);
void test_inst_manual();
bool test_eeprom();
void test_ina219(void);
bool test_adc();
bool test_dac(float);
void test_command(void);
static void output_buffer_clear(void);
int onewiretest(void);
void test_spi_adx(void);
void test_i2c_adx(void);
void test_i2c_ina219(void);
void wait_button_pressed(void);

#endif /* BUFFER_H */