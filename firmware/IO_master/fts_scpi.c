#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/master.h"
#include "hardware/resets.h"
#include "include/functadv.h"





//#include "scpi/scpi.h"
//#include "scpi/expression.h"

//#include "scpi/error.h"
//#include "scpi/parser.h"
//#include "include/scpi_user_config.h"

// This will hold the SCPI "instance".
scpi_t scpi_context;


// Input buffer for reading SCPI commands.

char scpi_input_buffer[SCPI_INPUT_BUFFER_SIZE];

// Error queue required by the library. Size taken from the docs.

scpi_error_t scpi_error_queue[SCPI_ERROR_QUEUE_SIZE];




// The function called by the SCPI library when it wants to send data.
size_t SCPI_write(scpi_t *context, const char *data, size_t len) {
 
    // added code to add null termination to the string data. uart_puts do not support parameter lenght.
    char t[128];          // temporay buffer 
    strncpy(t,data,128); // save data string to temporay buffer
    t[len] = '\0';      // add string termination
    uart_puts(UART_ID, t); // send answer to serial port

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

int SCPI_Error(scpi_t * context, int_fast16_t err) {
    // (void) context;
    SCPI_Beep();  // Beep for signal error
    fprintf(stdout, "**ERROR: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
    return SCPI_RES_OK;
}




scpi_reg_val_t srq_val = 0;

static scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    (void) context;

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


    fprintf(stdout,"tag test\r\n");
  
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

    SCPI_CHOICE_LIST_END,
};



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
    if (tag == BSTATE || tag == SESTATE || tag == PWSTATE) { // if returned value is expected
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
                sprintf(pv,"%2d.%2d, %2d.%2d, %2d.%2d, %2d.%2d\n",ans[0],ans[1],ans[2],ans[3],ans[4],ans[5],ans[6],ans[7]);
                fprintf(stdout,pv); // print string version for the 4 devices
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
            break;

        case GRUN:  // Read Run enable value
            value = gpio_get(GPIO_RUN);
            fprintf (stdout, "Read Slave Run_EN on gpio %d ,value: %d \r\n",GPIO_RUN, value);
            SCPI_ResultUInt8(context,value); // return SCPI value 
            break;

        case SOE:  // System Output enable
            fprintf (stdout, "Set Output Enable gpio %d to: %d \r\n",GPIO_OE, value);
            gpio_put(GPIO_OE, value);
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
    uint16_t answer[1];  // will contains the answer returned by command
    uint8_t tag;
    char* split;
    uint8_t status = NOERR;
    char mode;
    char sfull[64], pstr[96];
    uint32_t  value;    // value of parameter
    char varname[32], svalue[32];
    size_t i;


        // Helper structure to store member information
    struct ParamInfo {
        const char* name;
        size_t offset;
        size_t size;
    };


    // Define an array of MemberInfo structs with member names, offsets, and sizes
    // Each parameter to Read/Write on EEPROM need to be added on this array (except Check parameter)
    struct ParamInfo members[] = {
        {CHECK, offsetof(cfg,check), sizeof(((cfg *)0)->check)},
        {PARTNUMBER, offsetof(cfg,partnumber), sizeof(((cfg *)0)->partnumber)},
        {SERIALNUMBER, offsetof(cfg,serialnumber),sizeof(((cfg *)0)->serialnumber)},
        {MOD_OPTION, offsetof(cfg,mod_option),sizeof(((cfg *)0)->mod_option)},
        {COM_SER_SPEED, offsetof(cfg,com_ser_speed),sizeof(((cfg *)0)->com_ser_speed)},
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

    // Run only for read wwite parameter on eeprom
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

            if (found) { // if varname found
                status = cfg_eeprom_rw(mode,members[i].offset,members[i].size,svalue,strlen(svalue));    // write or read data on eeprom
                 if (status == NOERR) {
                    strncpy(&ee.data[members[i].offset], svalue, strlen(svalue));  // save parameter on eeprom structure
                    if (mode == 'r') {
                        SCPI_ResultCharacters(context,&ee.data[members[i].offset],members[i].size); // return value
                    }
                }
            }
        }
    }
      // raise error if is the case
    switch (status) {
        case NOERR:
            break;

        case EOOR:
            answer[0] = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;  
            break;
        case EIVN:
            answer[0] = SCPI_ERROR_ILLEGAL_VARIABLE_NAME; 
            break;
        case ECE:
             answer[0] = SCPI_ERROR_CHARACTER_DATA_ERROR;
             break;
        
        case EDE:
             answer[0] = SCPI_ERROR_EXECUTION_ERROR;
             break;

        case ERE:
             answer[0] = SCPI_ERROR_MASS_STORAGE_ERROR;
             break; 

        case EMP:
            answer[0] = SCPI_ERROR_MISSING_PARAMETER;  
            break;
    }    

    if (status != NOERR) {
        SCPI_ErrorPush(context, answer[0]);  // push errors 
        return SCPI_RES_ERR;
    } else {

        return SCPI_RES_OK;
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
    { .pattern = "*TST?", .callback = SCPI_CoreTstQ,}, // self-test
    { .pattern = "*WAI", .callback = SCPI_CoreWai,}, // wait for all pending operation to complete

     /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    { .pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    { .pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    { .pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

    /* Added to communicate with Interconnect IO board */
    {.pattern = "ROUTe:CLOSE", .callback = Callback_Relay_scpi,RCLOSE},
    {.pattern = "ROUTe:CLOSE[:EXCLusive]", .callback = Callback_Relay_scpi,RCLEX},
    {.pattern = "ROUTe:OPEN", .callback = Callback_Relay_scpi,ROPEN},
    {.pattern = "ROUTe:OPEN:ALL", .callback = Callback_Relay_all_scpi,ROPALL},
    {.pattern = "ROUTe:CHANnel:STATe?", .callback = Callback_Relay_scpi,RSTATE},
    {.pattern = "ROUTe:BANK:STATe?", .callback = Callback_Relay_all_scpi,BSTATE},
    {.pattern = "ROUTe:SE:STATe?", .callback = Callback_Relay_all_scpi,SESTATE},
    {.pattern = "ROUTe:CLOSE:Se", .callback = Callback_Relay_all_scpi,SECLOSE},
    {.pattern = "ROUTe:OPEN:Se", .callback = Callback_Relay_all_scpi,SEOPEN},
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

    {.pattern = "ANAlog:DAC:Volt", .callback = Callback_analog_scpi,SDAC},
    {.pattern = "ANAlog:DAC:Save", .callback = Callback_analog_scpi,WDAC},
    {.pattern = "ANAlog:ADC0:Volt?", .callback = Callback_analog_scpi,RADC0},
    {.pattern = "ANAlog:ADC1:Volt?", .callback = Callback_analog_scpi,RADC1},
    {.pattern = "ANAlog:ADC:Vsys?", .callback = Callback_analog_scpi,RADC3},
    {.pattern = "ANAlog:ADC:Temp?", .callback = Callback_analog_scpi,RADC4},
    {.pattern = "ANAlog:PWR:Volt?", .callback = Callback_analog_scpi,RPV},
    {.pattern = "ANAlog:PWR:Shunt?", .callback = Callback_analog_scpi,RPS},
    {.pattern = "ANAlog:PWR:Ima?", .callback = Callback_analog_scpi,RPI},
    {.pattern = "ANAlog:PWR:Pwm?", .callback = Callback_analog_scpi,RPP},
    {.pattern = "ANAlog:PWR:Cal", .callback = Callback_analog_scpi,CPI},

    {.pattern = "CFG:Write:Eeprom:STRing", .callback = Callback_eeprom_scpi,WEEP},
    {.pattern = "CFG:Read:Eeprom:STRing?", .callback = Callback_eeprom_scpi,REEP},
    {.pattern = "CFG:Write:Eeprom:Default", .callback = Callback_eeprom_scpi,WDEF},
    {.pattern = "CFG:Read:Eeprom:Full?", .callback = Callback_eeprom_scpi,RFUL},
    /*
    {.pattern = "CFG:Write:SERIALNUMBER", .callback = Callback_eeprom_scpi,WEESN},
    {.pattern = "CFG:Read:SERIALNUMBER?", .callback = Callback_eeprom_scpi,REESN},
    {.pattern = "CFG:Write:MOD_OPTION", .callback = Callback_eeprom_scpi,WEEMO},
    {.pattern = "CFG:Read:MOD_OPTION?", .callback = Callback_eeprom_scpi,REEMO},
    {.pattern = "CFG:Write:DEF_CSER", .callback = Callback_eeprom_scpi,WEEDS},
    {.pattern = "CFG:Read:DEF_CSER?", .callback = Callback_eeprom_scpi,REEDS},
    */


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