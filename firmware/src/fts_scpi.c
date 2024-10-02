/**
 * @file    fts_scpi.c
 * @author  Daniel Lockhead
 * @date    2024
 * 
 * @brief   Software code used to interpret the SCPI command reveived from external
 *   
 * 
 * @details This module contains all software functions related to SCPI command. THe SCPI command
 *          are defined on this module and the callback are executed on this module.
 *
 *
 * @copyright Copyright (c) 2021, Jan Breuer. Simpled BSD license.
 *            Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */
 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico_lib2/src/dev/dev_ds2431/dev_ds2431.h"
#include "include/scpi_user_config.h"
#include "include/test.h"
#include "include/scpi_uart.h"
#include "include/scpi_spi.h"
#include "include/scpi_i2c.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/master.h"
#include "hardware/resets.h"
#include "include/functadv.h"

#include "userconfig.h"         // contains Major and Minor version

/**
 * @brief SCPI instance for managing SCPI command processing.
 */
scpi_t scpi_context;  ///< Holds the SCPI context instance for command execution.

/**
 * @brief Input buffer for reading SCPI commands.
 * 
 * This buffer is used to store incoming SCPI commands read from
 * the external source before they are processed.
 */
char scpi_input_buffer[SCPI_INPUT_BUFFER_SIZE];  ///< Buffer to hold SCPI command strings.

/**
 * @brief Error queue required by the SCPI library.
 * 
 * This array is used to hold error messages and statuses encountered
 * during SCPI command processing. The size is determined based on
 * the library documentation.
 */
scpi_error_t scpi_error_queue[SCPI_ERROR_QUEUE_SIZE];  ///< Queue for SCPI error reporting.


// Function used by module test.c to validate the selftest
/**
 * @brief Function used by module test.c to capture the SCPI answer from the SCPI command. Used 
 *        to perform the selftest of the board
 * 
 * @param data String of the SCPI answer
 * @param len  Length of the answer
 * @return size_t length 
 */
static size_t output_buffer_write(const char * data, size_t len) {
    memcpy(out_buffer + out_buffer_pos, data, len);
    out_buffer_pos += len;
    out_buffer[out_buffer_pos] = '\0';
    return len;
}

/**
 * @brief The function write the SCPI answer to the main serial port. 
 *        The answer is also saved on global variable to be analyzed on odule test.c
 * 
 * @param context SCPI instance pointer
 * @param data    Pointer to the string of data
 * @param len     Length of the string
 * @return size_t True if string written with success
 */
size_t SCPI_write(scpi_t *context, const char *data, size_t len) {
 
    char outbuf[SCPI_INPUT_BUFFER_SIZE];  // Global variable output buffer used to test code

    strncpy(outbuf,data,SCPI_INPUT_BUFFER_SIZE); // save data string to temporay buffer
    outbuf[len] = '\0';      // add string termination
    uart_puts(UART_ID, outbuf); // send answer to serial port
    output_buffer_write(data, len);  // USED BY TEST to capture output of the command

	return fwrite(data, 1, len, stdout); // send answer to usb port for help debug
}

/**
 * @brief AIRCR Register for triggering a system reset.
 * 
 * This macro allows access to the Application Interrupt and Reset Control Register 
 * (AIRCR) used for performing a system reset on the master Pico.
 */
#define AIRCR_Register (*((volatile uint32_t*)(PPB_BASE + 0x0ED0C)))


/**
 * @brief function to execute the reset of the interconnect IO board following the
 *        SCPI command. Only master Pico is resetted because the Pico slaves are 
 *        resetted during boot-up
 * 
 * @param context  SCPI instance
 * @return scpi_result_t  True if reset performed with success
 */
scpi_result_t SCPI_Reset(scpi_t * context) {
    (void) context;
    fprintf(stdout, "*Reset execute begin\r\n");

    // perform a system reset using  Application Interrupt and Reset Control Register (AIRCR) 

    AIRCR_Register = 0x5FA0004;
    return SCPI_RES_OK;
}

/**
 * @brief SCPI Flush is for clear the input buffer
 * 
 * @param context  SCPI instance
 * @return scpi_result_t  True if reset performed with success
 */
 
static scpi_result_t SCPI_Flush(scpi_t * context) {
    (void) context;
    //fprintf(stdout, "SCPI Flush\r\n");
    return SCPI_RES_OK;
}


/**
 * @brief Send a beep tone following a SCPI command or internal function
 *
 */

void SCPI_Beep() {
    gpio_put(GPIO_BEEP, 1); // Tuen ON beeper
    sleep_ms(BEEP_TIME);    // wait time
    gpio_put(GPIO_BEEP, 0); // Turn OFF beeper
}


/**
 * @brief Send a burst of beep code to alert for boot error
 * 
 * @param nbeep Number of beep on the burst
 */

void ErrorBeep(uint8_t nbeep){
    for (uint8_t j = 0; j < 3 ; j++) {
        for( uint8_t i=0; i <nbeep; i++){
            SCPI_Beep();
            sleep_ms(250);    // wait time between beep
        }
    sleep_ms(500);    // wait time between burst of beep
    }
}

/**
 * @brief function called by SCPI when  error is found. The function will perform
 *        a short beep and turn on the error led. If error =0, the error led will be turn off.
 * 
 * @param context  SCPI instance
 * @param err      Error number
 * @return int     True if function performed with success
 */

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

/**
 * @brief Global variable to reset the Service Request Register (SRQ).
 * 
 * This variable holds the current value of the Service Request Register (SRQ).
 * It is initialized to `0` and used in the SCPI (Standard Commands for Programmable Instruments) context.
 */
scpi_reg_val_t srq_val = 0;


/**
 * @brief function to set the Service Request Register (SRQ) to a value
 * 
 * @param context SCPI instance
 * @param ctrl    The identifer of the SCPI control register
 * @param val     The value to save on control register
 * @return scpi_result_t True if function performed with success
 */

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


/**
 * @brief Additional callbacks to be used by the SCPI library.
 * 
 */
scpi_interface_t scpi_interface = {
	.write = SCPI_write,
	.error = SCPI_Error,
    .control = SCPI_Control,
    .flush = SCPI_Flush, 
	.reset = SCPI_Reset,
};


/**
 * @brief List of special string not already defined on SCPI parser
 * 
 */
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
    {/* name */ "SERIAL", /* type */ SCPI_SERIAL},
    {/* name */ "I2C", /* type */ SCPI_I2C},

    SCPI_CHOICE_LIST_END,
};


 /**
  * @brief Reimplement IEEE488.2 *TST?
  * 
  * @param context SCPI instance
  * @return scpi_result_t Result should be 0 if everything is ok
  */
static scpi_result_t SCPI_CallbackTstQ(scpi_t * context) {
    fprintf(stdout,"Board internal Selftest execute \r\n");
    IOBoard_Selftest(); // function to exedcute the internal selftest
    return SCPI_RES_OK;
}

/**
 * @brief   Set Bit on SCPI register Questionnable and Operation
 *          Send Bust of Beep to signal to operator the problem
 *          Send Error to Error Queue
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
 * @brief Parses a list of channel numbers and stores them in a 1-dimensional array.
 * 
 * This function is based on the SCPI parser for parsing lists of numbers, specifically
 * channel numbers greater than 0. It has been modified to support only 1-dimensional 
 * arrays, as opposed to the original 2-dimensional support.
 * 
 * @param context SCPI context structure used for parsing SCPI commands.
 * @param array Pointer to an array where the parsed channel numbers will be stored.
 * 
 * @return scpi_result_t Result of the SCPI parsing operation (e.g., success or failure).
 * 
 * @note Compare this implementation to SCPI-99 Vol 1, Ch. 8.3.2 for details on channel parsing.
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

/**
 * @brief Callback function to manage the relay actuation following 
 *        the reception of a list of relays to close, open or read
 * 
 * @param context  SCPI instance
 * @return scpi_result_t True if no error during execution
 */
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

    return SCPI_RES_OK;

}


// Open  particular Relay bank or all relay
/**
 * @brief Callback function to execute action on relay Bank following the SCPI command
 * 
 * @param context SCPI instance
 * @return scpi_result_t True if no error during execution
 */

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


/**
 * @brief Callback function to interpret the digital command received from the SCPI
 * 
 * @param context SCPI instance
 * @return scpi_result_t True if no error during execution
 */

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


/**
 * @brief Callback function to interpret the GPIO command received from the SCPI port
 * 
 * @param context SCPI instance
 */

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
/**
 * @brief Callback function to interpret the system command received from the SCPI port
 * 
 * @param context SCPI instance
 */

static scpi_result_t Callback_system_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    scpi_number_t paramRun;
    uint16_t ans[8];  // will contains the answer returned by command
    char pv[30]; 
    uint8_t tag = 0;
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

        case STBR:
            fprintf (stdout, "Run Internal Selftest # %d\r\n",value);
            context->buffer.position = 0;
            internal_test_sequence(ee.cfg.testboard_num,value);
            SCPI_Reset(context); // reset hardware after selftest
            break;

        default: 
            break;
    }
    return SCPI_RES_OK;
}


/**
 * @brief Callback function to interpret the analog command received from the SCPI port
 * 
 * @param context SCPI instance
 */

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


/**
 * @brief Callback function to interpret the eeprom command received from the SCPI port
 * 
 * @param context SCPI instance
 */

static scpi_result_t Callback_eeprom_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    uint16_t answer;  // will contains the answer returned by command
    uint8_t tag;
    char* split;
    uint8_t status = NOERR;
    char sfull[64], pstr[96];
    uint32_t  value;    // value of parameter
    char varname[32], svalue[32];
    size_t i;
    char *tnptr;
    long cnum;
    char mode = '\0';  //default value


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
    // The last field (true or false is to validate if the parameter is a number or not 

    struct ParamInfo members[] = {
        {CHECK, offsetof(cfg,check), sizeof(((cfg *)0)->check),FALSE},
        {PARTNUMBER, offsetof(cfg,partnumber), sizeof(((cfg *)0)->partnumber),FALSE},
        {SERIALNUMBER, offsetof(cfg,serialnumber),sizeof(((cfg *)0)->serialnumber),FALSE},
        {MOD_OPTION, offsetof(cfg,mod_option),sizeof(((cfg *)0)->mod_option),FALSE},
        {COM_SER_SPEED, offsetof(cfg,com_ser_speed),sizeof(((cfg *)0)->com_ser_speed),TRUE},
        {PSLAVE_RUN, offsetof(cfg,slave_force_run),sizeof(((cfg *)0)->slave_force_run),TRUE},
        {TESTBOARD_NUM, offsetof(cfg,testboard_num), sizeof(((cfg *)0)->testboard_num),FALSE},
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
            mode = 'r';   // read mode
            break;
    }

    // Run only for read write parameter on eeprom
    // etract varname and svalue
    if (tag == WEEP || tag == REEP) {
        res = SCPI_Parameter(context, &param1, true);  // Read first parameter
        if (res == false) { return SCPI_RES_ERR;}  // if no parameter read, raise error and exit

        // Split the string to distinguishes between varnames and the svalue
        split = strtok (param1.ptr," ',");
        strcpy(varname,split);
        strupr(varname);
        fprintf(stdout,"EEprom varname = %s\n",varname);

        if (tag  == WEEP){ // if tag  = Write command
            split = strtok (NULL," ',\r\n");  // Extract second parameter from the command
            strcpy(svalue,split);   // copy second parameter to svalue
            strupr(svalue);         // change lowercase to upper case
            if (svalue[0] == '\0') {   // if svalue not present, raise error
                fprintf(stdout,"Error, no svalue to write on eeprom \n");
                status = EMP;  // Set error flag
            } else { fprintf(stdout,"EEprom svalue = %s\n",svalue); }
        }

    }

    if (mode == 'w' || mode == 'r') {

        bool found = false; // set flag default value

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
                int check =stringtonumber( svalue,sizeof(svalue), &number);
                if (check != 0) {   
                    status = ENDE;      // raise error if not a number
                }
            }

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


/**
 * @brief Callback function to interpret the communcation command received from the SCPI port
 * 
 * @param context SCPI instance
 */

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


    if ( tag == C1W || tag == R1W || tag == CSWB || tag == CSWT ) {
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
                                scpi_spi_enable();
                                fprintf(stdout, "Enable SPI communication\r\n");
                            }
                            if (tag == CID) {
                                scpi_spi_disable();
                                fprintf(stdout, "Disable SPI communication\r\n");
                            }
                            if (tag == CRI) {
                                bval = scpi_spi_status();
                                fprintf(stdout, "Read status SPI communication: %d\r\n",bval);
                                SCPI_ResultBool(context,bval);
                            }
                        break;

                        case SCPI_SERIAL: 
                            if (tag == CIE) {
                                scpi_uart_enable();
                                fprintf(stdout, "Enable SERIAL communication\r\n");
                            }
                            if (tag == CID) {
                                scpi_uart_disable();
                                fprintf(stdout, "Disable SERIAL communication\r\n");
                            }
                            if (tag == CRI) {
                                bval = scpi_uart_status();
                                fprintf(stdout, "Read status SERIAL communication: %d\r\n",bval);
                                SCPI_ResultBool(context,bval);
                            }
                        break;

                        case SCPI_I2C: 
                            if (tag == CIE) {
                                scpi_i2c_enable();
                                fprintf(stdout, "Enable I2C communication\r\n");
                            }
                            if (tag == CID) {
                                scpi_i2c_disable();
                                fprintf(stdout, "Disable I2C communication\r\n");
                            }
                            if (tag == CRI) {
                                bval = scpi_i2c_status();
                                fprintf(stdout, "Read status I2C communication: %d\r\n",bval);
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
            break;

        case R1W: 
            ecode = onewire_read_info(&sdata,ADDR_INFO,NB_INFO,eid );  
            SCPI_ResultText(context,sdata);
            break;

        case W1W: 
            ecode = onewire_write_info(winfo,ADDR_INFO);  
            SCPI_ResultText(context,sdata);
            break;

        case CSWB: 
            fprintf(stdout, "Serial set Baudrate to %d\r\n", val);
            scpi_uart_set_baudrate(val);  
            break;

        case CSRB: 
            val = scpi_uart_get_baudrate();
            fprintf(stdout, "Serial readback actual Baudrate, speed= %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            break;

        case CSWT: 
            fprintf(stdout, "Serial set Timeout_ms to %d\r\n", val);
            scpi_uart_set_timeout(val);  
            break;

        case CSRT: 
            val = scpi_uart_get_timeout();
            fprintf(stdout, "Serial readback Timeout_ms: %d\r\n", val); 
            SCPI_ResultInt32(context,val); 
            break;

        case CSWH: 
            fprintf(stdout, "Serial set RTS-CTS Handshake to %d\r\n", val);
            scpi_uart_set_handshake(val);  
            break;

        case CSRH: 
            bval = scpi_uart_get_handshake();
            fprintf(stdout, "Serial readback RTS-CTS Handshake: %d\r\n", bval); 
            SCPI_ResultBool(context,bval);
            break;
        
        case CSWP: 
            ecode = scpi_uart_set_protocol(winfo);
            if (ecode != NOERR) { 
                fprintf(stdout, "Serial protocol error with value: %s\r\n", &winfo);
            } else {
                fprintf(stdout, "Serial set protocol to: %s\r\n", &winfo); 
            }
            break;

        case CSRP: 
            dpr = scpi_uart_get_protocol();
            fprintf(stdout, "Serial readback protocol: %s\r\n", dpr); 
            SCPI_ResultText(context,dpr);
            break;

        case CSWD: // Write data to uart only, the answer is discarded
            fprintf(stdout, "Serial transmit data: %s\r\n", &winfo);
            ecode = scpi_uart_write_data(winfo); // write data, do not expect answer
            break;

        case CSRD: 
            ecode = scpi_uart_write_read_data(winfo, ustr,SCPI_INPUT_BUFFER_SIZE); // write data, expect answer
            if (ecode != NOCERR) { 
                fprintf(stdout, "Serial Error with string: %s\r\n", winfo);
            } else {
                fprintf(stdout, "Serial transmit data: %s\r\n", &winfo);
                fprintf(stdout, "Serial Received data: %s\r\n", &ustr); 
                SCPI_ResultText(context,ustr); // return string with or without error
            }
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
        case UART_NOT_ENABLED: { answer= UART_ENABLE_ERROR; break;}
    }    

   
    if (ecode != NOERR) {
        SCPI_ErrorPush(context, answer);  // push errors 
        return SCPI_RES_ERR;
    } else {

        return SCPI_RES_OK;
    }
    free(sdata);
}


/**
 * @brief Callback function to interpret the synchronized communication command received from the SCPI port
 * 
 * @param context SCPI instance
 */

static scpi_result_t Callback_sync_com_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    scpi_number_t paramCom;

   // uint16_t answer;  // will contains the answer returned by command
    uint8_t tag;
    volatile int8_t ecode;
    uint16_t maxdata;
    uint32_t val=0;
    uint32_t readlen[1];
    uint64_t lval;
    bool retv = false;


    maxdata = SCPI_INPUT_BUFFER_SIZE;
    uint8_t wdata[maxdata];   // array to content the data to write
    uint8_t rdata[maxdata];    // array to content the data read from function
    int8_t idx = 0;  // array pointer index

    const char *dpr;  // Pointer to received data
    size_t lenblk = 0;     // Length of received data
    readlen[0] = 0;  // initialize
                                                                
    fprintf(stdout, "\nOn synchronous communication execute \r\n");
   
    ecode =NOERR;
    tag = SCPI_CmdTag(context);   //extract tag from the command

    //fprintf(stdout, "Tag = %d \r\n", tag);

    //!< Tag to read single data 
    if ( tag == SPWCS || tag == SPWDB || tag == SPWF || tag == SPWM || tag == ICWDB || tag == ICWF || tag == ICWA) {
        res = SCPI_Parameter(context, &param1, true);  // Read first parameter
        if (res) {
            // Is parameter a number without suffix?
            if (SCPI_ParamIsNumber(&param1, TRUE)) {
                // Convert parameter to unsigned int. Result is in value.
                 SCPI_ParamToUInt32(context, &param1, &val);
            }
        }
    }

    //!< Tag to read all data, using loop. The data could be in many forms
    if ( tag == SPWD || tag == SPRD || tag == ICWD || tag == ICRD) { 

        SCPI_CommandNumbers(context, readlen, 1,0); // extract number of bytes to read
        fprintf(stdout, "On Command, Nb of byte/word  to Read: %d \r\n", readlen[0]);

            //Loop to extract all data from the parameters
            while(SCPI_Parameter(context, &param1, FALSE)){     // extract first parameter

                // Section to check if parameters data is in format of Arbitrary block
                if (param1.type == SCPI_TOKEN_ARBITRARY_BLOCK_PROGRAM_DATA) {
                    dpr = param1.ptr;
                    lenblk = param1.len;

                    if (lenblk % 2 == 0) {
                        // Convert char array send by Arb to byte array
                        for (int j = 0; j < lenblk/2; j++) {
                            uint8_t byte = 0;
                            char sval[3] = {dpr[j * 2], dpr[j * 2 + 1], '\0'};
                            fprintf(stdout,"Data string # %d : %s\n",idx,sval);
                            wdata[idx++] = (unsigned char)strtol(sval, NULL, 16);
                        }          
                    } else {
                        printf("Error: Arbitrary block data length is odd, expect even number, Length: %d.\n", lenblk);
                        ecode =  ARB_ODD_ERR;
                    }
                }

                // Section to check if parameters data is a number
                if (SCPI_ParamIsNumber(&param1, TRUE)) {        // if parameter is number
                    SCPI_ParamToUInt64(context, &param1, &lval);

                    // determine number of bytes to be saved in array based on number type
                    size_t plen=0;
                    uint64_t val =lval;
                    if (param1.type == SCPI_TOKEN_HEXNUM) { plen = param1.len/2;} // hex numbers 
                    if (param1.type == SCPI_TOKEN_OCTNUM) { plen = param1.len/2;} // octal numbers 
                    if (param1.type == SCPI_TOKEN_BINNUM) { plen = param1.len/8;} // binary numbers 
                    if (param1.type == SCPI_TOKEN_DECIMAL_NUMERIC_PROGRAM_DATA) { 
                        while (val > 0) { // loop to find number of bytes
                            plen++;
                            val >>= 8; // Shift right by 8 bits (1 byte)
                        }
                    }
                    if (lval >= 0 && plen == 0 ) { plen = 1;} // particular case when plen = 0
                    // loop to save each byte on the write array, higher value first
                    for (int i = 0; i < plen; i++) {
                        wdata[idx] = (lval >> (8 * (plen - 1 - i))) & 0xff;
                        fprintf(stdout, "Byte from string: 0x%02x \r\n", wdata[idx]);
                        idx++;
                    }
                }
            }
              
    } // end of tag
 
    bool wordsize;  // flag to indicate of data is on word size
    bool regf;
    
    if (ecode != 0) { tag =0;} // if error found, do not execute instruction

    switch (tag) {

        case SPWD:
            fprintf(stdout, "SPI write data only, nbw to write: %d\r\n", idx);
            ecode = scpi_spi_wri_read_data(wdata,idx,rdata,readlen[0],&wordsize);
            break;

        case SPRD:
            if (idx == 0) {
                fprintf(stdout, "SPI read data only, Nb byte/word: %d\r\n", readlen[0]);
            } else{
                fprintf(stdout, "SPI write & read data, nb write %d, nb byte/word read: %d\r\n",idx, readlen[0]);
            }
            ecode = scpi_spi_wri_read_data(wdata,idx,rdata,readlen[0],&wordsize);
            break;
        
        case SPWF: 
            fprintf(stdout, "SPI set Baudrate to %d\r\n", val);
            scpi_spi_set_baudrate(val);  
            break;

        case SPRF: 
            val = scpi_spi_get_baudrate();
            fprintf(stdout, "SPI readback Baudrate, speed= %d\r\n", val); 
            retv = true;
            break;

        case SPWCS: 
            ecode = scpi_spi_set_chipselect(val); 
            if (ecode == 0) {
                fprintf(stdout, "SPI set Chipselect to %d\r\n", val);
            } else { 
                fprintf(stdout, "Unable to set SPI chipselect to gpio:  %d\r\n", val);
            }
            break;

        case SPRCS: 
            val = scpi_spi_get_chipselect();
            fprintf(stdout, "SPI readback chipselect gpio= %d\r\n", val);
            retv = true;
            break;

        case SPWDB: 
            ecode = scpi_spi_set_databits(val); 
            if (ecode == 0) {
                fprintf(stdout, "SPI set databits to %d\r\n", val);
            } else { 
                fprintf(stdout, "Unable to set SPI databits to:  %d\r\n", val);
            }
            break;

        case SPRDB: 
            val = scpi_spi_get_databits();
            fprintf(stdout, "SPI readback databits=  %d\r\n", val); 
            retv = true;
            break;

        case SPWM: 
            fprintf(stdout, "SPI set Mode to %d\r\n", val);
            ecode = scpi_spi_set_mode(val);  
            break;

        case SPRM: 
            val = scpi_spi_get_mode();
            fprintf(stdout, "SPI Mode is set to = %d\r\n", val);
            retv = true; 
            break;

        case ICWD:
            fprintf(stdout, "I2C write data only, nbw to write: %d\r\n", idx);
            ecode = scpi_i2c_wri_read_data(wdata,idx,rdata,readlen[0],&wordsize);
            break;

        case ICRD:
            if (idx == 0) {
                fprintf(stdout, "I2C read data only, Nb byte/word: %d\r\n", readlen[0]);
            } else{
                fprintf(stdout, "I2C write & read data, nb write %d, nb byte/word read: %d\r\n",idx, readlen[0]);
            }
            ecode = scpi_i2c_wri_read_data(wdata,idx,rdata,readlen[0],&wordsize);
            break;
        
        case ICWF: 
            fprintf(stdout, "I2C set Baudrate to %d\r\n", val);
            scpi_i2c_set_baudrate(val);  
            break;

        case ICRF: 
            val = scpi_i2c_get_baudrate();
            fprintf(stdout, "I2C readback Baudrate, speed= %d\r\n", val); 
            retv = true;
            break;

        case ICWA: 
            fprintf(stdout, "I2C set Device Address to 0x%x\r\n", val);
            scpi_i2c_set_address(val);  
            break;

        case ICRA: 
            val = scpi_i2c_get_address();
            fprintf(stdout, "I2C readback Device Address, addr= 0x%x\r\n", val); 
            retv = true;
            break;

         case ICWDB: 
            ecode = scpi_i2c_set_databits(val); 
            if (ecode == 0) {
                fprintf(stdout, "I2C set databits to %d\r\n", val);
            } else { 
                fprintf(stdout, "Unable to set I2C databits to:  %d\r\n", val);
            }
            break;

        case ICRDB: 
            val = scpi_i2c_get_databits();
            fprintf(stdout, "I2C readback databits=  %d\r\n", val); 
            retv = true;
            break;
    }


    if (retv == true) { //!< if single value need to be returned
            SCPI_ResultInt32(context,val); //!< return value 
    } 

    if ((tag == ICRD && ecode == NOERR) ||(tag == SPRD && ecode == NOERR)  )  {
        if (!wordsize) { // if bytes size need to be returned
            SCPI_ResultArrayUInt8(context, rdata,readlen[0],SCPI_FORMAT_ASCII);
        } else {
            uint16_t *wrdata = NULL;    // create pointer to read word data
            wrdata = (uint16_t*)(uintptr_t)rdata; // adjust pointer to word data
            SCPI_ResultArrayUInt16(context, wrdata,readlen[0],SCPI_FORMAT_ASCII);
        } 
    }

    // raise error if is the case
    switch (ecode) {
        case NOERR: { break;}
        case SPI_MODE_NUM_NOTVALID: { val= SPI_MODE_ERROR; break;}
        case SPI_CS_NUM_ERROR: { val= SPI_CS_ERROR; break;}
        case MALLOC_FAILURE: { val= MEMORY_ALLOCATION_ERROR; break;}
        case SPI_TIMEOUT: { val= SPI_TIMEOUT_ERROR; break;}
        case SPI_NOT_ENABLED: { val= SPI_ENABLE_ERROR; break;}
        case ARB_ODD_ERR:  { val= ARB_WORD_FORMAT_ERROR; break;}
        case I2C_MALLOC_FAILURE: { val= MEMORY_ALLOCATION_ERROR; break;}
        case I2C_GENERIC_ERR: { val= I2C_GENERIC_ERROR; break;}
        case I2C_TIMEOUT_ERR: { val= I2C_TIMEOUT_ERROR; break;}
        case I2C_ADDRESS_NACK: { val= I2C_ADDRESS_NACK_ERROR; break;}
        case I2C_DATA_NACK: { val= I2C_DATA_NACK_ERROR; break;}
        case I2C_BUS_ERR: { val= I2C_BUS_ERROR; break;}
        case I2C_NOT_ENABLED: { val= I2C_ENABLE_ERROR; break;}
        default: {val = ecode; break;}
    }    

    //!< if error found, send result to SCPI error queue
    if (ecode != NOERR) {
        SCPI_ErrorPush(context, val);  // push errors 
        return SCPI_RES_ERR;    //!< raise error 
    } else {
        return SCPI_RES_OK;
    }
   
} // end of sub


 /**
  * @brief  The SCPI commands supported by the pico master and the callbacks they use.
  * 
  */

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
    { .pattern = "*TST?", .callback = SCPI_CallbackTstQ,}, // selftest
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
    {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

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
    {.pattern = "SYSTem:SLAves:STAtus?",.callback = Callback_system_scpi,GSTA},
    {.pattern = "SYSTem:TESTboard",.callback = Callback_system_scpi,STBR},

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

    {.pattern = "COM:SPI:WRIte", .callback = Callback_sync_com_scpi,SPWD},
    {.pattern = "COM:SPI:REAd:LENgth#?", .callback = Callback_sync_com_scpi,SPRD},
    {.pattern = "COM:SPI:Baudrate", .callback = Callback_sync_com_scpi,SPWF},
    {.pattern = "COM:SPI:Baudrate?", .callback = Callback_sync_com_scpi,SPRF},
    {.pattern = "COM:SPI:Databits", .callback = Callback_sync_com_scpi,SPWDB},
    {.pattern = "COM:SPI:Databits?", .callback = Callback_sync_com_scpi,SPRDB},
    {.pattern = "COM:SPI:CS", .callback = Callback_sync_com_scpi,SPWCS},
    {.pattern = "COM:SPI:CS?", .callback = Callback_sync_com_scpi,SPRCS},
    {.pattern = "COM:SPI:Mode", .callback = Callback_sync_com_scpi,SPWM},
    {.pattern = "COM:SPI:Mode?", .callback = Callback_sync_com_scpi,SPRM},

    {.pattern = "COM:I2C:WRIte", .callback = Callback_sync_com_scpi,ICWD},
    {.pattern = "COM:I2C:REAd:LENgth#?", .callback = Callback_sync_com_scpi,ICRD},
    {.pattern = "COM:I2C:ADDRess", .callback = Callback_sync_com_scpi,ICWA},
    {.pattern = "COM:I2C:ADDRess?", .callback = Callback_sync_com_scpi,ICRA},
    {.pattern = "COM:I2C:Baudrate", .callback = Callback_sync_com_scpi,ICWF},
    {.pattern = "COM:I2C:Baudrate?", .callback = Callback_sync_com_scpi,ICRF},
    {.pattern = "COM:I2C:Databits", .callback = Callback_sync_com_scpi,ICWDB},
    {.pattern = "COM:I2C:Databits?", .callback = Callback_sync_com_scpi,ICRDB},

	SCPI_CMD_LIST_END
};

/**
 * @brief initialization of the SCPI library
 * 
 */

void init_scpi(){
   
    // Initialize the SCPI library.
	SCPI_Init(
		&scpi_context, scpi_commands, &scpi_interface, scpi_units_def,
		SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
		scpi_input_buffer, SCPI_INPUT_BUFFER_SIZE,
		scpi_error_queue, SCPI_ERROR_QUEUE_SIZE
	);
}