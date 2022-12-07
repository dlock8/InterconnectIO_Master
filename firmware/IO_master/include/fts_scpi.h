//
   
/*
 * Copyright (c) 2021 Valentin Milea <valentin.milea@gmail.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef _FTS_SCPI_H_
#define _FTS_SCPI_H_



#include "scpi/scpi.h"


#ifdef __cplusplus
extern "C" {
#endif

// define
#define SCPI_INPUT_BUFFER_SIZE 256
#define SCPI_ERROR_QUEUE_SIZE 17


// "IDN?" fields. Their meanings are "suggestions" in the standard.
#define SCPI_IDN1 "FirstTestStation"      /* Manufacturer */
#define SCPI_IDN2 "InterconnectIO"       /* Model */
#define SCPI_IDN3 "2022A"               /* no suggestion */
#define SCPI_IDN4 "0.1"                /* Firmware level */

#define MAXROW 36   /* maximum number of rows */
#define MAXCOL 1    /* maximum number of columns */
#define MAXDIM 1    /* maximum number of dimensions */

#define RCLOSE  1     // Close relay tag
#define RCLEX   2     // Close Exclusive relay tag
#define ROPEN   3     // Open relay tag
#define ROPALL  4     // Open all relay tag
#define RSTATE  5     // Read relay state tag
#define BSTATE  6     // Read relay bank state tag
#define SESTATE 7     // Read relay single ended state tag
#define SECLOSE 8     // Close SE relay tag
#define SEOPEN  9     // Open SE relay tag

#define SDIR    10     // Set direction of digital IO
#define SBDIR   11    // Set direction of digital IO Bit
#define SOUT    12     // Set port digital Output
#define SBOUT   13     // Set bit on port digital Output
#define RIN     14     // Read port digital input
#define RBIN    15     // Read port bit  digital input
#define RDIR    16     // Read direction of digital IO port
#define RBDIR   17     // Read direction for a bit on a port
#define GPSDIR  18    // Set direction for a single GPIO on a particular device
#define GPRDIR  19     // Read direction for a single GPIO on a particular device
#define GPOUT   20     // Set output for a single gpio on a particular device
#define GPIN    21     // Read a single gpio on a particular device
#define GPSPAD  22     // Set digital IO PAD parameter
#define GPGPAD  23     // Read digital IO PAD parameter
#define PWCLOSE 24     // CLose power relay
#define PWOPEN  25     // OPen power relay
#define PWSTATE 26     // Read state of power relay

#define SBEEP   50     // Send Beep pulse
#define SVER    51     // Read version of Pico Master and slave
#define SLERR   52     // Control of error led
#define SRUN    53     // Send Beep pulse



#define SCPI_BANK1 1     // Open BK1 relay tag
#define SCPI_BANK2 2     // Open BK2 relay tag
#define SCPI_BANK3 3     // Open BK3 relay tag
#define SCPI_BANK4 4     // Open BK4 relay tag
#define SCPI_BANK_ALL 5     // Open all relay tag
#define SCPI_LPR1  6     // Power relay 
#define SCPI_LPR2  7     // Power relay 
#define SCPI_HPR1  8     // Power relay 
#define SCPI_SSD1  9     // Power Solid State relay 

#define GPIO_LPR1  12     // Gpio number to control Power relay 
#define GPIO_LPR2  13     // Gpio number to control Power relay 
#define GPIO_HPR1  14     // Gpio number to control Power relay 
#define GPIO_SSD1  15     // Gpio number to control Power relay 

#define GPIO_BEEP  11     // Gpio number used to drive the beeper
#define GPIO_RUN   18     // Gpio number used to control RUN of the slave
#define GPIO_LED   19     // Gpio number used to led monitor green
#define GPIO_SYNC  22     // Gpio number used to sync all module


#define BEEP_TIME   10      // Beep time in mS

extern scpi_t scpi_context;
extern char scpi_input_buffer[];
extern scpi_error_t scpi_error_queue[];
extern scpi_command_t scpi_commands[];


size_t write_scpi(scpi_t *context, const char *data, size_t len);

scpi_result_t Relay_Chanlst(scpi_t *context, uint16_t *array);

void init_scpi();

#endif // 