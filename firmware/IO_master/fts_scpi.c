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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico_lib2/src/dev/dev_ds2431/dev_ds2431.h"
#include "pico_lib2/src/sys/include/sys_uart.h"
#include "pico_lib2/src/sys/include/sys_spi.h"
#include "include/scpi_user_config.h"
#include "include/test.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/master.h"
#include "hardware/resets.h"
#include "include/functadv.h"

#include "userconfig.h"         // contains Major and Minor version



//#include "scpi/scpi.h"
//#include "scpi/expression.h"

//#include "scpi/error.h"
//#include "scpi/parser.h"


// This will hold the SCPI "instance".
scpi_t scpi_context;


// Input buffer for reading SCPI commands.

char scpi_input_buffer[SCPI_INPUT_BUFFER_SIZE];

// Error queue required by the library. Size taken from the docs.

scpi_error_t scpi_error_queue[SCPI_ERROR_QUEUE_SIZE];


// Function used by module test.c to validate the selftest

static size_t output_buffer_write(const char * data, size_t len) {
    memcpy(out_buffer + out_buffer_pos, data, len);
    out_buffer_pos += len;
    out_buffer[out_buffer_pos] = '\0';
    return len;
}

// The function called by the SCPI library when it wants to send data.
size_t SCPI_write(scpi_t *context, const char *data, size_t len) {
 
    // added code to add null termination to the string data. uart_puts do not support parameter lenght.
    //char t[128];          // temporay buffer 

    char outbuf[SCPI_INPUT_BUFFER_SIZE];  // Global variable output buffer used to test code

    strncpy(outbuf,data,SCPI_INPUT_BUFFER_SIZE); // save data string to temporay buffer
    outbuf[len] = '\0';      // add string termination
    uart_puts(UART_ID, outbuf); // send answer to serial port
    output_buffer_write(data, len);  // USED BY TEST to capture output of the command

	return fwrite(data, 1, len, stdout); // send answer to usb port for help debug
}

// execute reset of all Pico
scpi_result_t SCPI_Reset(scpi_t * context) {
    (void) context;
    fprintf(stdout, "*Reset execute begin\r\n");
    // Put the MASTER PWM block into reset
    reset_block(RESETS_MASTER);
    // wait time
    // And bring it out
    unreset_block_wait(RESETS_MASTER);
    // Re-configure the hardware
    Hardware_Default_Setting();

    fprintf(stdout, "Reset execute completed\r\n");
    return SCPI_RES_OK;
}

// SCPI Flush is for clear the input buffer
static scpi_result_t SCPI_Flush(scpi_t * context) {
    (void) context;
    //fprintf(stdout, "SCPI Flush\r\n");
    return SCPI_RES_OK;
}

// Send a beep tone to alert for error
void SCPI_Beep() {
    gpio_put(GPIO_BEEP, 1); // Tuen ON beeper
    sleep_ms(BEEP_TIME);    // wait time
    gpio_put(GPIO_BEEP, 0); // Turn OFF beeper
}

// Send a burst of beep code to alert for boot error
void ErrorBeep(uint8_t nbeep){
    for (uint8_t j = 0; j < 3 ; j++) {
        for( uint8_t i=0; i <nbeep; i++){
            SCPI_Beep();
            sleep_ms(250);    // wait time between beep
        }
    sleep_ms(500);    // wait time between burst of beep
    }
}


int SCPI_Error(scpi_t * context, int_fast16_t err) {
    (void) context;

    if (err == 0 ) {  // if no error, do nothing
        gpio_put(GPIO_LED, 0); // Turn OFF led Error
    } else {
        SCPI_Beep();  // Beep for signal error
        gpio_put(GPIO_LED, 1); // Turn ON led Error 
        fprintf(stdout, "**ERROR: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
    }
    
    return SCPI_RES_OK;
}

scpi_reg_val_t srq_val = 0;

static scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    (void) context;

    fprintf(stdout, "SCPI Control\r\n");

    if (SCPI_CTRL_SRQ == ctrl) {
        srq_val = val;
    } else {
        fprintf(stdout, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }
    return SCPI_RES_OK;
}


// Additional callbacks to be used by the library.
scpi_interface_t scpi_interface = {
	.write = SCPI_write,
	.error = SCPI_Error,
    .control = SCPI_Control,
    .flush = SCPI_Flush, 
	.reset = SCPI_Reset,
};


// List of special string not already defined on SCPI parser
const scpi_choice_def_t scpi_special_all_numbers_def[] = {
    {/* name */ "ALL", /* type */ SCPI_BANK_ALL},
    {/* name */ "BANK1", /* type */ SCPI_BANK1},
    {/* name */ "BANK2", /* type */ SCPI_BANK2},
    {/* name */ "BANK3", /* type */ SCPI_BANK3},
    {/* name */ "BANK4", /* type */ SCPI_BANK4},
    {/* name */ "LPR1", /* type */ SCPI_LPR1},
    {/* name */ "LPR2", /* type */ SCPI_LPR2},
    {/* name */ "HPR1", /* type */ SCPI_HPR1},
    {/* name */ "SSR1", /* type */ SCPI_SSR1},
    {/* name */ "OC1", /* type */ SCPI_OC1},
    {/* name */ "OC2", /* type */ SCPI_OC2},
    {/* name */ "OC3", /* type */ SCPI_OC3},
    {/* name */ "ON", /* type */ 1},
    {/* name */ "OFF", /* type */ 0},
    {/* name */ "SPI", /* type */ SCPI_SPI},
    {/* name */ "UART", /* type */ SCPI_UART},
    {/* name */ "I2C", /* type */ SCPI_I2C},

    SCPI_CHOICE_LIST_END,
};



/**
 * Reimplement IEEE488.2 *TST?
 *
 * Result should be 0 if everything is ok
 * Result should be 1 if something goes wrong
 *
 * Return SCPI_RES_OK
 */
 // Hardware selftest
static scpi_result_t SCPI_CallbackTstQ(scpi_t * context) {
    fprintf(stdout,"Board Selftest execute \r\n");
    Board_Selftest();
    return SCPI_RES_OK;
}

/**
 * @brief
 * Set Bit on SCPI register Questionnable ann Operation
 * Send Bust of Beep to signal to operator the problem
 * Send Error to Erro Queue
 *
 * @param index Index to use on the Reginfo
 * @param scbit Set or Clear particular bit on register
  */
void RegBitHdwrErr(reg_info_index_t index, bool scbit){
    volatile uint16_t regp,regs;
    bool tbit;

    // Helper structure to store register information
    struct RegInfo {
        scpi_reg_name_t preg;    // name of primary error register (Error set or clear, mandatory)
        scpi_reg_name_t sreg;   // name of secondary register (Error set only,if required)
        scpi_reg_val_t pbit;    // Bit to set or clear in primary register
        scpi_reg_val_t sbit;    // Bit to set on secondary register (normally event register)
        uint8_t nbBeep;         // Number of Beep burst for Set error
        int16_t scpierror;      // SCPI error to push in case of Set Error
    };

    // Define an array of ErrorInfo structs for set value in primary register,
    // set value in secondary register (event register), send beep code burst to operator
    // and raise SCPI error message

    struct RegInfo mreg[] = {
        {SCPI_REG_QUESC,SCPI_REG_OPERC,QCR_I2C_COM,OPER_BOOT_FAIL,BEEP_I2C_FAIL,I2C_COMMUNICATION_ERROR},
        {SCPI_REG_QUESC,SCPI_REG_OPERC,QCR_VSYS_OUTLIMIT,OPER_BOOT_FAIL,BEEP_VSYS_OUT,VSYS_OUT_LIMITS},
        {SCPI_REG_QUESC,SCPI_REG_OPERC,QCR_MTEMP_HIGH,OPER_BOOT_FAIL,BEEP_TEMP_HIGH,TEMP_MASTER_HIGH},
        {SCPI_REG_QUESC,SCPI_REG_OPERC,QCR_WATCHDOG,OPER_BOOT_FAIL,BEEP_WATCHDOG,WATCHDOG_TRIG},
        {SCPI_REG_QUESC,0,QCR_EEP_READ_ERROR,0,BEEP_EEP_FAIL,SCPI_ERROR_MEMORY_USE_ERROR},
    };


    if (!scbit) {  // if bits need to be set or clear
        SCPI_RegSetBits(&scpi_context,mreg[index].preg,1 << mreg[index].pbit);  
    } else {
        SCPI_RegClearBits(&scpi_context,mreg[index].preg,0 << mreg[index].pbit);
    }

    if (!scbit && mreg[index].sreg > 0 ) {         // if required to update the secondary register
       SCPI_RegSetBits(&scpi_context,mreg[index].sreg,1 << mreg[index].sbit);
    }

    if (!scbit){   // if error bit has been set, send error message
        ErrorBeep(mreg[index].nbBeep);                                  // Send Beep Error code
        SCPI_ErrorPush(&scpi_context, mreg[index].scpierror);           // push error on  SCPI     
    }
}




/**
 * @brief
 * parses lists
 * channel numbers > 0.
 * no checks yet.
 * Only 1 dimension
 *
 * @param channel_list channel list, compare to SCPI99 Vol 1 Ch. 8.3.2
 */
scpi_result_t Relay_Chanlst(scpi_t *context, uint16_t *array) {
    scpi_parameter_t channel_list_param;

   // scpi_channel_value_t array[MAXROW * MAXCOL]; /* array which holds values in order (2D) */
    size_t chanlst_idx; /* index for channel list */
    size_t arr_idx = 0; /* index for array */
    size_t n, m = 1; /* counters for row (n) and columns (m) */

    /* get channel list */
    if (SCPI_Parameter(context, &channel_list_param, TRUE)) {
        scpi_expr_result_t res;
        scpi_bool_t is_range;
        int32_t values_from[MAXDIM];
        int32_t values_to[MAXDIM];
        size_t dimensions;

        bool for_stop_row = FALSE; /* true if iteration for rows has to stop */
        bool for_stop_col = FALSE; /* true if iteration for columns has to stop */
        int32_t dir_row = 1; /* direction of counter for rows, +/-1 */
        int32_t dir_col = 1; /* direction of counter for columns, +/-1 */

        /* the next statement is valid usage and it gets only real number of dimensions for the first item (index 0) */
        if (!SCPI_ExprChannelListEntry(context, &channel_list_param, 0, &is_range, NULL, NULL, 0, &dimensions)) {
            chanlst_idx = 0; /* call first index */
            arr_idx = 0; /* set arr_idx to 0 */
            do { /* if valid, iterate over channel_list_param index while res == valid (do-while cause we have to do it once) */
                res = SCPI_ExprChannelListEntry(context, &channel_list_param, chanlst_idx, &is_range, values_from, values_to, 4, &dimensions);
                if (is_range == FALSE) { /* still can have multiple dimensions */
                    if (dimensions == 1) {
                        /* here we have our values
                         * row == values_from[0]
                         * col == 0 (fixed number)
                         * call a function or something */
                        array[arr_idx] = values_from[0];
                      //  array[arr_idx].col = 0;
                    } else {
                        return SCPI_RES_ERR;
                    }
                    arr_idx++; /* inkrement array where we want to save our values to, not neccessary otherwise */
                    if (arr_idx >= MAXROW * MAXCOL) {
                        return SCPI_RES_ERR;
                    }
                } else if (is_range == TRUE) { 
                    if (values_from[0] > values_to[0]) {
                        dir_row = -1; /* we have to decrement from values_from */
                    } else { /* if (values_from[0] < values_to[0]) */
                        dir_row = +1; /* default, we increment from values_from */
                    }

                    /* iterating over rows, do it once -> set for_stop_row = false
                     * needed if there is channel list index isn't at end yet */
                    for_stop_row = FALSE;
                    for (n = values_from[0]; for_stop_row == FALSE; n += dir_row) {
                      
                        if (dimensions == 1) {
                            /* here we have values
                             * row == n
                             * col == 0 (fixed number)
                             * call function or sth. */
                            array[arr_idx] = n;
                          //  array[arr_idx].col = 0;
                            arr_idx++;
                            if (arr_idx >= MAXROW * MAXCOL) {
                                return SCPI_RES_ERR;
                            }
                        }
                        if (n >= (size_t)values_to[0]) {  /* endpoint reached, stop row for-loop */
                            for_stop_row = TRUE;
                        }
                    }
                } else {
                    return SCPI_RES_ERR;
                }
                /* increase index */
                chanlst_idx++;
            } while (SCPI_EXPR_OK == SCPI_ExprChannelListEntry(context, &channel_list_param, chanlst_idx, &is_range, values_from, values_to, 4, &dimensions));
            /* while checks, whether incremented index is valid */
        }
        /* do something at the end if needed */
         array[arr_idx] = 0;   // flag for end of array
        /* array[arr_idx].col = 0; */
    }

    {
        size_t i;
        fprintf(stdout, "Channel List: ");
        for (i = 0; i< arr_idx; i++) {
            fprintf(stdout, "%d,", array[i]);
            
        }
        fprintf(stdout, "\r\n");
    

    }
    return SCPI_RES_OK;
}




static scpi_result_t Callback_Relay_scpi(scpi_t *context) {
    scpi_parameter_t channel_list_param;
    volatile scpi_bool_t ValMatch;
    uint16_t tag, Vcomp, *px;
    char cdat,fres;
    volatile scpi_result_t  flag;
    uint16_t array[MAXROW * MAXCOL]; /* array which holds values in order (2D) */
    uint16_t answer[MAXROW * MAXCOL];
    size_t i = 0;
    char str[80];


    tag = SCPI_CmdTag(context);   //extract tag from the command

    fprintf(stdout,"tagvalue: %d\r\n", tag);

    flag = Relay_Chanlst(context, array);  // extract list of relay
    if (flag == SCPI_RES_ERR) {
        SCPI_ErrorPush(context, SCPI_RELAYS_LIST_ERROR);
        return SCPI_RES_ERR;
    }

    
    fres =  relay_execute(array,tag,answer); // Perform action requested
    if (!fres) {
        fprintf(stdout,"Relay error: %d\r\n", answer);
        SCPI_ErrorPush(context, answer[0]);
        return SCPI_RES_ERR;
    }

    if (tag == RSTATE) { // if returned value is expected
        do {  // loop on array until list of value is completed
            sprintf(str, "%d,", answer[i]);
            SCPI_ResultUInt8(context,answer[i]); // return SCPI value 
            i++;
        } while (array[i] > 0);
        printf(str);
        printf ("\r\n");  
    }


        fprintf(stdout, "Channel List from main: ");
        i = 0;
        do {
            fprintf(stdout, "%d,", array[i]);
            i++;
        } while (array[i] > 0);
        fprintf(stdout, "\r\n Channel List completed \r\n ");

    //SCPI_ResultText(context, "termine avec succes");

    return SCPI_RES_OK;

}


// Open  particular Relay bank or all relay
static scpi_result_t Callback_Relay_all_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_number_t paramRelay;
    uint16_t array[5]; //, se[5];
    uint16_t answer[MAXROW * MAXCOL];
    uint8_t tag;
    size_t i = 0;
    char str[80];

    tag = SCPI_CmdTag(context);   //extract tag from the command

    while(SCPI_ParamNumber(context, scpi_special_all_numbers_def, &paramRelay, FALSE)){
         //fprintf(stdout,"on switch \r\n");
        if (paramRelay.special) {
            switch (paramRelay.content.tag) {
                case SCPI_BANK1: 
                    array[i] = 10;    //  Add Bank1 on array list to open/close
                    //se[i] = SE_BK1;
                    i++;  
                    break;
                case SCPI_BANK2:        //  Add Bank2 on array list to open/close
                    array[i] = 20;
                    //se[i] = SE_BK2;
                    i++;  
                    break;
                case SCPI_BANK3:        //  Add Bank3 on array list to open/close
                    array[i] = 30;
                    //se[i] = SE_BK3;
                    i++;  
                    break;
                case SCPI_BANK4:        //  Add Bank4 on array list to open/close
                    array[i] = 40;
                    //se[i] = SE_BK4;
                    i++;  
                    break;

                case SCPI_BANK_ALL: 
                    array[i] = 10;     // Add all banks to the list
                    array[i+1] = 20;
                    array[i+2] = 30;
                    array[i+3] = 40;
                    i +=4;  
                    break;

                // Powe relay. offset of 500,600 or 700 is added to distinguishe from bank relay numbering
                case SCPI_LPR1: // Offset 600 = Pico Slave #2
                    array[i] = 600 +GPIO_LPR1;    //  Add gpio of lpr1  on array list to open/close
                    i++;  
                    break;

                case SCPI_LPR2: 
                    array[i] = 600 +GPIO_LPR2;    //  Add gpio of lpr2  on array list to open/close
                    i++;  
                    break;

                case SCPI_HPR1:   // Offset 700 = Pico Slave #3
                    array[i] = 700 +GPIO_HPR1;    //  Add gpio of HPR1  on array list to open/close
                    i++;  
                    break;
                
                case SCPI_SSR1:
                    array[i] = 700 +GPIO_SSR1;    //  Add gpio of SSR1 on array list to open/close
                    i++;  
                    break;

                // Open collector. offset of 500,600 or 700 is added to distinguishe from bank relay numbering
                case SCPI_OC1: // Offset 500 = Pico Slave #1
                    array[i] = 500 +GPIO_OC1;    //  Add gpio of OC1  on array list to open/close
                    i++;  
                    break;

                // Open collector. offset of 700,500 or 600 is added to distinguishe from bank relay numbering
                case SCPI_OC2: // Offset 600 = Pico Slave #2
                    array[i] = 600 +GPIO_OC2;    //  Add gpio of OC2  on array list to open/close
                    i++;  
                    break;

                // Open collector. offset of 700,500 or 600 is added to distinguishe from bank relay numbering
                case SCPI_OC3: // Offset 700 = Pico Slave #3
                    array[i] = 700 +GPIO_OC3;    //  Add gpio of OC3  on array list to open/close
                    i++;  
                    break;
                
                default: 
                    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                    return SCPI_RES_ERR;
            }
        }    

    }
    array[i] = 0;   // flag for end of array
    

    if (i > 0) {
       res =  relay_execute(array,tag,answer); // Perform action requested
       
    } else { // in case of i = 0
        if (!SCPI_ParamErrorOccurred(context)) { // if no error already occured
            SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER); // raise error
            return SCPI_RES_ERR;
        }else {return SCPI_RES_ERR;} // reurn error flag due to error occured
        
    }


    i=0;  // reset loop counter
    if (tag == BSTATE || tag == SESTATE || tag == PWSTATE || tag == OCSTATE ) { // if returned value is expected
        do {  // loop on array until list of value is completed
           fprintf(stdout, " 0x%x,", answer[i]); // print value on debug port
           SCPI_ResultUInt8(context,answer[i]); // return SCPI value 
            i++;
        } while (array[i] > 0);
        fprintf (stdout,"\r\n"); // send new line

    }
   
return SCPI_RES_OK;
}


// Digital Scpi command execution
static scpi_result_t Callback_Digital_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    uint16_t answer[1];
    uint8_t tag;
    uint32_t value = 0;
    int32_t numbers[2] = {0,0};  // initialise array to get a know value in case of only 1 number

fprintf(stdout, "On digital execute \r\n");
res = SCPI_Parameter(context, &param1, FALSE);

if (res) {
    // Is parameter a number without suffix?
    if (SCPI_ParamIsNumber(&param1, FALSE)) {
        // Convert parameter to unsigned int. Result is in value.
        SCPI_ParamToUInt32(context, &param1, &value);
    }
}
    
    SCPI_CommandNumbers(context, numbers, 2, 2); // fill array with command number

    fprintf(stdout, "Digital TEST numbers %d %d\r\n", numbers[0], numbers[1]);

    tag = SCPI_CmdTag(context);   //extract tag from the command

    if (numbers[0] > 1 || numbers[1] > 7) {  // if port or bit number is out of limit, return error
        answer[0] = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;  
        fprintf(stdout,"Error on command: Data out of range for PORT{0-1} or BIT{0-7}  \r\n");
        SCPI_ErrorPush(context, answer[0]);
        return SCPI_RES_ERR;
    }

    res = digital_execute(tag, numbers[0], numbers[1], value,answer); // execute xommand

    if (!res) {   // if failure found during command
        fprintf(stdout,"Digital error: %d\r\n", answer);
        SCPI_ErrorPush(context, answer[0]);
        return SCPI_RES_ERR;
    }


    if (tag==RDIR|| tag==RBDIR|| tag==RIN|| tag==RBIN) { // if returned value is expected
    
        fprintf(stdout, "Value read:  0x%x,\r\n", answer[0]); // return value on debug port
        SCPI_ResultUInt8(context,answer[0]); // return SCPI value 

    }

return SCPI_RES_OK;
}

// Low level command
static scpi_result_t Callback_gpio_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    uint16_t answer[1];  // will contains the answer returned by command
    int32_t numbers[2] = {0,0};  // initialise array to get a know value in case of only 1 number
    uint8_t tag;
    uint32_t value = 0;

    fprintf(stdout, "On gpio execute \r\n");
    res = SCPI_Parameter(context, &param1, FALSE);

    if (res) {
        // Is parameter a number without suffix?
        if (SCPI_ParamIsNumber(&param1, FALSE)) {
        // Convert parameter to unsigned int. Result is in value.
        SCPI_ParamToUInt32(context, &param1, &value);
        }
    }


    SCPI_CommandNumbers(context, numbers, 2, 2); //Create array of number

    fprintf(stdout, "GPIO TEST numbers %d %d\r\n", numbers[0], numbers[1]);

    tag = SCPI_CmdTag(context);   //extract tag from the command


    if (numbers[0] > 3 || numbers[1] > 28) {  // if device or gpio number is out of limit, return error
        answer[0] = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;  
        fprintf(stdout,"Error on command: Data out of range for DEVice{0-3} or GPio{0-28} \r\n");
        SCPI_ErrorPush(context, answer[0]);
        return SCPI_RES_ERR;
    }

    res = gpio_execute(tag, numbers[0], numbers[1], value,answer); // execute command

    if (!res) {   // if failure found during command
        fprintf(stdout,"Gpio execute error: %d\r\n", answer);
        SCPI_ErrorPush(context, answer[0]);
        return SCPI_RES_ERR;
    }

    if (tag==GPIN||  tag==GPRDIR|| tag==GPGPAD) { // if returned value is expected   
        fprintf(stdout, "GPIO Value read:  0x%x,\r\n", answer[0]); // return value on debug port
        SCPI_ResultUInt8(context,answer[0]); // return SCPI value 
    }

return SCPI_RES_OK;
}


// System level command
static scpi_result_t Callback_system_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    scpi_number_t paramRun;
    uint16_t ans[8];  // will contains the answer returned by command
    char pv[30]; 
   // int32_t numbers[2] = {0,0};  // initialise array to get a know value in case of only 1 number
volatile    uint8_t tag;
    uint32_t value = 0;
    float fval;

    fprintf(stdout, "On system execute \r\n");

    res = SCPI_Parameter(context, &param1, FALSE);

    // if ON or OFF, transform to value 0 or 1
    if (res && param1.type == SCPI_TOKEN_PROGRAM_MNEMONIC) {
       SCPI_ParamToChoice(context, &param1, scpi_special_all_numbers_def, &value);
    }
 
    if (res) {
        // Is parameter a number without suffix?
        if (SCPI_ParamIsNumber(&param1, FALSE)) {
        // Convert parameter to unsigned int. Result is in value.
        SCPI_ParamToUInt32(context, &param1, &value);
        }
    }

    tag = SCPI_CmdTag(context);   //extract tag from the command


    switch (tag) {
        case SBEEP:
            fprintf (stdout, "Scpi command beep \r\n");
            SCPI_Beep(); 
            break;

        case SVER:
            fprintf (stdout, "Scpi command pico version \r\n");
            res = system_execute(tag,ans); // get arrays of version
            if (res) { //if no failure detected 
                // Build string to be returned base on the array of version received
                sprintf(pv,"%d.%d, %d.%d, %d.%d, %d.%d",ans[0],ans[1],ans[2],ans[3],ans[4],ans[5],ans[6],ans[7]);
                fprintf(stdout,pv); // print string version for the 4 devices
                fprintf(stdout,"\n"); // print newline
                SCPI_ResultText(context,pv); // sent result
            } else {
                  // if failure found during command
                fprintf(stdout,"System execute error: %d\r\n", ans);
                SCPI_ErrorPush(context, ans[0]);
                return SCPI_RES_ERR;
            }
            break;

        case GSTA:
            fprintf (stdout, "Scpi command to get pico device status \r\n");
            res = system_execute(tag,ans); // get arrays of version
            if (res) { //if no failure detected 
                // Build string to be returned base on the array of byte received
                sprintf(pv,"Slave1: 0x%x, Slave2: 0x%x, Slave3: 0x%x",ans[0],ans[1],ans[2]);
                fprintf(stdout,pv); // print string version for the 3 slaves
                fprintf(stdout,"\n"); // print newline
                SCPI_ResultText(context,pv); // sent result
            } else {
                  // if failure found during command
                fprintf(stdout,"System execute error: %d\r\n", ans);
                SCPI_ErrorPush(context, ans[0]);
                return SCPI_RES_ERR;
            }
            break;

    
        case SLERR:  // Ctrl of led error
            fprintf (stdout, "Set Error led on gpio %d to: %d \r\n",GPIO_LED,value);
            gpio_put(GPIO_LED, value);
            break;

        case GLERR:  // Read led error
            value = gpio_get(GPIO_LED);
            fprintf (stdout, "Read Error led on gpio %d ,value: %d \r\n",GPIO_LED, value);
            SCPI_ResultUInt8(context,value); // return SCPI value 
            break;
        
        case SRUN:  // System Pico RUN_EN,
            fprintf (stdout, "Set Pico RUN_EN gpio %d to: %d \r\n",GPIO_RUN,value);
            gpio_put(GPIO_RUN, value);
            if (!value) {  // Set or Clear User Request bit on ESR (set when slaves are disabled)   
                SCPI_RegSetBits(context,SCPI_REG_ESR,1 << ESR_USER_BIT );  
            } else {
                SCPI_RegClearBits(context,SCPI_REG_ESR,0 << ESR_USER_BIT );
            }
            break;

        case GRUN:  // Read Run enable value
            value = gpio_get(GPIO_RUN);
            fprintf (stdout, "Read Slave Run_EN on gpio %d ,value: %d \r\n",GPIO_RUN, value);
            SCPI_ResultUInt8(context,value); // return SCPI value 
            break;

        case SOE:  // System Output enable
            fprintf (stdout, "Set Output Enable gpio %d to: %d \r\n",GPIO_OE, value);
            gpio_put(GPIO_OE, value);
            if (value) {  // Set or Clear Power ON bit on ESR
                SCPI_RegSetBits(context,SCPI_REG_ESR,1 << ESR_PON_BIT );  
            } else {
                SCPI_RegClearBits(context,SCPI_REG_ESR,0 << ESR_PON_BIT );
            }
            break;

        case GOE:  // Read System Ouput enable
            value = gpio_get(GPIO_OE);
            fprintf (stdout, "Read System Output Enable on gpio %d ,value: %d \r\n",GPIO_OE, value);
            SCPI_ResultUInt8(context,value); // return SCPI value 
            break;
        
        default: 
            break;
    }
    return SCPI_RES_OK;
}

// Low level command
static scpi_result_t Callback_analog_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    uint16_t answer[1];  // will contains the answer returned by command
    uint8_t tag,ecode;
    float value = 0;
    float value2 = 0;
    bool retv; 
    

    fprintf(stdout, "On analog execute \r\n");

    tag = SCPI_CmdTag(context);   //extract tag from the command

    if ( tag == SDAC || tag == WDAC || tag == CPI ) {
   
        res = SCPI_Parameter(context, &param1, TRUE);  // Read first parameter
        if (res) {
            // Is parameter a number without suffix?
            if (SCPI_ParamIsNumber(&param1, TRUE)) {
                // Convert parameter to float. Result is in value.
                SCPI_ParamToFloat(context, &param1, &value);
            }
        }

        if ( tag == CPI) {  // if second parameter expected
            res = SCPI_Parameter(context, &param1, TRUE);  // read parameter #2 
            if (res) {
                // Is parameter a number without suffix?
                if (SCPI_ParamIsNumber(&param1, TRUE)) {
                // Convert parameter to float. Result is in value.
                SCPI_ParamToFloat(context, &param1, &value2);
                }
            }
        }
    }


    switch (tag) {
        case SDAC:
            ecode=dac_set(value,false); //set value
            retv = false;   // no value returned
            break;
    
        case WDAC:
            ecode=dac_set(value,true); //set value and save as default
            retv = false;   // no value returned
            break;

        case RADC0:
            value = read_master_adc(0);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RADC1:
            value = read_master_adc(1);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RADC3:
            value = read_master_adc(3);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RADC4:
            value = read_master_adc(4);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RPV:
            value = read_power(0);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RPI:
            value = read_power(1);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RPP:
            value = read_power(2);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case RPS:
            value = read_power(3);
            ecode = NOERR;
            retv = true;   //  value returned
            break;

        case CPI:
            calibrate_power(value, value2);
            retv = false;   //  no value to return
            break;
    }

    // raise error if is the case
    switch (ecode) {
        case NOERR:
            break;

        case EOOR:
            answer[0] = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;  
            SCPI_ErrorPush(context, answer[0]); 
            break;

        case EDE:
             answer[0] = SCPI_ERROR_EXECUTION_ERROR;
             SCPI_ErrorPush(context, answer[0]);
             break; 
    }

    if (retv) { // if returned value is expected   
        SCPI_ResultFloat(context,value); // return SCPI value 
    }

return SCPI_RES_OK;
}


// callback for read or write eeprom single parameter
static scpi_result_t Callback_eeprom_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    uint16_t answer;  // will contains the answer returned by command
    uint8_t tag;
    char* split;
    uint8_t status = NOERR;
    char mode;
    char sfull[64], pstr[96];
    uint32_t  value;    // value of parameter
    char varname[32], svalue[32];
    size_t i;
    char *tnptr;
    long cnum;


        // Helper structure to store member information
    struct ParamInfo {
        const char* name;   // name of parameter
        size_t offset;      // offset on the structure for the parameter
        size_t size;        // size of the parameter
        bool isnum;         // set to TRUE if number is mandatory
    };

    for(i=0;i<32;i++) { svalue[i] = '\0';} // initialize array


    // Define an array of MemberInfo structs with member names, offsets, and sizes
    // Each parameter to Read/Write on EEPROM need to be added on this array
    // The las field (true or false is to validate if the parameter is a number or not 

    struct ParamInfo members[] = {
        {CHECK, offsetof(cfg,check), sizeof(((cfg *)0)->check),FALSE},
        {PARTNUMBER, offsetof(cfg,partnumber), sizeof(((cfg *)0)->partnumber),FALSE},
        {SERIALNUMBER, offsetof(cfg,serialnumber),sizeof(((cfg *)0)->serialnumber),FALSE},
        {MOD_OPTION, offsetof(cfg,mod_option),sizeof(((cfg *)0)->mod_option),FALSE},
        {COM_SER_SPEED, offsetof(cfg,com_ser_speed),sizeof(((cfg *)0)->com_ser_speed),TRUE},
        {PSLAVE_RUN, offsetof(cfg,slave_force_run),sizeof(((cfg *)0)->slave_force_run),TRUE},
    };

    fprintf(stdout, "\n\nOn eeprom execute \r\n");
    tag = SCPI_CmdTag(context);   //extract tag from the command

     //calculate the number of members
    size_t numMembers = sizeof(members) / sizeof(members[0]);

    switch (tag) {
        case WDEF:
            status = cfg_eeprom_write_default();
            break;
        case RFUL:
            status = cfg_eeprom_read_full();
            if (status != NOERR) {break;}   // if error do not execute the eeprom reading
            fprintf(stdout, "\n\nEEprom full content: \r\n");
            for ( i = 0; i < numMembers; ++i) {
                // copy parameter to string
                strncpy(sfull,&ee.data[members[i].offset],members[i].size);
                sfull[members[i].size] = '\0';  // add end of string to temporary buffer
                sprintf(pstr,"%s = %s\n",members[i].name,sfull);  // build string to return
                // fprintf(stdout,"Parameter %s\n",pstr);
                SCPI_ResultCharacters(context,pstr,strlen(pstr)); // return value
            }
            break;
        case WEEP:
            mode = 'w';   // write mode
            break;
        case REEP:
            mode = 'r';   // write mode
            break;
    }

    // Run only for read write parameter on eeprom
    if (tag == WEEP || tag == REEP) {
        res = SCPI_Parameter(context, &param1, true);  // Read first parameter
        if (res == false) { return SCPI_RES_ERR;}  // if no parameter read, raise error and exit

        // Split the string to distinguishes between varnames and the svalue
        split = strtok (param1.ptr," ',");
        strcpy(varname,split);
        strupr(varname);
        fprintf(stdout,"EEprom varname = %s\n",varname);

        if (tag  == REEP){  // if tag  = Read command
            mode = 'r';
        }
    
        if (tag  == WEEP){ // if tag  = Write command
            mode = 'w';
            split = strtok (NULL," ',\r\n");  // Extract second parameter from the command
            strcpy(svalue,split);   // copy second paramter to svalue
            strupr(svalue);         // change lowercase to upper case
            if (svalue[0] == '\0') {   // if svalue not present, raise error
                fprintf(stdout,"Error, no svalue to write on eeprom \n");
                status = EMP;  // Set error flag
            } else { fprintf(stdout,"EEprom svalue = %s\n",svalue); }
        }

        bool found = false;

        if (status == NOERR) {  // if no error found.
            // Loop through the member info array to find matching member name
            for ( i = 0; i < numMembers; ++i) {
                if (strcmp(varname, members[i].name) == 0) {
                    fprintf(stdout,"Cfg struct parameter: %s , offset: %u, size: %u\n",varname, members[i].offset,members[i].size );
                    found = true;
                    break;
                }
            }
            if (!found) {
                    status = EIVN;
            }

            // check if svalue is a number in case we expect number
            if (found == true && mode == 'w' && members[i].isnum == 1) {
                long number;
                int check =stringtonumber( svalue, &number);
                if (check != 0) {   
                    status = ENDE;      // raise error if not a number
                }
            }

            volatile int tval = strlen(svalue);
            if (found && status == NOERR) { // if varname found
                status = cfg_eeprom_rw(mode,members[i].offset,members[i].size,svalue,strlen(svalue));    // write or read data on eeprom
                 if (status == NOERR) {
                    strncpy(&ee.data[members[i].offset], svalue, members[i].size);  // save parameter on eeprom structure
                    if (mode == 'r') {
                        SCPI_ResultCharacters(context,&ee.data[members[i].offset],members[i].size); // return value
                    }
                }
            }
        }
    }
      // raise error if is the case
    switch (status) {
        case NOERR: { break;}
        case ENDE:  { answer = SCPI_ERROR_NUMERIC_DATA_ERROR; break;}
        case EOOR:  { answer = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE; break;}
        case EIVN:  { answer = SCPI_ERROR_ILLEGAL_VARIABLE_NAME; break;}
        case ECE:   { answer = SCPI_ERROR_CHARACTER_DATA_ERROR; break;}
        case EDE:   { answer= SCPI_ERROR_EXECUTION_ERROR; break;}
        case ERE: case EBE:{ answer= SCPI_ERROR_MASS_STORAGE_ERROR; break; } 
        case EMP:   { answer = SCPI_ERROR_MISSING_PARAMETER; break;}
    }    
    if (status != NOERR) {
        SCPI_ErrorPush(context, answer);  // push errors 
        return SCPI_RES_ERR;
    } else {

        return SCPI_RES_OK;
    }

}


// Low level command
static scpi_result_t Callback_com_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    scpi_number_t paramCom;

    uint16_t answer;  // will contains the answer returned by command
    uint8_t tag,ecode;
    uint32_t val=0;
    size_t lgt,eid;
    bool retv = false;
    bool bval;
    const char *dpr;

    char* sdata = NULL;
    char winfo[SCPI_INPUT_BUFFER_SIZE];   // big string to contents filtered data
    char ustr[SCPI_INPUT_BUFFER_SIZE];    // big string to get temporary data
                                                                
    fprintf(stdout, "\nOn communication execute \r\n");

    winfo[0] = '\0';


    ecode =NOERR;
    tag = SCPI_CmdTag(context);   //extract tag from the command

    fprintf(stdout, "Tag = %d \r\n", tag);
    
    if (tag == CSWH) { 
        // if ON or OFF, transform to value 0 or 1
        res = SCPI_Parameter(context, &param1, true);  // Read first parameter
        if (res && param1.type == SCPI_TOKEN_PROGRAM_MNEMONIC) {
           SCPI_ParamToChoice(context, &param1, scpi_special_all_numbers_def, &val);
        } else {
               // Convert parameter to number, result  is in value.
                 SCPI_ParamToUInt32(context, &param1, &val);
        }
    }


    if ( tag == C1W || tag == R1W || tag == CSWB || tag == CSWT || tag == SPWF || tag == SPWM  ) {
        res = SCPI_Parameter(context, &param1, true);  // Read first parameter
        if (res) {
            // Is parameter a number without suffix?
            if (SCPI_ParamIsNumber(&param1, TRUE)) {
                // Convert parameter to unsigned int. Result is in value.
                 SCPI_ParamToUInt32(context, &param1, &val);
                 eid = val; // change number to size_t
            }
        }
    }



    if ( tag == W1W || tag == CSWP  ) {
        res = SCPI_Parameter(context, &param1, true);  // Read first parameter
        if (res) {
            char str[NB_INFO];
            size_t i,j;
            j = 0;
            strcpy(str,param1.ptr);
            for (i = 0; i < strlen(str); i++) {
                //remove not informative character from string
                if (str[i] != '\'' && str[i] != '"' && str[i] != '\n' && str[i] != '\r') {
                    winfo[j++] = str[i];
                }
            }
        }
    }

    if ( tag == CSWD || tag == CSRD) {
        if (tag == CSWD) { res = SCPI_Parameter(context, &param1, true); } // Read parameter mandatory
        if (tag == CSRD) { res = SCPI_Parameter(context, &param1, false); } // Read parameter, not mandatory
        if (res) {
            size_t i,j,ctr;
            j = 0;
            strcpy(ustr,param1.ptr);
            for (i = 0; i < strlen(ustr); i++) {
                //remove  comma character from string
                if (ustr[i] != '\'' && ustr[i] != '"'){ 
                    winfo[j++] = ustr[i];
                } else { ctr = j;}  //save pointer to detect end of string
            }
            winfo[ctr] = '\0'; // add end of string marker
        }
    }


    if ( tag == CIE || tag == CID || tag == CRI  ) {
            // extract special word as parameter 
           while(SCPI_ParamNumber(context, scpi_special_all_numbers_def, &paramCom, FALSE)){
                if (paramCom.special) {
                    switch (paramCom.content.tag) {
                        case SCPI_SPI: 
                            if (tag == CIE) {
                                sys_spi_enable();
                                fprintf(stdout, "Enable SPI communication\r\n");
                            }
                            if (tag == CID) {
                                sys_spi_disable();
                                fprintf(stdout, "Disable SPI communication\r\n");
                            }
                            if (tag == CRI) {
                                bval = sys_spi_status();
                                fprintf(stdout, "Read status SPI communication: %d\r\n",bval);
                                SCPI_ResultBool(context,bval);
                            }
                        break;

                        case SCPI_UART: 
                            if (tag == CIE) {
                                sys_uart_enable();
                                fprintf(stdout, "Enable UART communication\r\n");
                            }
                            if (tag == CID) {
                                sys_uart_disable();
                                fprintf(stdout, "Disable UART communication\r\n");
                            }
                            if (tag == CRI) {
                                bval = sys_uart_status();
                                fprintf(stdout, "Read status UART communication: %d\r\n",bval);
                                SCPI_ResultBool(context,bval);
                            }
                        break;
                    }
                }

           }
    }

    switch (tag) {
        case C1W: 
            ecode = onewire_check_devices(&sdata, eid );     //check presence of one wire 
            SCPI_ResultText(context,sdata);
            retv = true;   //  value returned
            break;

        case R1W: 
            ecode = onewire_read_info(&sdata,ADDR_INFO,NB_INFO,eid );  
            SCPI_ResultText(context,sdata);
            retv = true;   //  value returned
            break;

        case W1W: 
            ecode = onewire_write_info(winfo,ADDR_INFO);  
            SCPI_ResultText(context,sdata);
            retv = true;   // value returned
            break;

        case CSWB: 
            fprintf(stdout, "Uart set Baudrate to %d\r\n", val);
            sys_uart_set_baudrate(val);  
            retv = false;   // no value returned
            break;

        case CSRB: 
            val = sys_uart_get_baudrate();
            fprintf(stdout, "Uart readback actual Baudrate, speed= %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            retv = true;   // no value returned
            break;

        case CSWT: 
            fprintf(stdout, "Uart set Timeout_ms to %d\r\n", val);
            sys_uart_set_timeout(val);  
            retv = false;   // no value returned
            break;

        case CSRT: 
            val = sys_uart_get_timeout();
            fprintf(stdout, "Uart readback Timeout_ms: %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            retv = true;   // value returned
            break;

        case CSWH: 
            fprintf(stdout, "Uart set RTS-CTS Handshake to %d\r\n", val);
            sys_uart_set_handshake(val);  
            retv = false;   // no value returned
            break;

        case CSRH: 
            bval = sys_uart_get_handshake();
            fprintf(stdout, "Uart readback RTS-CTS Handshake: %d\r\n", bval); 
            SCPI_ResultBool(context,bval);
            retv = true;   //  value returned
            break;
        
        case CSWP: 
            ecode = sys_uart_set_protocol(winfo);
            if (ecode != NOERR) { 
                fprintf(stdout, "Uart protocol error with value: %s\r\n", &winfo);
            } else {
                fprintf(stdout, "Uart set protocol to: %s\r\n", &winfo); 
            }
            retv = false;   //  value returned
            break;

        case CSRP: 
            dpr = sys_uart_get_protocol();
            fprintf(stdout, "Uart readback protocol: %s\r\n", dpr); 
            SCPI_ResultText(context,dpr);
            retv = true;   //  value returned
            break;

        case CSWD: // Write data to uart only, the answer is discarded
            fprintf(stdout, "Uart transmit data: %s\r\n", &winfo);
            sys_uart_write_data(winfo); // write data, do not expect answer
            retv = false;   //  value returned
            break;

        case CSRD: 
            ecode = sys_uart_write_read_data(winfo, ustr,SCPI_INPUT_BUFFER_SIZE); // write data, expect answer
            if (ecode != NOCERR) { 
                fprintf(stdout, "Uart Error with string: %s\r\n", winfo);
            } else {
                fprintf(stdout, "Uart transmit data: %s\r\n", &winfo);
                fprintf(stdout, "Uart Received data: %s\r\n", &ustr); 
            }
            SCPI_ResultText(context,ustr); // return string with or without error
            retv = false;   //  value returned
            break;

        case SPWF: 
            fprintf(stdout, "SPI set Baudrate to %d\r\n", val);
            sys_spi_set_baudrate(val);  
            retv = false;   // no value returned
            break;

        case SPRF: 
            val = sys_spi_get_baudrate();
            fprintf(stdout, "SPI readback Baudrate, speed= %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            retv = true;   // no value returned
            break;

        case SPWCS: 
            ecode = sys_spi_set_chipselect(val); 
            if (ecode == 0) {
                fprintf(stdout, "SPI set Chipselect to %d\r\n", val);
            } else { 
                fprintf(stdout, "Unable to set SPI chipselect to gpio:  %d\r\n", val);
            }
            retv = false;   // no value returned
            break;

        case SPRCS: 
            val = sys_spi_get_chipselect();
            fprintf(stdout, "SPI readback chipselect gpio= %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            retv = true;   // no value returned
            break;


        case SPWDB: 
            ecode = sys_spi_set_databits(val); 
            if (ecode == 0) {
                fprintf(stdout, "SPI set Chipselect to %d\r\n", val);
            } else { 
                fprintf(stdout, "Unable to set SPI chipselect to gpio:  %d\r\n", val);
            }
            retv = false;   // no value returned
            break;

        case SPRDB: 
            val = sys_spi_get_databits();
            fprintf(stdout, "SPI readback chipselect gpio= %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            retv = true;   // no value returned
            break;

        case SPWM: 
            fprintf(stdout, "SPI set Mode to %d\r\n", val);
            sys_spi_set_mode(val);  
            retv = false;   // no value returned
            break;

        case SPRM: 
            val = sys_spi_get_mode();
            fprintf(stdout, "SPI Mode is set to = %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            retv = true;   // no value returned
            break;

        case SPWT: // Write data to SPI only, the answer is discarded
            fprintf(stdout, "SPI transmit data: %s\r\n", &winfo);
            //sys_uart_write_data(winfo); // write data, do not expect answer
            retv = false;   //  value returned
            break;
        
    }


       // raise error if is the case
    switch (ecode) {
        case NOERR: { break;}
        case OW_NB_ONEWIRE:  { answer = NB_ONEWIRE; break;}
        case OW_NO_ONEWIRE:  { answer = NO_ONEWIRES; break;}
        case OW_STR_NOT_IDENTICAL:  { answer = STR_ONEWIRE; break;}
        case OW_READ_WRITE_FAIL:   { answer = WR_ONEWIRE; break;}
        case OW_WRITE_FAIL:   { answer= WRITE_ONEWIRE; break;}
        case OW_READ_FAIL:   { answer= READ_ONEWIRES; break;}
        case OW_NO_VALIDID:   { answer= HEX_VALIDID; break;}
        case UART_PROT_NUM_NOTVALID: { answer= UART_NUMBER_ERROR; break;}
        case UART_PROT_LETTER_NOVALID: { answer= UART_LETTER_ERROR; break;}
        case UART_RX_TIMEOUT_MS : { answer= UART_RX_ERROR; break;}
        case UART_LASTCHAR_TIMEOUT_MS: { answer= UART_LASTCHAR_ERROR; break;}
        case UART_BUFFER_FULL: { answer= UART_RXBUFFER_ERROR; break;}
        case SPI_MODE_NUM_NOTVALID: { answer= SPI_MODE_ERROR; break;}
        case SPI_CS_NUM_ERROR: { answer= SPI_CS_ERROR; break;}
    }    

   
    if (ecode != NOERR) {
        SCPI_ErrorPush(context, answer);  // push errors 
        return SCPI_RES_ERR;
    } else {

        return SCPI_RES_OK;
    }
    free(sdata);
}

// Low level command
static scpi_result_t Callback_sync_com_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    scpi_number_t paramCom;

   // uint16_t answer;  // will contains the answer returned by command
    uint8_t tag,ecode;
    uint32_t val=0;
    int32_t readlen[1];
    uint64_t lval;
   // size_t lgt,eid;
   // bool retv = false;
   // bool bval;
   // const char *dpr;

   // char* sdata = NULL;
   volatile uint8_t wdata[SCPI_INPUT_BUFFER_SIZE];   // array to content the data to write
   volatile uint8_t rdata[SCPI_INPUT_BUFFER_SIZE];    // array to content the data read from function
    int idx = 0;  // array pointer index

    const char *data;  // Pointer to received data
    size_t length = 0;     // Length of received data
                                                                
    fprintf(stdout, "\nOn synchrounous communication execute \r\n");
   // winfo[0] = '\0';

    ecode =NOERR;
    tag = SCPI_CmdTag(context);   //extract tag from the command

    fprintf(stdout, "Tag = %d \r\n", tag);

    if ( tag == SPWT ) { 

        SCPI_CommandNumbers(context, readlen, 1,0);
        fprintf(stdout, "Read length: %d \r\n", readlen[0]);


        //Loop to extract all data from the parameters
        while(SCPI_Parameter(context, &param1, FALSE)){
            if (SCPI_ParamIsNumber(&param1, TRUE)) {
                // Convert parameter to unsigned int. Result is in value.
                SCPI_ParamToUInt64(context, &param1, &lval);
                fprintf(stdout, "Value to write: 0x%x \r\n", lval);
                int plen = param1.len / 2;  // Calculate number of byte based on string length
                // loop to save each byte on the write array
                for (int i = 0; i < plen; i++) {
                    wdata[idx++] = (lval >> (8 * (plen - 1 - i))) & 0xff;
                }
            }
            const char *dpr;
            if (param1.type  == SCPI_TOKEN_ARBITRARY_BLOCK_PROGRAM_DATA) {
                 // Call SCPI_ParamArbitraryBlock function
                scpi_bool_t result = SCPI_ParamArbitraryBlock(context, &dpr, &length, true);
                if (result == true) {
                    // Convert char array to byte array
                    for (int j = 0; j < length; j++) {
                        printf("0x%x, %d :", dpr[j],(unsigned char)dpr[j]);
                        wdata[idx++] = (unsigned char)dpr[j];
                    }
                    
                } else {
                    printf("Error: Failed to receive block of data.\n");
                }
            }
        
        }



            res = SCPI_Parameter(context, &param1, FALSE);


                        // Is parameter a number without suffix?
            if (SCPI_ParamIsNumber(&param1, TRUE)) {
                // Convert parameter to unsigned int. Result is in value.
                 SCPI_ParamToUInt32(context, &param1, &val);
                 // change number to size_t
            }


            if (param1.type  == SCPI_TOKEN_ARBITRARY_BLOCK_PROGRAM_DATA) {
                printf("Parameter are ARB block DATA");
            }
            // Call SCPI_ParamArbitraryBlock function
            scpi_bool_t result = SCPI_ParamArbitraryBlock(context, &data, &length, true);

            if (result == true) {
                printf("Received block of data with length %zu bytes.\n", length);
                printf("First few bytes of received data: ");
                for (size_t i = 0; i < length && i < 10; i++) {
                    printf("0x%x, %02X :", data[i],(unsigned char)data[i]);
                }
                printf("\n");
                SCPI_ResultArbitraryBlockHeader(context,length);
                printf("\nData: ");
                SCPI_ResultArbitraryBlockData(context, data, length);
                printf("\n");
                // Process the received data...
            } else {
                printf("Error: Failed to receive block of data.\n");
            }
          
    }



}


    // The SCPI commands we support and the callbacks they use.
scpi_command_t scpi_commands[] = {
	
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},  // Clear Status
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},  // Event Status enable
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,}, // query Event Status
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,}, // event status register query
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,}, // Instrument Identification
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},  // Set operation complete bit 
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,}, // wait for operation to complete
    { .pattern = "*RST", .callback = SCPI_CoreRst,}, // Reset instrument
    { .pattern = "*SRE", .callback = SCPI_CoreSre,}, // Service request enable
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,}, // Service request query
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,}, // read status byte
    { .pattern = "*TST?", .callback = SCPI_CallbackTstQ,}, // self-test
    { .pattern = "*WAI", .callback = SCPI_CoreWai,}, // wait for all pending operation to complete

     /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    { .pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    { .pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    { .pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

    { .pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    { .pattern = "STATus:QUEStionable:CONDition?", .callback = SCPI_StatusQuestionableConditionQ,},
    { .pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,},
    { .pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,},

    {.pattern = "STATus:OPERation[:EVENt]?", .callback = SCPI_StatusOperationEventQ, },
    {.pattern = "STATus:OPERation:CONDition?", .callback = SCPI_StatusOperationConditionQ, },
    {.pattern = "STATus:OPERation:ENABle", .callback = SCPI_StatusOperationEnable, },
    {.pattern = "STATus:OPERation:ENABle?", .callback = SCPI_StatusOperationEnableQ, },
    { .pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

    /* Added to communicate with Interconnect IO board */
    {.pattern = "ROUTe:CLOSE", .callback = Callback_Relay_scpi,RCLOSE},
    {.pattern = "ROUTe:CLOSE[:EXCLusive]", .callback = Callback_Relay_scpi,RCLEX},
    {.pattern = "ROUTe:OPEN", .callback = Callback_Relay_scpi,ROPEN},
    {.pattern = "ROUTe:OPEN:ALL", .callback = Callback_Relay_all_scpi,ROPALL},
    {.pattern = "ROUTe:CHANnel:STATe?", .callback = Callback_Relay_scpi,RSTATE},
    {.pattern = "ROUTe:BANK:STATe?", .callback = Callback_Relay_all_scpi,BSTATE},
    {.pattern = "ROUTe:REV:STATe?", .callback = Callback_Relay_all_scpi,SESTATE},
    {.pattern = "ROUTe:CLOSE:Rev", .callback = Callback_Relay_all_scpi,SECLOSE},
    {.pattern = "ROUTe:OPEN:Rev", .callback = Callback_Relay_all_scpi,SEOPEN},
    {.pattern = "ROUTe:CLOSE:PWR", .callback = Callback_Relay_all_scpi,PWCLOSE},
    {.pattern = "ROUTe:OPEN:PWR", .callback = Callback_Relay_all_scpi,PWOPEN},
    {.pattern = "ROUTe:STATE:PWR?", .callback = Callback_Relay_all_scpi,PWSTATE},
    {.pattern = "ROUTe:CLOSE:OC", .callback = Callback_Relay_all_scpi,OCCLOSE},
    {.pattern = "ROUTe:OPEN:OC", .callback = Callback_Relay_all_scpi,OCOPEN},
    {.pattern = "ROUTe:STATE:OC?", .callback = Callback_Relay_all_scpi,OCSTATE},

    {.pattern = "DIGital:DIRection:PORT#", .callback = Callback_Digital_scpi,SDIR},
    {.pattern = "DIGital:DIRection:PORT#:BIT#", .callback = Callback_Digital_scpi,SBDIR},
    {.pattern = "DIGital:Out:PORT#", .callback = Callback_Digital_scpi,SOUT},
    {.pattern = "DIGital:Out:PORT#:BIT#", .callback = Callback_Digital_scpi,SBOUT},
    {.pattern = "DIGital:In:PORT#?", .callback = Callback_Digital_scpi,RIN},
    {.pattern = "DIGital:In:PORT#:BIT#?", .callback = Callback_Digital_scpi,RBIN},
    {.pattern = "DIGital:DIRection:PORT#?", .callback = Callback_Digital_scpi,RDIR},
    {.pattern = "DIGital:DIRection:PORT#:BIT#?", .callback = Callback_Digital_scpi,RBDIR},

    {.pattern = "GPIO:DIRection:DEVice#:GP#", .callback = Callback_gpio_scpi,GPSDIR},
    {.pattern = "GPIO:DIRection:DEVice#:GP#?", .callback = Callback_gpio_scpi,GPRDIR},
    {.pattern = "GPIO:Out:DEVice#:GP#", .callback = Callback_gpio_scpi,GPOUT},
    {.pattern = "GPIO:In:DEVice#:GP#?", .callback = Callback_gpio_scpi,GPIN},
    {.pattern = "GPIO:SETPad:DEVice#:GP#", .callback = Callback_gpio_scpi,GPSPAD},
    {.pattern = "GPIO:GETPad:DEVice#:GP#?", .callback = Callback_gpio_scpi,GPGPAD},

    {.pattern = "SYSTem:BEEPer", .callback = Callback_system_scpi,SBEEP},
    {.pattern = "SYSTem:DEVice:VERSion?", .callback = Callback_system_scpi,SVER},
    {.pattern = "SYSTem:LED:ERRor", .callback = Callback_system_scpi,SLERR},
    {.pattern = "SYSTem:OUTput", .callback = Callback_system_scpi,SOE},
    {.pattern = "SYSTem:SLAves", .callback = Callback_system_scpi,SRUN},
    {.pattern = "SYSTem:LED:ERRor?", .callback = Callback_system_scpi,GLERR},
    {.pattern = "SYSTem:OUTput?", .callback = Callback_system_scpi,GOE},
    {.pattern = "SYSTem:SLAves?", .callback = Callback_system_scpi,GRUN},
    {.pattern = "SYSTEM:SLAves:STAtus?",.callback = Callback_system_scpi,GSTA},

    {.pattern = "ANAlog:DAC:Volt", .callback = Callback_analog_scpi,SDAC},
    {.pattern = "ANAlog:DAC:Save", .callback = Callback_analog_scpi,WDAC},
    {.pattern = "ANAlog:ADC0:Volt?", .callback = Callback_analog_scpi,RADC0},
    {.pattern = "ANAlog:ADC1:Volt?", .callback = Callback_analog_scpi,RADC1},
    {.pattern = "ANAlog:ADC:Vsys?", .callback = Callback_analog_scpi,RADC3},
    {.pattern = "ANAlog:ADC:Temp?", .callback = Callback_analog_scpi,RADC4},
    {.pattern = "ANAlog:PWR:Volt?", .callback = Callback_analog_scpi,RPV},
    {.pattern = "ANAlog:PWR:Shunt?", .callback = Callback_analog_scpi,RPS},
    {.pattern = "ANAlog:PWR:Ima?", .callback = Callback_analog_scpi,RPI},
    {.pattern = "ANAlog:PWR:Pmw?", .callback = Callback_analog_scpi,RPP},
    {.pattern = "ANAlog:PWR:Cal", .callback = Callback_analog_scpi,CPI},

    {.pattern = "CFG:Write:Eeprom:STRing", .callback = Callback_eeprom_scpi,WEEP},
    {.pattern = "CFG:Read:Eeprom:STRing?", .callback = Callback_eeprom_scpi,REEP},
    {.pattern = "CFG:Write:Eeprom:Default", .callback = Callback_eeprom_scpi,WDEF},
    {.pattern = "CFG:Read:Eeprom:Full?", .callback = Callback_eeprom_scpi,RFUL},
    
    {.pattern = "COM:OWire:Write", .callback = Callback_com_scpi,W1W},
    {.pattern = "COM:OWire:Read?", .callback = Callback_com_scpi,R1W},
    {.pattern = "COM:OWire:Check?", .callback = Callback_com_scpi,C1W},

    {.pattern = "COM:INITialize:ENAble", .callback = Callback_com_scpi,CIE},
    {.pattern = "COM:INITialize:DISable", .callback = Callback_com_scpi,CID},
    {.pattern = "COM:INITialize:STATus?", .callback = Callback_com_scpi,CRI},

    {.pattern = "COM:SERIAL:Write", .callback = Callback_com_scpi,CSWD},
    {.pattern = "COM:SERIAL:Read?", .callback = Callback_com_scpi,CSRD},
    {.pattern = "COM:SERIAL:Baudrate", .callback = Callback_com_scpi,CSWB},
    {.pattern = "COM:SERIAL:Baudrate?", .callback = Callback_com_scpi,CSRB},
    {.pattern = "COM:SERIAL:Protocol", .callback = Callback_com_scpi,CSWP},
    {.pattern = "COM:SERIAL:Protocol?", .callback = Callback_com_scpi,CSRP},
    {.pattern = "COM:SERIAL:Handshake", .callback = Callback_com_scpi,CSWH},
    {.pattern = "COM:SERIAL:Handshake?", .callback = Callback_com_scpi,CSRH},
    {.pattern = "COM:SERIAL:Timeout", .callback = Callback_com_scpi,CSWT},
    {.pattern = "COM:SERIAL:Timeout?", .callback = Callback_com_scpi,CSRT},

    {.pattern = "COM:SPI:WRIte:REAd:LENgth#", .callback = Callback_sync_com_scpi,SPWT},
    {.pattern = "COM:SPI:WRIte[:Byte]", .callback = Callback_com_scpi,SPWB},
    {.pattern = "COM:SPI:WRIte[:Word]", .callback = Callback_com_scpi,SPWW},
    {.pattern = "COM:SPI:WRIte:REAd?", .callback = Callback_com_scpi,SPWRT},
    {.pattern = "COM:SPI:WRIte:REAd[:Byte]?", .callback = Callback_com_scpi,SPWRB},
    {.pattern = "COM:SPI:WRIte:REAd[:Word]?", .callback = Callback_com_scpi,SPWRW},
    {.pattern = "COM:SPI:REAd?", .callback = Callback_com_scpi,SPRT},
    {.pattern = "COM:SPI:REAd[:Byte]?", .callback = Callback_com_scpi,SPRB},
    {.pattern = "COM:SPI:REAd[:Word]?", .callback = Callback_com_scpi,SPRW},

   // {.pattern = "COM:SPI:READ?", .callback = Callback_com_scpi,SPRB},
   // {.pattern = "COM:SPI:READ?", .callback = Callback_com_scpi,SPRB},
   // {.pattern = "COM:SPI:WRITEW", .callback = Callback_com_scpi,SPWW},
   // {.pattern = "COM:SPI:READW?", .callback = Callback_com_scpi,SPRW},
    {.pattern = "COM:SPI:Baudrate", .callback = Callback_com_scpi,SPWF},
    {.pattern = "COM:SPI:Baudrate?", .callback = Callback_com_scpi,SPRF},
    {.pattern = "COM:SPI:CS", .callback = Callback_com_scpi,SPWCS},
    {.pattern = "COM:SPI:CS?", .callback = Callback_com_scpi,SPRCS},
    {.pattern = "COM:SPI:Mode", .callback = Callback_com_scpi,SPWM},
    {.pattern = "COM:SPI:Mode?", .callback = Callback_com_scpi,SPRM},




	SCPI_CMD_LIST_END
};

void init_scpi(){
   
    // Initialize the SCPI library.
	SCPI_Init(
		&scpi_context, scpi_commands, &scpi_interface, scpi_units_def,
		SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
		scpi_input_buffer, SCPI_INPUT_BUFFER_SIZE,
		scpi_error_queue, SCPI_ERROR_QUEUE_SIZE
	);
}