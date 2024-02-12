//
   
/*
 * Copyright (c) 2021 
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _MASTER_H_
#define _MASTER_H_


#ifdef __cplusplus
extern "C" {
#endif

#define UART_ID   uart1
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 8
#define UART_RX_PIN 9

// Define default value for stdio. 
// on development the result of printf is send to USB and serial port in same time
// On normal operation, the serial port is used for SCPI com only 
#undef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 9

#undef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 8

#undef PICO_DEFAULT_UART_BAUD_RATE
#define PICO_DEFAULT_UART_BAUD_RATE  115200

// EEprom parameter supported by SCPI command
// Add new parameter below using #define
#define CHECK         "CHECK"
#define PARTNUMBER    "PARTNUMBER"
#define SERIALNUMBER  "SERIALNUMBER"
#define MOD_OPTION    "MOD_OPTION"
#define COM_SER_SPEED "COM_SER_SPEED"
#define PSLAVE_RUN    "PICO_SLAVES_RUN"


// EEprom configuration structure
// Add new parameter on the structure below

typedef struct eecfg_t {
  char check[1];          // To check if eeprom read correctly the proof character
  char partnumber[12];
  char serialnumber[5];
  char mod_option[14];
  char com_ser_speed[6];
  char slave_force_run[1];    // force master RUN_EN = 1, useful for debug slaves pico
}cfg;


typedef union eef {
    struct eecfg_t cfg;
    char data[sizeof(cfg)];
}eep;


extern eep ee;

// Default value for EEprom structure
// Add default value for the new parameter below using #define
#define EE_CHECK_CHAR   '#'
#define EE_PARTNUMBER   "500-1000-010"
#define EE_SERIALNUMBER "00001"
#define EE_MOD_OPTION   "DAC,PWR"
#define EE_CSER_SPEED   "115200"
#define EE_SLAVE_RUN    "0"   // 0 = Disable (will toggle),  1 = Enable (Always 1)

// Assign default value to default eeprom structure
// Add new parameter at the end of the string below 
#define DEF_EEPROM {EE_CHECK_CHAR,EE_PARTNUMBER,EE_SERIALNUMBER,EE_MOD_OPTION,EE_CSER_SPEED, EE_SLAVE_RUN}

// Add parameter on struct ParamInfo  file fts_scpi.c  around line 850



#define  BEEP_I2C_FAIL  2  // Define burst of Beep
#define  BEEP_EEP_FAIL   3  // Define burst of Beep
#define  BEEP_VSYS_OUT   4  // Define burst of Beep
#define  BEEP_TEMP_HIGH  5  // Define burst of Beep
#define  BEEP_WATCHDOG   6  // Define burst of Beep

#define MAX_PICO_TEMP   60  // Maximum temperature acceptable for pico
#define MAX_VSYS_VOLT   6   // Maximum voltage to Pico Master Vsys
#define MIN_VSYS_VOLT   4   // Minimum voltage to Pico Master Vsys

#define DEFAULT_PWR_VAL 0xc1f  // Value of configuration register after ina219_init




void Hardware_Default_Setting();


#endif // 