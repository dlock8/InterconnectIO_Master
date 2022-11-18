#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"


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
	return fwrite(data, 1, len, stdout);
}

// execute reset of all Pico
scpi_result_t SCPI_Reset(scpi_t * context) {
    (void) context;
    // to be added
    fprintf(stdout, "**Reset\r\n");
    return SCPI_RES_OK;
}

static scpi_result_t SCPI_Flush(scpi_t * context) {
    (void) context;
    //fprintf(stdout, "SCPI Flush\r\n");
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {
    (void) context;
    /* BEEP */
    fprintf(stdout, "BEEP\r\n");
    fprintf(stdout, "**ERROR: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
    return 0;
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
    uint16_t Valtag, Vcomp, *px;
    char cdat,fres;
    volatile scpi_result_t  flag;
    uint16_t array[MAXROW * MAXCOL]; /* array which holds values in order (2D) */
    uint16_t answer[MAXROW * MAXCOL];
    size_t i = 0;
    char str[80];


    fprintf(stdout,"tag test\r\n");
  
    Valtag = SCPI_CmdTag(context);   //extract tag from the command

    fprintf(stdout,"tagvalue: %d\r\n", Valtag);

    flag = Relay_Chanlst(context, array);  // extract list of relay
    if (flag == SCPI_RES_ERR) {
        SCPI_ErrorPush(context, SCPI_RELAYS_LIST_ERROR);
        return SCPI_RES_ERR;
    }

    
    fres =  relay_execute(array,Valtag,answer); // Perform action requested
    if (!fres) {
        fprintf(stdout,"Relay error: %d\r\n", answer);
        SCPI_ErrorPush(context, answer[0]);
        return false;
    }

    if (Valtag == RSTATE) { // if returned value is expected
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

    SCPI_ResultText(context, "termine avec succes");

    return SCPI_RES_OK;

}

// List of special string not already define on SCPI parser
const scpi_choice_def_t scpi_special_all_numbers_def[] = {
    {/* name */ "ALL", /* type */ SCPI_BANK_ALL},
    {/* name */ "BANK1", /* type */ SCPI_BANK1},
    {/* name */ "BANK2", /* type */ SCPI_BANK2},
    {/* name */ "BANK3", /* type */ SCPI_BANK3},
    {/* name */ "BANK4", /* type */ SCPI_BANK4},
    SCPI_CHOICE_LIST_END,
};



// Open  particular Relay bank or all relay
static scpi_result_t Callback_Relay_all_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_number_t paramRelay;
    uint16_t array[5], se[5];
    uint16_t answer[MAXROW * MAXCOL];
    uint8_t Valtag;
    size_t i = 0;
    char str[80];

    Valtag = SCPI_CmdTag(context);   //extract tag from the command

    while(SCPI_ParamNumber(context, scpi_special_all_numbers_def, &paramRelay, FALSE)){
        printf("on switch \r\n");
        if (paramRelay.special) {
            switch (paramRelay.content.tag) {
                case SCPI_BANK1: 
                    array[i] = 10;    //  Add Bank1 on array list to open
                    se[i] = SE_BK1;
                    i++;  
                    break;
                case SCPI_BANK2:        //  Add Bank2 on array list to open
                    array[i] = 20;
                    se[i] = SE_BK2;
                    i++;  
                    break;
                case SCPI_BANK3: 
                    array[i] = 30;
                    se[i] = SE_BK3;
                    i++;  
                    break;
                case SCPI_BANK4: 
                    array[i] = 40;
                    se[i] = SE_BK4;
                    i++;  
                    break;

                case SCPI_BANK_ALL: 
                    array[i] = 10;     // Add all banks to the list
                    array[i+1] = 20;
                    array[i+2] = 30;
                    array[i+3] = 40;
                    i +=4;  
                    break;
                
                default: 
                    SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
                    return SCPI_RES_ERR;
            }
        }    

    }
    array[i] = 0;   // flag for end of array
    

    if (i > 0) {
       res =  relay_execute(array,Valtag,answer); // Perform action requested
    } else {
        if (SCPI_ParamErrorOccurred(context)) {
            SCPI_ErrorPush(context, SCPI_ERROR_MISSING_PARAMETER);
        } else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        }
        return SCPI_RES_ERR;
    }


    i=0;  // reset loop counter
    if (Valtag == BSTATE || Valtag == SESTATE) { // if returned value is expected
        do {  // loop on array until list of value is completed
           sprintf(str, " 0x%x,", answer[i]);
           SCPI_ResultUInt8(context,answer[i]); // return SCPI value 
            i++;
        } while (array[i] > 0);
        //sprintf(str, "\r\n"); // print on debug port
        printf(str);
        printf ("\r\n"); 

    }
   
return SCPI_RES_OK;
}


// Digital Scpi command execution
static scpi_result_t Callback_Digital_scpi(scpi_t *context) {
    scpi_bool_t res;
    scpi_parameter_t param1;
    uint16_t answer[80];
 //   uint16_t array[5], se[5];
 //   uint16_t answer[MAXROW * MAXCOL];
 volatile   uint8_t Valtag;
//    size_t i = 0;
 //   char str[80];
uint32_t value = 0;

res = SCPI_Parameter(context, &param1, TRUE);

if (res) {
    // Is parameter a number without suffix?
    if (SCPI_ParamIsNumber(&param1, FALSE)) {
        // Convert parameter to unsigned int. Result is in value.
        SCPI_ParamToUInt32(context, &param1, &value);
    }
}

    int32_t numbers[2];
    SCPI_CommandNumbers(context, numbers, 2, 12);

    fprintf(stdout, "TEST numbers %d %d\r\n", numbers[0], numbers[1]);

    Valtag = SCPI_CmdTag(context);   //extract tag from the command

    res = digital_execute(Valtag, numbers[0], numbers[1], value,answer); 

return SCPI_RES_OK;
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

    {.pattern = "DIGital:DIRection:PORT#", .callback = Callback_Digital_scpi,SDIR},
    {.pattern = "DIGital:DIRection:PORT#:BIT#", .callback = Callback_Digital_scpi,SBDIR},
    {.pattern = "DIGital:PADs:PORT#", .callback = Callback_Digital_scpi,SPAD},
    {.pattern = "DIGital:Out:PORT#", .callback = Callback_Digital_scpi,SOUT},
    {.pattern = "DIGital:In:PORT#?", .callback = Callback_Digital_scpi,RIN},
    {.pattern = "DIGital:DIRection:PORT#?", .callback = Callback_Digital_scpi,RDIR},
    {.pattern = "DIGital:PADs:PORT#?", .callback = Callback_Digital_scpi,RPAD},


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