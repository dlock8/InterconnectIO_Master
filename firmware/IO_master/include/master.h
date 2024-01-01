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

#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1


// EEprom parameter supported by SCPI command
// Add new parameter below using #define

#define CHECK         "CHECK"
#define PARTNUMBER    "PARTNUMBER"
#define SERIALNUMBER  "SERIALNUMBER"
#define MOD_OPTION    "MOD_OPTION"
#define COM_SER_SPEED "COM_SER_SPEED"



// EEprom configuration structure
// Add new parameter on the structure below

typedef struct eecfg_t {
  char check[1];          // To check if eeprom read correctly the proof character
  char partnumber[12];
  char serialnumber[5];
  char mod_option[14];
  char com_ser_speed[6];
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

// Assign default value to default eeprom structure
// Add new parameter at the end of the string below 
#define DEF_EEPROM {EE_CHECK_CHAR,EE_PARTNUMBER,EE_SERIALNUMBER,EE_MOD_OPTION,EE_CSER_SPEED}

// Add parameter on struct ParamInfo  file fts_scpi.c  around line 800


void Hardware_Default_Setting();


#endif // 