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

#define RCLOSE 1     // Close relay tag
#define RCLEX 2     // Close Exclusive relay tag
#define ROPEN 3     // Open relay tag
#define ROPALL 4     // Open all relay tag
#define RSTATE 5     // Read relay state tag
#define BSTATE 6     // Read relay bank state tag
#define SESTATE 7     // Read relay single ended state tag




#define SCPI_BANK1 1     // Open BK1 relay tag
#define SCPI_BANK2 2     // Open BK2 relay tag
#define SCPI_BANK3 3     // Open BK3 relay tag
#define SCPI_BANK4 4     // Open BK4 relay tag
#define SCPI_BANK_ALL 5     // Open all relay tag



extern scpi_t scpi_context;
extern char scpi_input_buffer[];
extern scpi_error_t scpi_error_queue[];
extern scpi_command_t scpi_commands[];


size_t write_scpi(scpi_t *context, const char *data, size_t len);

scpi_result_t Relay_Chanlst(scpi_t *context, uint16_t *array);

void init_scpi();

#endif // 