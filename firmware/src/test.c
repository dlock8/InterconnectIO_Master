/**
 * @file    test.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Software code used to validate the software and hardware developed on this
            project.
 *
 * @details This file contains all software functions used to validate the combination of
 *          the selftest board connected to the interconnect IO board. The SCPI command
 *          has been validated, and the interconnect IO board hardware has been validated
 *          using a step-by-step debugger.
 *
 *          The internal selftest is a complete example of how to test the interconnect IO
 *          board using the selftest board. External instruments connected to the interconnect
 *          IO board are validated using true instruments or a loopback connector.
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "include/scpi_user_config.h"
#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "pico_lib2/src/dev/dev_ds2431/dev_ds2431.h"
#include "pico_lib2/src/sys/include/sys_adc.h"
#include "hardware/watchdog.h"  // Include watchdog functions
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "include/functadv.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/master.h"
#include "include/scpi_spi.h"
#include "include/scpi_i2c.h"
#include "include/test.h"


/**
 * @brief Output buffer for storing data to be sent.
 *
 * This buffer holds the data that will be sent via the output channel (e.g., serial or communication port).
 * It has a fixed size of 1024 bytes.
 */
char out_buffer[1024];

/**
 * @brief Current position in the output buffer.
 *
 * Tracks the position in the `out_buffer` where the next byte of data will be written.
 */
size_t out_buffer_pos;

/**
 * @brief Macro to simulate SCPI command input via the master serial port.
 *
 * This macro simulates receiving an SCPI command string and processes it using
 * the SCPI context. It calls the `SCPI_Input` function to handle the command input.
 *
 * @param cmd The SCPI command string to be processed.
 *
 * @note This macro assigns the result of `SCPI_Input` to a variable named `result`.
 * Ensure that the variable `result` is defined before using this macro.
 */
#define TEST_SCPI_INPUT(cmd) result = SCPI_Input(&scpi_context, cmd, strlen(cmd))

/*! @brief - Function to clear the output_buffer
 *   Output_buffer is used by selftest program to capture the response
 *   form a SCPi command executed by the SCPI parser
 *
 *   @return
 */
static void output_buffer_clear(void)
{
  out_buffer[0] = '\0';
  out_buffer_pos = 0;
}

/*! @brief - ChatGPt function to remove CR and LF from a string
 *
 *   @param str String who contains the CR and LF to remove
 *   @return  String without CR and LF
 */
void removeCRLF(char* str)
{
  if (str == NULL)
  {
    // Handle NULL pointer gracefully
    return;
  }

  int len = strlen(str);
  int readIndex = 0;
  int writeIndex = 0;

  // Iterate through the string
  while (readIndex < len)
  {
    // If the character is not a carriage return or newline, keep it
    if (str[readIndex] != '\r' && str[readIndex] != '\n')
    {
      str[writeIndex] = str[readIndex];
      writeIndex++;
    }

    readIndex++;
  }

  // Null-terminate the modified string
  str[writeIndex] = '\0';
}

// Following code is to manage the circular buffer used to capture the failure
// message during test sequence

/**
 * @brief Structure who contains circular buffer messages and pointer
 *
 */
typedef struct
{
  char messages[BUFFER_SIZE][MESSAGE_LENGTH]; /**< Array of messages */
  int start;                                  /**< Start index of the buffer */
  int end;                                    /**< End index of the buffer */
  int full;                                   /**< Flag indicating if the buffer is full */
} CircularBuffer;

/**
 * @brief Initialisation of the index for circular buffer
 *
 * @param buffer Structure who contains the index and message
 */
void init_buffer(CircularBuffer* buffer)
{
  buffer->start = 0; /**< Initialize Start index of the buffer */
  buffer->end = 0;   /**< Initialize End index of the buffer */
  buffer->full = 0;  /**< Initialize Flag indicating if the buffer is full */
}

/**
 * @brief Function to add_message on circular buffer
 *
 * @param buffer    pointer to structure who contains messages
 * @param message   pointer to message to save on structure
 */
void add_message(CircularBuffer* buffer, const char* message)
{
  strncpy(buffer->messages[buffer->end], message, MESSAGE_LENGTH);
  buffer->end = (buffer->end + 1) % BUFFER_SIZE;
  if (buffer->full)
  {
    buffer->start = (buffer->start + 1) % BUFFER_SIZE;
  }
  if (buffer->end == buffer->start)
  {
    buffer->full = 1;
  }
}

/**
 * @brief Function to print message saved on circular buffer
 *
 * @param buffer pointer to structure who contains messages
 */
void print_messages(const CircularBuffer* buffer)
{
  int i = buffer->start;
  while (i != buffer->end || buffer->full)
  {
    fprintf(stdout, "%s\n", buffer->messages[i]);
    uart_puts(UART_ID, buffer->messages[i]);  // send message to serial port
    uart_puts(UART_ID, "\n");                 // Send newline
    i = (i + 1) % BUFFER_SIZE;
    if (i == buffer->end && !buffer->full) break;
  }
}

/*!
 * @struct TestResult
 * @brief  Structure to count the test result during selftest sequence
 *
 * This structure hold the counter to be able to display a resume at the end of selftest.
 */
struct TestResult
{
  int total; /**< Total of test performed. */
  int good;  /**< Total of PASS test performed. */
  int bad;   /**< Total of FAIL test performed. */
  int error; /**< Total of Error captured. */
};

/*! @brief - Send SCPI command and validate the value for PASS or FAIL.
 *            The analog value returned by command is validated to determine is between determined limit
 *
 *    @param title Test title to be used to report the purpose of the measure
 *    @param cmd   String command to send to the SCPI engine
 *    @param expect_value   Value expected to be returned by the measure
 *    @param unit     unit to use on printf
 *    @param lo_limit  Lower deviation from expect_value acceptable to PASS the test
 *    @param hi_limit  Higher deviation from expect_value acceptable to PASS the test
 *    @param counter Structure who content the result of test. Be used by final report
 *    @param buffer  Pointer to circular buffer, who contains the error
 *    @return
 */
void test_cmd_result(const char* title, const char* cmd, float expect_value, const char* unit, float lo_limit, float hi_limit,
                     struct TestResult* counter, CircularBuffer* buffer)
{
  float readv;
  float hl, ll;
  char message[MESSAGE_LENGTH];

  counter->total++;       // increment counter
  output_buffer_clear();  // clear result before capture output
  watchdog_update(); /** refresh watchdog */

  SCPI_Input(&scpi_context, cmd, strlen(cmd)); /** Send command to SCPI engine*/
  // transform string received from command to number
  do
  {
    if (sscanf((out_buffer), "%f", &(readv)) != 1)
    {
      fprintf(stderr, "\t ERROR converting buffer to float, rvalue: %s\n", out_buffer);
      (readv) = -99.99;
      counter->error++;
    }
  } while (0);

  // on ADC measure,multiplication of value by 2 is required due to voltage divider on selftest board
  if (strstr(cmd, "ADC0") != NULL || strstr(cmd, "ADC1") != NULL)
  {
    readv = readv * 2;
  }
  hl = expect_value + hi_limit;
  ll = expect_value - lo_limit;

  if (readv > hl || readv < ll)
  {
    snprintf(message, MESSAGE_LENGTH, "%s  ---> FAIL  VAL:%.2f %s, LL:%.2f, HL:%.2f ", title, readv, unit, ll, hl);
    fprintf(stdout, "%s\n", message);
    add_message(buffer, message);
    counter->bad++;
  }
  else
  {
    fprintf(stdout, "%s  ---> PASS  VAL:%.2f %s, LL:%.2f, HL:%.2f  \n", title, readv, unit, ll, hl);
    counter->good++;
  }
}

/*! @brief - Send SCPI command and validate the value for PASS or FAIL.
 *            The answer value returned by command is validated with the expected result
 *
 *    @param title Test title to be used to report the purpose of the measure
 *    @param cmd   String command to send to the SCPI engine
 *    @param expected_result   String to validate with the result returned by the command
 *    @param counter Structure who content the result of test. Be used by final report
 *    @param buffer  Pointer to circular buffer, who contains the error
 *    @return
 */
void test_cmd_out(const char* title, const char* cmd, char* expected_result, struct TestResult* counter, CircularBuffer* buffer)
{
  float readv;
  bool valid = true;
  char message[MESSAGE_LENGTH];

  counter->total++;                            // increment counter
  output_buffer_clear();                       // clear result before capture output
  SCPI_Input(&scpi_context, cmd, strlen(cmd)); /** Send command to SCPI engine*/
  removeCRLF(out_buffer);                      // remove \r\n from string before comparaison
  watchdog_update(); /** refresh watchdog */

  // validate if string are identical, raise flag if not identical
  do
  {
    if (strcmp((expected_result), (out_buffer)) != 0)
    {
      valid = false;
    }
  } while (0);

  // report result of the comparaison
  if (!valid)
  {
    snprintf(message, MESSAGE_LENGTH, "%s  ---> FAIL  Expected: %s, Read: %s", title, expected_result, out_buffer);
    fprintf(stdout, "%s\n", message);
    add_message(buffer, message);
    counter->bad++;
  }
  else
  {
    fprintf(stdout, "%s  ---> PASS  Read: %s\n", title, out_buffer);
    counter->good++;
  }
}

/**
 * @brief Tests if the given command string contains the expected substring.
 *
 * This function checks whether a given SCPI command string contains the specified
 * expected substring and updates the test results accordingly.
 *
 * @param title Description or title of the test case.
 * @param cmd The SCPI command string to be tested.
 * @param expected_substring The substring that is expected to be found within the command string.
 * @param counter A pointer to the test result structure to store the success or failure of the test.
 * @param buffer A pointer to a circular buffer used for handling the input/output during testing.
 *
 * @return true if the expected substring is found in the command string, false otherwise.
 */
void test_cmd_substring(const char* title, const char* cmd, char* expected_substring, struct TestResult* counter, CircularBuffer* buffer)
{
  float readv;
  bool valid = true;
  char message[MESSAGE_LENGTH];

  counter->total++;                            // increment counter
  output_buffer_clear();                       // clear result before capture output
  SCPI_Input(&scpi_context, cmd, strlen(cmd)); /** Send command to SCPI engine*/
  removeCRLF(out_buffer);                      // remove \r\n from string before comparaison
  watchdog_update();
  // validate if string are identical, raise flag if not identical
  if (strstr(out_buffer, expected_substring) == NULL)
  {
    valid = false;
  }

  // report result of the comparaison
  if (!valid)
  {
    snprintf(message, MESSAGE_LENGTH, "%s  ---> FAIL  Expected: %s, Read: %s", title, expected_substring, out_buffer);
    fprintf(stdout, "%s\n", message);
    add_message(buffer, message);
    counter->bad++;
  }
  else
  {
    fprintf(stdout, "%s  ---> PASS  Read: %s\n", title, out_buffer);
    counter->good++;
  }
}

/**
 * @brief  function to wait for read character from user. The loop refresh the watchdog timeout
 *
 * @return char character entry by the user
 */
char read_uart_char()
{
  bool lf = true;
  char in[3];

  while (lf)
  {
    if (uart_is_readable(UART_ID))
    {
      // read first character
      uart_read_blocking(UART_ID, &in[0], 1);  // read one character and save on array
      in[1] = '\n';
      in[2] = 0;               // complete string before send to serial
      uart_puts(UART_ID, in);  // echo string to terminal
      lf = false;
    }
    else
    {
      // Feed the watchdog while waiting for data
      watchdog_update();
    }
  }
  return in[0];
}

/**
 * @brief Main routine to perform internal test using selftest board.
 *        The routine has been used to validate hardware and software during development
 *        if run=0, the menu will be displayed to operator to select which internal test to run.
 *        IF run > 0, the internal test will run directly
 *
 * @param testboard_num  Partnumber of the selftest board to be validated using 1-wire data
 * @param run  Test number to run, if equal to 0, the test menu will be displayed
 */
void internal_test_sequence(const char* testboard_num, uint8_t run)
{
  int result;
  int rtn;
  char in;
  char strval[120];
  uint8_t tnb;
  bool lp = true;

  // Disable RX interrupt to take control of serial input port
  uart_set_irq_enables(UART_ID, false, false);

  // loop until exit key is pressed
  while (lp)
  {
    // if selftest menu is requested (run = 0)
    if (run == 0)
    {
      in = 0;  // initialize value
      sprintf(strval, "\n\n\tInternal Test Sequences\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "1- Selftest using only selftest board, no check of Onewire\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "2- Selftest run only if selftest board is installed, Onewire validation\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "3- Selftest using selftest board and loopback connector\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "4- Selftest of instruments in manual mode using selftest board\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "5- Test of SCPI command,selftest board is required\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "0- Exit test sequence\n");
      uart_puts(UART_ID, strval);  // Send string
      sprintf(strval, "\tEnter test number to execute and press enter: ");
      uart_puts(UART_ID, strval);  // Send string

      in = read_uart_char();
      tnb = in - '0';  // transform character read to number

      // loop to clear all character received in serial port
      int i = 0;
      sleep_ms(100);  // let time to receive all characters in serial port
      char dum[20];
      while (uart_is_readable(UART_ID))
      {
        uart_read_blocking(UART_ID, &dum[i], 1);  // read one character and save on array
        // uart_puts(UART_ID, in[i]); // echo character to serial port
        i++;
      }
    }
    else
    {              // if run > 0, execute test directly and exit
      tnb = run;   // assign test number to value of parameter run
      lp = false;  // exit while loop when test completed
    }

    // execute selftest program
    switch (tnb)
    {
      case 0:
        lp = false;  // exit while loop
        break;
      case 1:
        rtn = test_selftest(testboard_num, 1);
        break;
      case 2:
        rtn = test_selftest(testboard_num, 2);
        break;
      case 3:
        rtn = test_selftest(testboard_num, 3);
        break;
      case 4:
        test_inst_manual();
        break;
      case 5:
        test_command();
        break;

      default:
        lp = true;
    }

    if (rtn == 1)
    {
      sprintf(strval, "Selftest board not detected reading onewire\n");
      uart_puts(UART_ID, strval);  // Send string
    }
  }

  // Enable RX interrupt to take control of serial input
  uart_set_irq_enables(UART_ID, true, false);

  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF \r\n");  // turn OFF Error led
  TEST_SCPI_INPUT("SYST:OUT OFF\r\n"); /** Open Power Relay to remove power on Selftest board */

  sprintf(strval, "\nEnd of Internal Test Sequence\n");
  uart_puts(UART_ID, strval);  // Send string
}

/*! @brief - Run test sequence to validate hardware with selftest board
 *
 *   This function perform the selftest to validate the hardware using only internal instruments.
 *   The test sequence follow the Validation_Test.xls Excel list
 *
 *   Detailed report is send to stdout port (usb)
 *   Summary report is send to serial port
 *
 *    @return    return value of 1 is selftest board is not detected
 */
uint8_t test_selftest(const char* testboard_num, uint8_t run)
{
  int result; /**< Variable used by TEST_SCPI_INPUT */

  char strval[120]; /**< array who will contains parts of SCPI command */

  struct TestResult c_test = {0, 0, 0, 0};  // reset counter structure
  out_buffer_pos = 0;

  CircularBuffer buffer;
  init_buffer(&buffer);

  fprintf(stdout, "\tSelftest Hardware Test\n");   // send message to debug port
  sprintf(strval, "\nSELFTEST HARDWARE TEST \n");  // build string to return
  uart_puts(UART_ID, strval);                      // Send result

  /** Read 1-wire to detect if the selftest board is connected to interconnect IO*/
  char* strdata = NULL;

  // if testboard value >= 2, the selftest will run only if the 1-wire number match with
  // the testboard number.

  if (run >= 2)
  {
    result = onewire_read_info(&strdata, ADDR_INFO, NB_INFO, 1);
    fprintf(stdout, "\tSelftest 1-wire: %s\n", strdata);  // send message to debug port
                                                          // Use strstr to find the substring
    char* pos = strstr(strdata, testboard_num);
    if (pos != NULL)
    {
      printf("Testboard string found at position: %ld\n", *pos);
    }
    else
    {
      printf("Testboard string not found.\n");
      return 1;
    }
  }

  TEST_SCPI_INPUT("SYST:SLA OFF\r\n"); /** Disable slaves Pico to reset configuration*/
  sleep_ms(100);
  TEST_SCPI_INPUT("SYST:SLA ON\r\n"); /** Start slaves Pico after reset*/
  sleep_ms(500);
  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF\r\n"); /** Turn OFF red led */

  // initialisation of all resources used on selftest board to toggle relays
  // All digital IO are set as input after reset.
  // All IO need to be set as output and initialized to 0 before the start of the test
  //
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");   // set direction port 0 ouput
  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");   // set direction port 1 input
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");   // set port 0 to 00
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP8 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP9 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1  0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9  0 \r\n");
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP18 1 \r\n");   // set to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP19 0 \r\n");   // set to input
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG output
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP19  0 \r\n");  // CTRL Input
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");

  // Setup I2C configuration to be used to communicate with selftest Board

  TEST_SCPI_INPUT("COM:I2C:D 8 \r\n");       // Set I2C_master to Databits to 8
  TEST_SCPI_INPUT("COM:I2C:B 100000\r\n");   // set i2C_master Baudrate to 100k
  TEST_SCPI_INPUT("COM:I2C:ADDR #H20\r\n");  // set I2C Address of selftest board (0x20)
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");   // Enable Master I2C

  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF \r\n");  // turn OFF Error led

  TEST_SCPI_INPUT("SYST:OUT ON\r\n"); /** Close Power Relay et apply 5V to Selftest board */
  sleep_ms(250);                      /** Wait to let time to relay to close and power the selftest board */
  test_cmd_result("Test 1.1: 5VDC Check with ADC0", "ANA:ADC0:VOLT? \r\n", 5.0, "V", 0.3, 0.3, &c_test, &buffer);

  // Port 0 validation
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");  // set direction port 0 output
  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");  // set direction port 1 input
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H55 \r\n");
  test_cmd_out("Test 2.0: Port 0 = 0x55(85), Read Port 1 value ", "DIG:IN:PORT1?\r\n", "85", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #HAA \r\n");
  test_cmd_out("Test 2.1: Port 0 = 0xAA(170), Read Port 1 value ", "DIG:IN:PORT1?\r\n", "170", &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n");  // set direction port 1 output
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #H00 \r\n");  // set direction port 1 input
  TEST_SCPI_INPUT("DIG:OUT:PORT1 #H33 \r\n");
  test_cmd_out("Test 2.2: Port 1 = 0x33(51), Read Port 0 value ", "DIG:IN:PORT0?\r\n", "51", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT1 #HCC \r\n");
  test_cmd_out("Test 2.3: Port 1 = 0xCC(204), Read Port 0 value ", "DIG:IN:PORT0?\r\n", "204", &c_test, &buffer);

  // replace original value after test of digital port
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");  // set direction port 0 output
  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");  // set direction port 1 input
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // reset digital output

  // Test of handshake ctrl and flag
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // FLAG
  test_cmd_out("Test 2.4: Handshake Flag = 1, Read Ctrl bit ", "GPIO:IN:DEV1:GP19?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG
  test_cmd_out("Test 2.4: Handshake Flag = 0, Read Ctrl bit ", "GPIO:IN:DEV1:GP19?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:IN:DEV1:GP19?  \r\n");  // CTRL

  // Digital test 2.6 and 2.7 to verify each GPIO using Pico located on selftest board is TBD
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H20 \r\n");  // Close K16 (VM2)
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");   // Close OC1
  test_cmd_result("Test 3.0: Output Collector OC1 Drive 1 (Active),read ADC0", "ANA:ADC0:VOLT? \r\n", 0.2, "V", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");  // Open OC1
  test_cmd_result("Test 3.1: Output Collector OC1 Drive 0 (Inactive),read ADC0", "ANA:ADC0:VOLT? \r\n", 5, "V", 0.4, 1, &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H10 \r\n");  // Close K15 (VM3)
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");   // Close OC2
  test_cmd_result("Test 3.2 Output Collector OC2 Drive 1 (Active),read ADC0", "ANA:ADC0:VOLT? \r\n", 0.2, "V", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");  // Open OC2
  test_cmd_result("Test 3.3: Output Collector OC2 Drive 0 (Inactive),read ADC0", "ANA:ADC0:VOLT? \r\n", 5, "V", 0.4, 1, &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H90 \r\n");  // Close K15,K9 (VM4)
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3 \r\n");   // Close OC3
  test_cmd_result("Test 3.4 Output Collector OC3 Drive 1 (Active),read ADC0", "ANA:ADC0:VOLT? \r\n", 0.2, "V", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");  // Open OC3
  sleep_ms(1);
  test_cmd_result("Test 3.5: Output Collector OC3 Drive 0 (Inactive),read ADC0", "ANA:ADC0:VOLT? \r\n", 5, "V", 0.4, 0.8, &c_test, &buffer);

  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  1 \r\n");  // Close K13 (ADC1)
  test_cmd_result("Test 4.0: 5VDC Check with ADC1", "ANA:ADC1:VOLT? \r\n", 5.0, "V", 0.4, 0.3, &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H40 \r\n");  // Close K2
  TEST_SCPI_INPUT("ANA:DAC:VOLT 3 \r\n");      // Set ouput to 3v

  test_cmd_result("Test 4.1: Dac output @ 3Vdc, read ADC1", "ANA:ADC1:VOLT? \r\n", 3.0, "V", 0.4, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ANA:DAC:VOLT 0.25 \r\n");  // Set ouput to 0.25V

  test_cmd_result("Test 4.2: Dac output @ 0.25Vdc, read ADC1", "ANA:ADC1:VOLT? \r\n", 0.25, "V", 0.02, 0.08, &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n");  // Open K13 (ADC1)
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");    // Open K2

  sleep_ms(15);
  test_cmd_result("Test 5.0: PWR Module check Bus Voltage (5VDC), read Vdc:", "ANA:PWR:V? \r\n", 5, "V", 0.4, 0.3, &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  sleep_ms(15);
  test_cmd_result("Test 5.1: PWR Module check Shunt voltage , read mV:", "ANA:PWR:S? \r\n", 50, "mV", 7, 7, &c_test, &buffer);
  test_cmd_result("Test 5.2: PWR Module check current on 10 ohm(R2), read I(mA):", "ANA:PWR:I? \r\n", 500, "mA", 50, 50, &c_test, &buffer);

  // Perform Calibration of the PWR module INA219
  float readv;                                             // contains the current value read from last command
  sscanf((out_buffer), "%f", &(readv));                    // transform string in output_buffer to float number
  sprintf(strval, "ANAlog:PWR:Cal %.2f, 500\r\n", readv);  // build calibration string to be used as command
  TEST_SCPI_INPUT(strval);                                 // send command
  test_cmd_result("Test 5.3: PWR Module check current on 10 ohm(R2), read I(mA):", "ANA:PWR:I? \r\n", 500, "mA", 5, 5, &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4
  TEST_SCPI_INPUT("ANAlog:PWR:Cal 500,500\r\n");  // reset value to default value

  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H32 \r\n");     // Close K7,K15,K16 (VM5)
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");      // Close K11

  test_cmd_result("Test 6.0: Resistor load (10 ohms) check,read ADC0", "ANA:ADC0:VOLT? \r\n", 2.5, "V", 0.4, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");       // Open K11
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");     // Open K7,K15,K16 (VM5)

  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");  // Close K10
  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR2 \r\n");
  sleep_ms(15);
  test_cmd_result("Test 7.0: Low Power Relay NC1 check,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR2 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1 \r\n");
  sleep_ms(15);
  test_cmd_result("Test 7.1: Low Power Relay NO1 check,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
  sleep_ms(15);
  test_cmd_result("Test 7.2: Low Power Relay OPEN1 check,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");  // Open K10

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H01 \r\n");  // Close K8 (PS3)
  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR2 \r\n");
  sleep_ms(15);
  test_cmd_result("Test 7.3: Low Power Relay NC2 check,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR2 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1 \r\n");
  sleep_ms(15);
  test_cmd_result("Test 7.4: Low Power Relay NO2 check,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
  sleep_ms(15);
  test_cmd_result("Test 7.5: Low Power Relay OPEN2 check,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // Open K8 (PS3)

  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H02 \r\n");     // Close K7 (PS5)
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR HPR1 \r\n");
  sleep_ms(30);  // wait time to power relay actuate
  test_cmd_result("Test 8.0: High Power Relay CLOSE check,read I(mA)", "ANA:PWR:I? \r\n", 250, "mA", 15, 50, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:PWR HPR1 \r\n");
  sleep_ms(30);  // wait time to power relay open
  test_cmd_result("Test 8.1: High Power Relay OPEN check,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H01 \r\n");  // Close K8
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");   // Close K10 (PS6)
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR SSR1 \r\n");
  test_cmd_result("Test 9.0: Solid State Relay CLOSE check,read I(mA)", "ANA:PWR:I? \r\n", 250, "mA", 15, 50, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:PWR SSR1 \r\n");
  test_cmd_result("Test 9.1: Solid State Relay CLOSE check,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");       // Open K10 (PS6)
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4

  /** RELAY BANK TEST */
  int i, j, t;
  i = 0;  // count index
  j = 0;
  t = 10;

  /** Loop to test BK1-BK2 channel*/
  for (i = 0; i < 8; i++)
  {
    /** Test the High side of relay*/
    TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");            // Close K7,K8 (PS7)
    sprintf(strval, "ROUT:CLOSE (@10%d,20%d)\r\n", i, i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 CH%d-H Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@20%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 BK2_CH%d-H Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:CLOSE (@20%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 BK2_CH%d-H Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@10%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 BK1_CH%d-H Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@20%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);

    /** Test the LOW side of relay*/
    TEST_SCPI_INPUT("DIG:OUT:PORT0 #H0B \r\n");  // Close K7,K8,K14 (PS8)
    TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3 \r\n");   // Close K12 (PS6)

    sprintf(strval, "ROUT:CLOSE (@10%d,20%d)\r\n", i, i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 CH%d-L Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@20%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 BK2_CH%d-L Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:CLOSE (@20%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 BK2_CH%d-L Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@10%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK1-BK2 BK1_CH%d-L Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@20%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");  // OPen K12 (PS6)
  }                                            // end of loop for BK1-BK2 relay test

  // Check for the COM relay H Side
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H0B \r\n");    // Close K7,K8,K14 (PS7)
  TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n");  // Close BK1-Bk2 CH0 relays and SE relays
  test_cmd_result("Test 11.0: Relay BK1-BK2 COM-H Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@208)\r\n");  // Open SE relay
  test_cmd_result("Test 11.1: Relay BK1-BK2, BK2-COM-H Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n");  // Close BK1-Bk2 CH0 relays
  test_cmd_result("Test 11.2: Relay BK1-BK2 COM-H Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@108)\r\n");  // Open SE relay
  test_cmd_result("Test 11.3: Relay BK1-BK2, BK1-COM-H Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  // Check for the COM relay L Side
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");    // Close K7,K8,K14 (PS7)
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3 \r\n");     // Close K12 (PS6)
  TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n");  // Close BK1-Bk2 CH0 relays and SE relays
  test_cmd_result("Test 11.4: Relay BK1-BK2 COM-L Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@208)\r\n");  // Open SE relay
  test_cmd_result("Test 11.5: Relay BK1-BK2, BK2-COM-L Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n");  // Close BK1-Bk2 CH0 relays
  test_cmd_result("Test 11.6: Relay BK1-BK2 COM-L Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@108)\r\n");  // Open SE relay
  test_cmd_result("Test 11.7: Relay BK1-BK2, BK1-COM-L Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");  // OPen K12

  i = 0;  // count index
  j = 0;
  t = 12;

  /** Loop to test BK3-BK4 channel*/
  for (i = 0; i < 8; i++)
  {
    /** Test the High side of relay*/
    TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");  // Close K7,K8
    TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");   // Close K11 (PS9)

    sprintf(strval, "ROUT:CLOSE (@30%d,40%d)\r\n", i, i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sleep_ms(30);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 CH%d-H Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@40%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 BK4_CH%d-H Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:CLOSE (@40%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 BK4_CH%d-H Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@30%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 BK3_CH%d-H Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@40%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");

    /** Test the LOW side of relay*/
    TEST_SCPI_INPUT("DIG:OUT:PORT0 #H13 \r\n");     // Close K7,K8,K11,K12 (PS10),K15
    TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2,OC3 \r\n");  // Close K11,K12
    sleep_ms(30);

    sprintf(strval, "ROUT:CLOSE (@30%d,40%d)\r\n", i, i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 CH%d-L Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@40%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 BK4_CH%d-L Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:CLOSE (@40%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 BK4_CH%d-L Close Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@30%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    sprintf(strval, "Test %d.%d Relay BK3-BK4 BK3_CH%d-L Open Test, read I(mA)", t, j++, i);  // build verification string
    test_cmd_result(strval, "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

    sprintf(strval, "ROUT:OPEN (@40%d)\r\n", i);  // build channel string
    TEST_SCPI_INPUT(strval);
    TEST_SCPI_INPUT("ROUT:OPEN:OC OC2,OC3 \r\n");  // OPen K11,K12
  }

  // Check for the COM relay H Side
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H13 \r\n");    // Close K7,K8,K15
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");     // Close K11 (PS9)
  TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n");  // Close BK3-Bk4 CH0 relays and SE relays
  sleep_ms(30);
  test_cmd_result("Test 13.0: Relay BK3-BK4 COM-H Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@408)\r\n");  // Open SE relay
  test_cmd_result("Test 13.1: Relay BK3-BK4, BK2-COM-H Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n");  // Close BK3-Bk4 CH0 relays
  test_cmd_result("Test 13.2: Relay BK3-BK4 COM-H Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@308)\r\n");  // Open SE relay
  test_cmd_result("Test 13.3: Relay BK3-BK4, BK3-COM-H Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  // Check for the COM relay L Side
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");     // Close K7,K8,
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2,OC3 \r\n");  // Close K11,K12 (PS10)
  TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n");   // Close BK3-Bk4 CH0 relays and SE relays
  sleep_ms(30);
  test_cmd_result("Test 13.4: Relay BK3-BK4 COM-L Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@408)\r\n");  // Open SE relay
  test_cmd_result("Test 13.5: Relay BK3-BK4, BK4-COM-L Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n");  // Close BK3-Bk4 CH0 relays
  test_cmd_result("Test 13.6: Relay BK3-BK4 COM-L Close Test,read I(mA)", "ANA:PWR:I? \r\n", 50, "mA", 5, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN (@308)\r\n");  // Open SE relay
  test_cmd_result("Test 13.7: Relay BK3-BK4, BK3-COM-L Open Test,read I(mA)", "ANA:PWR:I? \r\n", 0, "mA", 0.2, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2,OC3 \r\n");  // OPen K11,K12

  // Test of I2C. I2C is used to communicate with selftest board, the test option are limited because
  // we don't want to lost communication with selftest board
  TEST_SCPI_INPUT("COM:INIT:DIS I2C\r\n");  // Disable Master I2C, set GPIO to input
  // When I2C disable, check if pull-up resistor are present
  test_cmd_out("Test 14.1: I2C COM, Read GPIO #6 ", "GPIO:IN:DEV0:GP6? \r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 14.2: I2C COM, Read GPIO #7 ", "GPIO:IN:DEV0:GP7? \r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:D 8 \r\n");       // Set I2C_master to Databits to 8
  TEST_SCPI_INPUT("COM:I2C:B 100000\r\n");   // set i2C_master Baudrate to 100k
  TEST_SCPI_INPUT("COM:I2C:ADDR #H20\r\n");  // set I2C Address of selftest board (0x20)
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");   // Enable Master I2C
  test_cmd_out("Test 14.3: I2C COM, Get Device Status ", "COM:I2C:READ:LEN1? 100\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 14.4: I2C COM, Get Major version", "COM:I2C:READ:LEN1? 01 \r\n", "1", &c_test, &buffer);
  // Command to read GPIO Function (75) of GPIO6 (6)
  test_cmd_out("Test 14.5: I2C COM, Get GPIO function of #6 ", "COM:I2C:READ:LEN1? 75,6 \r\n", "3", &c_test, &buffer);
  test_cmd_out("Test 14.6: I2C COM, Get GPIO function of #7 ", "COM:I2C:READ:LEN1? 75,7 \r\n", "3", &c_test, &buffer);

  // Validate SPI in SIO mode first to very physical lines between interconnect IO board and
  // selftest board. More easy to debug if connections is missing
  TEST_SCPI_INPUT("COM:SPI:CS 5\r\n");       // set CS to GPIO 5
  TEST_SCPI_INPUT("COM:INIT:DIS SPI \r\n");  // Disable MASTER SPI, set all GP to input
  TEST_SCPI_INPUT("COM:I2C:WRI 112,1\r\n");  // Disable selftest SPI, set GPIO as output
  TEST_SCPI_INPUT("COM:I2C:WRI 10,2\r\n");   // Set Selftest GPIO2 =0
  test_cmd_out("Test 15.0: SPI COM, Read GPIO #2 in SIO mode", "GPIO:IN:DEV0:GP2? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,2\r\n");  // Set Selftest GPIO2 =1
  test_cmd_out("Test 15.1: SPI COM, Read GPIO #2 in SIO mode", "GPIO:IN:DEV0:GP2? \r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 10,4\r\n");  // Set Selftest GPIO4 =0, SPI_TX
  test_cmd_out("Test 15.2: SPI COM, Read GPIO #3 in SIO mode", "GPIO:IN:DEV0:GP3? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,4\r\n");  // Set Selftest GPIO4 =1, SPI_TX
  test_cmd_out("Test 15.3: SPI COM, Read GPIO #3 in SIO mode", "GPIO:IN:DEV0:GP3? \r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 10,3\r\n");  // Set Selftest GPIO3 =0
  test_cmd_out("Test 15.4: SPI COM, Read GPIO #4 in SIO mode", "GPIO:IN:DEV0:GP4? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,3\r\n");  // Set Selftest GPIO3 =1
  test_cmd_out("Test 15.5: SPI COM, Read GPIO #4 in SIO mode", "GPIO:IN:DEV0:GP4? \r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 10,5\r\n");  // Set Selftest GPIO5 =0
  test_cmd_out("Test 15.6: SPI COM, Read GPIO #5 in SIO mode", "GPIO:IN:DEV0:GP5? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,5\r\n");  // Set Selftest GPIO5 =1
  test_cmd_out("Test 15.7: SPI COM, Read GPIO #5 in SIO mode", "GPIO:IN:DEV0:GP5? \r\n", "1", &c_test, &buffer);

  // Validate SPI communication
  TEST_SCPI_INPUT("COM:SPI:D 16 \r\n");     // Set I2C_master to Databits to 16
  TEST_SCPI_INPUT("COM:SPI:M 4\r\n");       // set mode to 0 and CS toggle each byte
  TEST_SCPI_INPUT("COM:SPI:B 100000\r\n");  // set SPI Baudrate to 100k
  TEST_SCPI_INPUT("COM:INIT:ENA SPI\r\n");  // Enable SPI

  TEST_SCPI_INPUT("COM:I2C:WRI 113,#H18\r\n");  // Set SPI Mode 0 and Databits = 16 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 111,1 \r\n");    // Enable SPI on selftest board
  sleep_ms(250);                                // delay necessary after the enable, why?
  TEST_SCPI_INPUT("COM:SPI:READ:LEN1? #H1234\r\n");
  test_cmd_out("Test 15.8: SPI COM, Selftest Word Write-read", "COM:SPI:READ:LEN1? #H0001\r\n", "60875", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:I2C:WRI 113,#H10\r\n");  // Set SPI Mode 0 databits = 8 on selftest board
  TEST_SCPI_INPUT("COM:SPI:D 8 \r\n");          // Set I2C_master to Databits to 8
  TEST_SCPI_INPUT("COM:I2C:WRI 111,1 \r\n");    // Re-Enable SPI Selftest after change in config

  TEST_SCPI_INPUT("COM:SPI:READ:LEN1? #Hab\r\n");
  test_cmd_out("Test 15.9: SPI COM,Mode 0, Selftest Byte Write-read", "COM:SPI:READ:LEN1? #H1\r\n", "84", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SPI:M 5\r\n");           // set mode to 1 and CS toggle each byte
  TEST_SCPI_INPUT("COM:I2C:WRI 113,#H12\r\n");  // Set SPI Mode 1 databits = 8 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 111,1 \r\n");    // Re-Enable SPI Selftest after change in config

  TEST_SCPI_INPUT("COM:SPI:READ:LEN1? #HA5\r\n");
  test_cmd_out("Test 15.10: SPI COM,Mode 1, Selftest Byte Write-read", "COM:SPI:READ:LEN1? #H1\r\n", "90", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SPI:M 6\r\n");           // set mode to 2 and CS toggle each byte
  TEST_SCPI_INPUT("COM:I2C:WRI 113,#H14\r\n");  // Set SPI Mode 2 databits = 8 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 111,1 \r\n");    // Re-Enable SPI Selftest after change in config

  TEST_SCPI_INPUT("COM:SPI:READ:LEN1? #H5A\r\n");
  test_cmd_out("Test 15.11: SPI COM,Mode 2, Selftest Byte Write-read", "COM:SPI:READ:LEN1? #H1\r\n", "165", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SPI:M 7\r\n");           // set mode to 3 and CS toggle each byte
  TEST_SCPI_INPUT("COM:I2C:WRI 113,#H15\r\n");  // Set SPI Mode 3 databits = 8 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 111,1 \r\n");    // Re-Enable SPI Selftest after change in config

  TEST_SCPI_INPUT("COM:SPI:READ:LEN1? #H78\r\n");
  test_cmd_out("Test 15.12: SPI COM,Mode 3, Selftest Byte Write-read", "COM:SPI:READ:LEN1? #H1\r\n", "135", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:INIT:DIS SPI \r\n");  // Disable MASTER SPI, set all GP to input
  TEST_SCPI_INPUT("COM:I2C:WRI 112,1\r\n");  // Disable selftest SPI, set GPIO as output

  // Serial Test Section

  // Validate UART in SIO mode first to very physical lines between interconnect IO board and
  // selftest board. More easy to debug if connections is missing
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");      // Enable Master I2C to talk to selftest board
  TEST_SCPI_INPUT("COM:INIT:DIS SERIAL \r\n");  // Disable MASTER UART, set all GP to input
  TEST_SCPI_INPUT("COM:I2C:WRI 102,1\r\n");     // Disable selftest UART, set GPIO as output
  TEST_SCPI_INPUT("COM:I2C:WRI 10,12\r\n");     // Set Selftest GPIO12 =0 (Rx)
  test_cmd_out("Test 16.0: SERIAL COM, Read GPIO #13 (Rx) in SIO mode", "GPIO:IN:DEV0:GP13? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,12\r\n");  // Set Selftest GPIO12 =1 (Rx)
  test_cmd_out("Test 16.1: SERIAL COM, Read GPIO #13 (Rx) in SIO mode", "GPIO:IN:DEV0:GP13? \r\n", "1", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:I2C:WRI 10,13\r\n");  // Set Selftest GPIO13 =0 (Tx)
  test_cmd_out("Test 16.2: SERIAL COM, Read GPIO #12 (Tx) in SIO mode", "GPIO:IN:DEV0:GP12? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,13\r\n");  // Set Selftest GPIO13 =1 (Tx)
  test_cmd_out("Test 16.3: SERIAL COM, Read GPIO #12 (Tx) in SIO mode", "GPIO:IN:DEV0:GP12? \r\n", "1", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:I2C:WRI 10,14\r\n");  // Set Selftest GPIO14 =0 (Rts)
  test_cmd_out("Test 16.4: SERIAL COM, Read GPIO #15 (Rts) in SIO mode", "GPIO:IN:DEV0:GP15? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,14\r\n");  // Set Selftest GPIO14 =1 (Rts)
  test_cmd_out("Test 16.5: SERIAL COM, Read GPIO #15 (Rts) in SIO mode", "GPIO:IN:DEV0:GP15? \r\n", "1", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:I2C:WRI 10,15\r\n");  // Set Selftest GPIO15 =0 (Cts)
  test_cmd_out("Test 16.6: SERIAL COM, Read GPIO #14 (Cts) in SIO mode", "GPIO:IN:DEV0:GP14? \r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:WRI 11,15\r\n");  // Set Selftest GPIO15 =1 (Cts)
  test_cmd_out("Test 16.7: SERIAL COM, Read GPIO #14 (Cts) in SIO mode", "GPIO:IN:DEV0:GP14? \r\n", "1", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:INIT:ENA SERIAL\r\n");  // Enable SPI
  test_cmd_out("Test 16.8 SCPI SERIAL command", "COM:INIT:STAT? SERIAL\r\n", "1", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SERIAL:Timeout 1000\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Handshake OFF\r\n");
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");  // Enable Master I2C

  TEST_SCPI_INPUT("COM:SERIAL:Baudrate 115200\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Protocol O72\r\n");
  TEST_SCPI_INPUT("COM:I2C:WRI 103,#HEA\r\n");  // Set Uart to O72 @ 115200 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 101,#H0\r\n");
  sleep_ms(30); 
  test_cmd_out("Test 16.9 SCPI SERIAL command O72,115200", "COM:SERIAL:Read? 'TEST O72,115200\r'\r\n", "\"TEST O72,115200\"", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SERIAL:Baudrate 38400\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Protocol N81\r\n");
  TEST_SCPI_INPUT("COM:I2C:WRI 103,#H4C\r\n");  // Set Uart to N81 @ 38400 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 101,#H0\r\n");   // Enable uart on selftest board
  sleep_ms(30); 
  test_cmd_out("Test 16.10 SCPI SERIAL command N81,38400", "COM:SERIAL:Read? 'TEST N81,38400\r'\r\n", "\"TEST N81,38400\"", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SERIAL:Handshake OFF\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Baudrate 19200\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Protocol E61\r\n");
  TEST_SCPI_INPUT("COM:I2C:WRI 103,#H14\r\n");  // Set Uart to E61 @ 19200 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 101,#H0\r\n");
  sleep_ms(30); 
  test_cmd_out("Test 16.11 SCPI SERIAL command E61,19200", "COM:SERIAL:Read? '1234567890,19200\r'\r\n", "\"1234567890,19200\"", &c_test, &buffer);

  TEST_SCPI_INPUT("COM:SERIAL:Handshake ON\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Baudrate 57600\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:Protocol N82\r\n");
  TEST_SCPI_INPUT("COM:I2C:WRI 103,#H8E\r\n");  // Set Uart to N82 @ 57600 on selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 101,#H1\r\n");
  sleep_ms(30); 
  test_cmd_out("Test 16.12 SCPI SERIAL command Handshake,57600", "COM:SERIAL:Read? 'TEST HANDSHAKE,57600\r'\r\n", "\"TEST HANDSHAKE,57600\"", &c_test,
               &buffer);

  TEST_SCPI_INPUT("COM:INIT:DIS SERIAL\r\n");  // Disable Serial
  TEST_SCPI_INPUT("COM:I2C:WRI 102,#H0\r\n");

  // 1-WIRE test section
  test_cmd_substring("Test 17.0 1-WIRE command Check Device", "COM:OW:Check? 2\r\n", "VALID_OWID: 2D", &c_test, &buffer);
  test_cmd_substring("Test 17.1 1-WIRE command Check Device", "COM:OW:READ? 2\r\n", "500-1010", &c_test, &buffer);

  // Validate service GPIO between Interconnect and Selftest board

  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");   // Enable Master I2C to talk to selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 21,11\r\n");  // Set GPIO 11 to input on selftest Board
  TEST_SCPI_INPUT("SYSTEM:LED:ERR ON\r\n");  // Turn Error LED ON
  test_cmd_out("Test 18.0: ERROR LED TEST, Get GPIO 11 value when LED is ON  ", "COM:I2C:READ:LEN1? 15,11\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF\r\n");
  test_cmd_out("Test 18.1: ERROR LED TEST, Get GPIO 11 value when LED is OFF  ", "COM:I2C:READ:LEN1? 15,11\r\n", "1", &c_test, &buffer);

  if (run == 3)
  {
    // This section of test requires to have loopback connector
    // Loopback CTRL INST (J23):   Connect J23-1 to J23-2
    // Loopback JTAG (J26):    Connect J26-5 to J26-9, Connect J26-1 to J26-3 and J26-7

    // Test of J23 Loopback
    TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 0 \r\n");  // set io to input
    TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 1 \r\n");  // set io to output
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1 0  \r\n");
    test_cmd_out("Test 19.0: J23 Loopback Test, GP1 set to 0, GP1 read?  ", "GPIO:IN:DEV0:GP1? \r\n", "0", &c_test, &buffer);
    test_cmd_out("Test 19.1: J23 Loopback Test, GP1 set to 0, GP0 read?  ", "GPIO:IN:DEV0:GP0? \r\n", "0", &c_test, &buffer);
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1 1  \r\n");
    test_cmd_out("Test 19.2: J23 Loopback Test, GP1 set to 1, GP1 read?  ", "GPIO:IN:DEV0:GP1? \r\n", "1", &c_test, &buffer);
    test_cmd_out("Test 19.3: J23 Loopback Test, GP1 set to 1, GP0 read?  ", "GPIO:IN:DEV0:GP0? \r\n", "1", &c_test, &buffer);
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1 0  \r\n");
    test_cmd_out("Test 19.4: J23 Loopback Test, GP1 set to 0, GP0 read?  ", "GPIO:IN:DEV0:GP0? \r\n", "0", &c_test, &buffer);
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1 1  \r\n");

    TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 1 \r\n");  // set io to output
    TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 0 \r\n");  // set io to input
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0  \r\n");
    test_cmd_out("Test 19.5: J23 Loopback Test, GP0 set to 0, GP0 read?  ", "GPIO:IN:DEV0:GP0? \r\n", "0", &c_test, &buffer);
    test_cmd_out("Test 19.6: J23 Loopback Test, GP0 set to 0, GP1 read?  ", "GPIO:IN:DEV0:GP1? \r\n", "0", &c_test, &buffer);
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 1  \r\n");
    test_cmd_out("Test 19.7: J23 Loopback Test, GP0 set to 0, GP0 read?  ", "GPIO:IN:DEV0:GP0? \r\n", "1", &c_test, &buffer);
    test_cmd_out("Test 19.8: J23 Loopback Test, GP0 set to 1, GP1 read?  ", "GPIO:IN:DEV0:GP1? \r\n", "1", &c_test, &buffer);
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0  \r\n");
    test_cmd_out("Test 19.9: J23 Loopback Test, GP0 set to 0, GP1 read?  ", "GPIO:IN:DEV0:GP1? \r\n", "0", &c_test, &buffer);
    TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 1  \r\n");

    // Test of J26 Loopback

    TEST_SCPI_INPUT("COM:I2C:WRI 20,16\r\n");  // Set Selftest GPIO16 to output
    TEST_SCPI_INPUT("COM:I2C:WRI 21,18\r\n");  // Set Selftest GPIO18 to input
    TEST_SCPI_INPUT("COM:I2C:WRI 10,16\r\n");  // Set Selftest GPIO16 = 0
    test_cmd_out("Test 20.0: J26 Loopback Test, Selftest  GP16 set to 0, GP16 read?  ", "COM:I2C:REA:LEN1? 15,16\r\n", "0", &c_test, &buffer);
    test_cmd_out("Test 20.1: J26 Loopback Test, Selftest  GP16 set to 0, GP18 read?  ", "COM:I2C:REA:LEN1? 15,18\r\n", "0", &c_test, &buffer);

    TEST_SCPI_INPUT("COM:I2C:WRI 11,16\r\n");  // Set Selftest GPIO16 = 1
    test_cmd_out("Test 20.2: J26 Loopback Test, Selftest  GP16 set to 0, GP16 read?  ", "COM:I2C:REA:LEN1? 15,16\r\n", "1", &c_test, &buffer);
    test_cmd_out("Test 20.3: J26 Loopback Test, Selftest  GP16 set to 1, GP21 read?  ", "COM:I2C:REA:LEN1? 15,18\r\n", "1", &c_test, &buffer);
    TEST_SCPI_INPUT("COM:I2C:WRI 10,16\r\n");  // Set Selftest GPIO16 = 0

    TEST_SCPI_INPUT("COM:I2C:WRI 20,21\r\n");  // Set Selftest GPIO21 to output
    TEST_SCPI_INPUT("COM:I2C:WRI 21,19\r\n");  // Set Selftest GPIO19 to input
    TEST_SCPI_INPUT("COM:I2C:WRI 21,17\r\n");  // Set Selftest GPIO17 to input
    TEST_SCPI_INPUT("COM:I2C:WRI 10,21\r\n");  // Set Selftest GPIO21 = 0
    test_cmd_out("Test 20.4: J26 Loopback Test, Selftest  GP21 set to 0, GP21 read?  ", "COM:I2C:REA:LEN1? 15,21\r\n", "0", &c_test, &buffer);
    test_cmd_out("Test 20.5: J26 Loopback Test, Selftest  GP21 set to 0, GP17 read?  ", "COM:I2C:REA:LEN1? 15,17\r\n", "0", &c_test, &buffer);
    test_cmd_out("Test 20.6: J26 Loopback Test, Selftest  GP21 set to 0, GP19 read?  ", "COM:I2C:REA:LEN1? 15,19\r\n", "0", &c_test, &buffer);
    TEST_SCPI_INPUT("COM:I2C:WRI 11,21\r\n");  // Set Selftest GPIO21 = 1
    test_cmd_out("Test 20.7: J26 Loopback Test, Selftest  GP21 set to 0, GP21 read?  ", "COM:I2C:REA:LEN1? 15,21\r\n", "1", &c_test, &buffer);
    test_cmd_out("Test 20.8: J26 Loopback Test, Selftest  GP21 set to 1, GP17 read?  ", "COM:I2C:REA:LEN1? 15,17\r\n", "1", &c_test, &buffer);
    test_cmd_out("Test 20.9: J26 Loopback Test, Selftest  GP21 set to 1, GP19 read?  ", "COM:I2C:REA:LEN1? 15,19\r\n", "1", &c_test, &buffer);
    TEST_SCPI_INPUT("COM:I2C:WRI 10,21\r\n");  // Set Selftest GPIO21 = 0
  }

  /** PRINT FINAL REPORT AFTER TEST COMPLETION*/
  fprintf(stdout, "\n\n\t SELFTEST COMPLETED REPORT \n\n");
  fprintf(stdout, "\t Number of Tests performed:\t%d\r\n", c_test.total);
  fprintf(stdout, "\t Number of Tests PASS:\t\t%d\r\n", c_test.good);
  fprintf(stdout, "\t Number of Tests FAIL:\t\t%d\r\n", c_test.bad);
  fprintf(stdout, "\t Number of Tests ERROR:\t%d\r\n", c_test.error);

  // send result string to serial port
  sprintf(strval, "SELFTEST RESULTS: \n NbTotal: %d, NbGood: %d, NbBad: %d, NbError: %d\n", c_test.total, c_test.good, c_test.bad,
          c_test.error);       // build string to return
  uart_puts(UART_ID, strval);  // Send result

  //  // Print all stored failure messages
  if (c_test.bad > 0 || c_test.error > 0)
  {
    TEST_SCPI_INPUT("SYSTEM:LED:ERR ON \r\n");  // turn ON Error led
    fprintf(stdout, "\nStored bad messages:\n");

    print_messages(&buffer);
  }

  sprintf(strval, "SELFTEST COMPLETED \n");  // build string to return
  uart_puts(UART_ID, strval);                // Send string
}

/*! @brief - The function perform the selftest of the instruments connected to the
 *    interconnect_IO board. The instruments are controlled manually using instruction
 *    on the master serial port
 *
 */
void test_inst_manual()
{
  int result;
  char in[20];
  char strval[120];
  char c;

  TEST_SCPI_INPUT("SYST:SLA OFF\r\n"); /** Disable slaves Pico to reset configuration*/
  sleep_ms(100);
  TEST_SCPI_INPUT("SYST:SLA ON\r\n"); /** Start slaves Pico after reset*/
  sleep_ms(500);
  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF\r\n"); /** Turn OFF red led */

  // initialisation of all resources used on selftest board to toggle relays
  // All digital IO are set as input after reset.
  // All IO need to be set as output and initialized to 0 before the start of the test
  //
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");   // set direction port 0 ouput
  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");   // set direction port 1 input
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");   // set port 0 to 00
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP8 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP9 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1  0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9  0 \r\n");
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP18 1 \r\n");   // set to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP19 0 \r\n");   // set to input
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG output
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP19  0 \r\n");  // CTRL Input
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");

  // Setup I2C configuration to be used to communicate with selftest Board

  TEST_SCPI_INPUT("COM:I2C:D 8 \r\n");       // Set I2C_master to Databits to 8
  TEST_SCPI_INPUT("COM:I2C:B 100000\r\n");   // set i2C_master Baudrate to 100k
  TEST_SCPI_INPUT("COM:I2C:ADDR #H20\r\n");  // set I2C Address of selftest board (0x20)
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");   // Enable Master I2C

  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF \r\n");  // turn OFF Error led

  TEST_SCPI_INPUT("SYST:OUT ON\r\n"); /** Close Power Relay et apply 5V to Selftest board */
  sleep_ms(250);                      /** Wait to let time to relay to close and power the selftest board */

  // Disable RX interrupt to take control of serial input
  uart_set_irq_enables(UART_ID, false, false);

  sprintf(strval, "\n Manual Instruments Test \n");
  uart_puts(UART_ID, strval);  // Send string

  // Using DMM in resistance mode, measure sense resistor R6 (10 ohm)
  sprintf(strval, "Connect DMM to Sense pins (+:J20-5 & -:J20-6)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set DMM to be able to read 10 ohms resistors\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Test 25.0 Verify if ohmmeter value is between 10 and 16 Ohm, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  // Using DMM in resistance mode, measure test resistors R5 (4.7 ohm), R6 (10 ohm) and R7 (4.7 ohm)
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 1 \r\n");  // Close K6
  sprintf(strval, "\nConnect DMM to input pins (+:J20-2 & -:J20-3)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set DMM to be able to read 25 ohms resistors, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char
  sprintf(strval, "Test 25.1 Verify if DMM ohmmeter value is between 20 and 23 Ohm, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0 \r\n");  // Open K6

  // Using DMM in resistance mode, measure selftest relays path when VM1 is grounded by K4
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H08 \r\n");     // Close K14 (VM1)
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");      // Close K10 to isolate PS1
  sprintf(strval, "Test 25.2 Verify if DMM ohmmeter value is between 0 and 5 Ohm, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                  // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4

  // Using PWR_5V, validate Voltmeter function of DMM
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H08 \r\n");  // Close K14 (VM1)
  sprintf(strval, "\nSet DMM to Voltmeter,press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                             // wait for input char
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");  // Open K10
  sprintf(strval, "Test 25.3 Verify if DMM Voltmeter value is between 4.75V and 5.25V, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  // Using PWR_5V and R1 (100 Ohm), validate Low current function of DMM
  sprintf(strval, "\nConnect DMM to Current (I:J20-1 & -:J20-3) . Set for Current measurement, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1 1 \r\n");  // Close K5
  sprintf(strval, "Test 25.4 Verify if DMM Ammeter value is between 48mA and 52mA, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  // Using PWR_5V and R2 (10 Ohm), validate High current function of DMM
  // Expected current is 500mA but due to current resistance of DMM and relay contact
  // The final current is around 350mA
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8 1 \r\n");    // Close K13
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  sprintf(strval, "Test 25.5 Verify if DMM Ammeter value is between 300mA and 400mA, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                  // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1 0 \r\n");    // Open K5
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8 0 \r\n");    // Open K13

  // Test of DMM trigger signal
  // Trigger signal will be activated and voltage will be measured with DMM
  sprintf(strval, "\nConnect DMM Trig pins to DMM input High (+:J20-4 & -:J20-3)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set DMM to voltmeter mode (5V Range), press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char
  sprintf(strval, "Test 25.6 Verify if DMM value value is between 0V  and 0.1 Volt, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                  // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // DVM_TRIG =1
  sprintf(strval, "Test 25.7 Verify if DMM value value is between 2V  and 3.3 Volt, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                  // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // DVM_TRIG =0

  // Test Of PS1 and PS2 power supply source
  // 10 ohm resistor located on selftest board will be used as load for the supply
  // Voltage at load will be measured by DMM on (VM6). SSR is used to connect (PS4) to the load
  sprintf(strval, "\nConnect DMM to input pins (+:J20-2 & -:J20-3)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set DMM to be able to read 10V\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Connect 12Vdc Power supply to PS1 (+:J17-1 & -:J17:3), press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                  // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #HBD \r\n");     // Close K3,K8,K14,K15,K16,K9
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");      // close K10 to set PS4
  sprintf(strval, "Test 26.0 Verify if DMM value value is between 0V  and 0.1 Volt, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                // wait for input char
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR SSR1 \r\n");  // Connect PS4 to 10 ohm and read to VM6
  sprintf(strval, "Test 26.1 Verify if DMM value value is between 5V and 6V, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                               // wait for input char
  TEST_SCPI_INPUT("ROUT:OPEN:PWR SSR1 \r\n");  // Connect PS4 to 10 ohm and read to VM6

  sprintf(strval, "\nConnect 10Vdc Power supply to PS2 (+:J17-2 & -:J17:4), press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                // wait for input char
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9 1 \r\n");  // Close K1
  sprintf(strval, "Test 26.2 Verify if DMM value value is between 0V  and 0.1 Volt, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                // wait for input char
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR SSR1 \r\n");  // Connect PS4 to 10 ohm and read to VM6
  sprintf(strval, "Test 26.3 Verify if DMM value value is between 4V and 5V, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                                  // wait for input char
  TEST_SCPI_INPUT("ROUT:OPEN:PWR SSR1 \r\n");     // Connect PS4 to 10 ohm and read to VM6
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");     // Open K3,K8,K14,K15,K16,K9
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");       // Open K10
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9 0 \r\n");    // Open K1

  // Test of Oscilloscope CH1 and CH2 using the Pico PWM on selftest board
  sprintf(strval, "\nConnect Oscilloscope to CH1 (J18)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set Vertical channel to 1V and timebase to 500us, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                               // wait for input char
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H02 \r\n");  // Close K7
  TEST_SCPI_INPUT("COM:I2C:WRI 80,1\r\n");     // Set PWM ON
  TEST_SCPI_INPUT("COM:I2C:WRI 81,1\r\n");     // Set PWM Freq to 1Khz
  sprintf(strval, "Test 27.0 Verify on SCOPE CH1 if 3.3V@1KHz square wave is present, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  sprintf(strval, "\nConnect Oscilloscope to CH2 (J19)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set Vertical channel to 1V and timebase to 5us, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                               // wait for input char
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H06 \r\n");  // Close K3,K7
  TEST_SCPI_INPUT("COM:I2C:WRI 80,1\r\n");     // Set PWM ON
  TEST_SCPI_INPUT("COM:I2C:WRI 81,100\r\n");   // Set PWM Freq to 100Khz
  sprintf(strval, "Test 27.1 Verify on SCOPE CH2 if 3.3V@100KHz square wave is present, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                               // wait for input char
  TEST_SCPI_INPUT("COM:I2C:WRI 80,0\r\n");     // Set PWM OFF
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // Open K3,K7

  // Test of AWG using oscilloscope CH1
  sprintf(strval, "\nConnect Signal Generator to AWG input (J21), Connect Oscilloscope to CH1 (J18)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set Vertical channel to 1V and timebase to 50us\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set Signal Generator to 10KHz sinus at 5Vpp, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char
  sprintf(strval, "Test 28.0 Verify on SCOPE CH1 if 5Vpp @10KHz sinus is present, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  // Test of INST using oscilloscope CH1
  sprintf(strval, "\nConnect Signal Generator to SPARE input (J22), Connect Oscilloscope to CH1 (J18)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set Vertical channel to 1V and timebase to 50us\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Set Signal Generator to 10KHz triangle at 5Vpp, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                               // wait for input char
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H40 \r\n");  // Close K2
  sprintf(strval, "Test 29.0 Verify on SCOPE CH1 if 5Vpp @10KHz triangle is present, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);                               // wait for input char
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H40 \r\n");  // Open K2

  // Test of USB connector using USB Flash Drive connected to USB connector on selftest board
  sprintf(strval, "\nConnect USB cable Type B between computer and USB connector on interconnect IO (J25)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Connect USB Flash Drive to USB connector Type A on Selftest Board (J4), press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char
  sprintf(strval, "Test 30.0 Verify if computer as detected and could read the USB flash drive, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  // Test of RJ45 connector by passing active network communication to RJ45 connector on selftest board
  sprintf(strval, "\nDisconnect network cable from computer and connect to RJ45 on interconnect IO board (J24)\n");
  uart_puts(UART_ID, strval);  // Send string
  sprintf(strval, "Connect a new network cable type RJ45 between selftest board (J3) and computer, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char
  sprintf(strval, "Test 31.0 Verify if computer network is ON and work properly, press enter\n");
  uart_puts(UART_ID, strval);  // Send string
  c = read_uart_char();
  sleep_ms(500);  // wait for input char

  TEST_SCPI_INPUT("SYST:OUT OFF\r\n"); /** Open Power Relay et remove 5V to Selftest board */
  sleep_ms(250);                       /** Wait to let time to relay to close and power the selftest board */

  // Enable RX interrupt to take control of serial input
  uart_set_irq_enables(UART_ID, true, false);

  sprintf(strval, "\n\n End of Manual Instruments Test\n");
  uart_puts(UART_ID, strval);  // Send string
}

/*! @brief - The function has been used to test the scpi command function.
 *            Mainly used to validate software functionalities.
 *            selftest board could be required to be present.
 *
 */
void test_command(void)
{
  int result;
  int i = 0;
  char strval[120];

  struct TestResult c_test = {0, 0, 0, 0};  // reset counter structure
  out_buffer_pos = 0;
  CircularBuffer buffer;
  init_buffer(&buffer);

  TEST_SCPI_INPUT("*CLS\r\n");

  // Clear scpi registers before checking registers.
  for (i = 0; i < 9; i++)
  {
    scpi_context.registers[i] = 0;
  }

  // SCPI_CoreCls(&scpi_context);
  TEST_SCPI_INPUT("SYST:OUT ON\r\n");  // Power selftest board
  test_cmd_out("Test 1.0 SCPI Base command IDN?", "*IDN?\r\n", "FirstTestStation,InterconnectIO,2022A,1.0", &c_test, &buffer);
  test_cmd_out("Test 1.1 SCPI Base command OPC?", "*OPC?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 1.2 SCPI Base command SYST:VERS?", "SYST:VERS?\r\n", "1999.0", &c_test, &buffer);

  test_cmd_out("Test 2.0 SCPI Register command STB?", "*STB?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 2.1 SCPI Register command ESE?", "*ESE?\r\n", "0", &c_test, &buffer);

  test_cmd_out("Test 2.2 SCPI Register command QUEST:COND?", "STATus:QUEStionable:CONDition?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 2.3 SCPI Register command OPER:COND?", "STATus:OPER:CONDition?\r\n", "0", &c_test, &buffer);
  SCPI_RegSetBits(&scpi_context, SCPI_REG_QUESC, 1);  // Set bit on Questionable Data Register
  test_cmd_out("Test 2.4 SCPI Register command QUEST:COND?", "STATus:QUEStionable:CONDition?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("STATus:QUEStionable:ENABle 255 \n");
  test_cmd_out("Test 2.5 SCPI Register command  QUEST:ENAB?", "STATus:QUEStionable:ENABle?\r\n", "255", &c_test, &buffer);
  test_cmd_out("Test 2.6 SCPI Register command STB?", "*STB?\r\n", "8", &c_test, &buffer);
  test_cmd_out("Test 2.7 SCPI Register command QUEST:Event?", "STATus:QUEStionable:Event?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 2.8 SCPI Register command STB?", "*STB?\r\n", "0", &c_test, &buffer);
  SCPI_RegClearBits(&scpi_context, SCPI_REG_QUESC, 1);  // Clear bit on Questionable Data Register

  SCPI_RegSetBits(&scpi_context, SCPI_REG_OPERC, 2);  // Set bit on Operation Data Register
  test_cmd_out("Test 2.9 SCPI Register command OPER:COND?", "STAT:OPER:COND?\r\n", "2", &c_test, &buffer);
  TEST_SCPI_INPUT("STATus:OPER:ENABle 2 \n");
  test_cmd_out("Test 2.10 SCPI Register command OPER:ENAB?", "STAT:OPER:ENAB?\r\n", "2", &c_test, &buffer);
  test_cmd_out("Test 2.11 SCPI Register command STB?", "*STB?\r\n", "128", &c_test, &buffer);
  test_cmd_out("Test 2.12 SCPI Register command OPER:Event?", "STATus:OPER:Event?\r\n", "2", &c_test, &buffer);
  test_cmd_out("Test 2.13 SCPI Register command STB?", "*STB?\r\n", "0", &c_test, &buffer);
  SCPI_RegClearBits(&scpi_context, SCPI_REG_OPERC, 2);  // Clear bit on Operation Data Register

  TEST_SCPI_INPUT("SYST:SLA OFF\r\n");  // Set bit 6 of ESR register
  TEST_SCPI_INPUT("SYST:OUT ON\r\n");   // Set bit 7 of ESR register
  TEST_SCPI_INPUT("*ESE 255 \r\n");
  test_cmd_out("Test 2.14 SCPI Register command ESE?", "*ESE?\r\n", "255", &c_test, &buffer);
  test_cmd_out("Test 2.15 SCPI Register command STB?", "*STB?\r\n", "32", &c_test, &buffer);
  test_cmd_out("Test 2.16 SCPI Register command ESR?", "*ESR?\r\n", "192", &c_test, &buffer);  // Clear register after read
  test_cmd_out("Test 2.17 SCPI Register command ESR?", "*ESR?\r\n", "0", &c_test, &buffer);    // Check register clear
  TEST_SCPI_INPUT("SYST:SLA ON\r\n");                                                          // Clear bit 6 of ESR register
  TEST_SCPI_INPUT("SYST:OUT OFF\r\n");                                                         // Clear bit 7 of ESR register

  TEST_SCPI_INPUT("STATus:QUES:ENABle 7 \n");
  SCPI_RegSetBits(&scpi_context, SCPI_REG_QUESC, 1);  // Set bit on Questionable Data Register
  TEST_SCPI_INPUT("STATus:OPER:ENABle 7 \n");
  SCPI_RegSetBits(&scpi_context, SCPI_REG_OPERC, 2);  // Set bit on Operation Data Register
  test_cmd_out("Test 2.18 SCPI Register command STB?", "*STB?\r\n", "136", &c_test, &buffer);
  TEST_SCPI_INPUT("*CLS\r\n");  // Clear event register of Questionable and Operation
  test_cmd_out("Test 2.19 SCPI Register command STB?", "*STB?\r\n", "0", &c_test, &buffer);
  SCPI_RegClearBits(&scpi_context, SCPI_REG_QUESC, 1);  // Clear bit on Questionable Data Register
  SCPI_RegClearBits(&scpi_context, SCPI_REG_OPERC, 2);  // Clear bit on Operation Data Register

  SCPI_RegSetBits(&scpi_context, SCPI_REG_QUESC, 1);  // Set bit on Questionable Data Register
  test_cmd_out("Test 2.20 SCPI Register command STB?", "*STB?\r\n", "8", &c_test, &buffer);
  TEST_SCPI_INPUT("STAT:PRES\r\n");  // Clear event register of Questionable
  test_cmd_out("Test 2.21 SCPI Register command STB?", "*STB?\r\n", "0", &c_test, &buffer);
  SCPI_RegClearBits(&scpi_context, SCPI_REG_QUESC, 1);  // Clear bit on Questionable Data Register

  test_cmd_out("Test 3.0 SCPI Error command LED:ERR?", "SYSTEM:LED:ERR?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 3.1 SCPI Error command ERR:COUNT?", "SYST:ERR:COUNt?\r\n", "0", &c_test, &buffer);
  SCPI_ErrorPush(&scpi_context, SCPI_ERROR_TIME_OUT);  // raise error to verify error command and led
  test_cmd_out("Test 3.2 SCPI Error command LED:ERR?", "SYSTEM:LED:ERR?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 3.3 SCPI Error command ERR:COUNT?", "SYST:ERR:COUNt?\r\n", "1", &c_test, &buffer);
  SCPI_ErrorPush(&scpi_context, SCPI_ERROR_QUERY_ERROR);  // raise error to verify error command and led
  test_cmd_out("Test 3.4 SCPI Error command", "SYST:ERR:COUNt?\r\n", "2", &c_test, &buffer);
  test_cmd_out("Test 3.5 SCPI Error command", "SYST:ERR?\r\n", "-365,\"Time out error\"", &c_test, &buffer);
  test_cmd_out("Test 3.6 SCPI Error command", "SYST:ERR:NEXT?\r\n", "-400,\"Query error\"", &c_test, &buffer);
  test_cmd_out("Test 3.7 SCPI Error command", "SYSTEM:LED:ERR?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("SYSTEM:LED:ERR ON\r\n"); /** Turn ON red led */
  test_cmd_out("Test 3.8 SCPI Error command", "SYSTEM:LED:ERR?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF\r\n"); /** Turn OFF red led */
  test_cmd_out("Test 3.9 SCPI Error command", "SYSTEM:LED:ERR?\r\n", "0", &c_test, &buffer);

  // RElay command Check

  TEST_SCPI_INPUT("ROUT:CLOSE (@100:102,201:204,303:306,404:407)\r\n");
  test_cmd_out("Test 4.0 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "7,30,120,240", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN (@100,201,303,404)\r\n");
  test_cmd_out("Test 4.1 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "6,28,112,224", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:CLOSE:EXCL (@100,201,303,404)\r\n");
  test_cmd_out("Test 4.2 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "1,2,8,16", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:CLOSE (@115,215,315,415)\r\n");
  test_cmd_out("Test 4.3 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "129,130,136,144", &c_test, &buffer);
  test_cmd_out("Test 4.4 SCPI Relay command", "ROUT:CHAN:STAT? (@115,215,315,415)\r\n", "1,1,1,1", &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2,BANK3,BANK4\r\n");
  test_cmd_out("Test 4.5 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "0,0,0,0", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN (@115,215,315,415)\r\n");
  test_cmd_out("Test 4.6 SCPI Relay command", "ROUT:CHAN:STAT? (@115,215,315,415)\r\n", "0,0,0,0", &c_test, &buffer);
  test_cmd_out("Test 4.7 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "0,0,0,0", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:CLOSE (@108:115,208:215,308:315,408:415)\r\n");
  test_cmd_out("Test 4.8 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "255,255,255,255", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2,BANK3,BANK4\r\n");
  TEST_SCPI_INPUT("ROUT:OPEN (@115,215,315,415)\r\n");  // temporary to fix bug
  test_cmd_out("Test 4.9 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "0,0,0,0", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:CLOSE:Rev BANK1,BANK2,BANK3,BANK4\r\n");
  test_cmd_out("Test 4.10 SCPI Relay command", "ROUT:REV:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "1,1,1,1", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:Rev BANK2,BANK4\r\n");
  test_cmd_out("Test 4.11 SCPI Relay command", "ROUT:REV:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "1,0,1,0", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2,BANK3,BANK4\r\n");
  test_cmd_out("Test 4.12 SCPI Relay command", "ROUT:REV:STAT? BANK1,BANK2,BANK3,BANK4\r\n", "0,0,0,0", &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1,LPR2,HPR1,SSR1 \r\n");
  test_cmd_out("Test 4.13 SCPI PWR Relay command", "ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1\r\n", "1,1,1,1", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR2,SSR1 \r\n");
  test_cmd_out("Test 4.14 SCPI PWR Relay command", "ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1\r\n", "1,0,1,0", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1,HPR1 \r\n");
  test_cmd_out("Test 4.15 SCPI PWR Relay command", "ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1\r\n", "0,0,0,0", &c_test, &buffer);

  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1,OC2,OC3 \r\n");
  test_cmd_out("Test 5.0 SCPI Open Collector command", "ROUT:STATE:OC? OC1,OC2,OC3\r\n", "1,1,1", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
  test_cmd_out("Test 5.1 SCPI Open Collector command", "ROUT:STATE:OC? OC1,OC2,OC3\r\n", "0,1,1", &c_test, &buffer);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2,OC3 \r\n");
  test_cmd_out("Test 5.2 SCPI Open Collector command", "ROUT:STATE:OC? OC1,OC2,OC3\r\n", "0,0,0", &c_test, &buffer);

  // Digital command Check
  // Require presence of the selftest board ( need to be powered)

  TEST_SCPI_INPUT("SYST:OUT ON\r\n");  // Power selftest board

  TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n");  // set direction port0 output(1)
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #H00 \r\n");  // set direction port1 input(0)
  TEST_SCPI_INPUT("DIG:OUT:PORT1 #H55 \r\n");
  test_cmd_out("Test 6.1 SCPI Digital command", "DIG:IN:PORT0?\r\n", "85", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT1 #HAA \r\n");
  test_cmd_out("Test 6.2 SCPI Digital command", "DIG:IN:PORT0?\r\n", "170", &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HF0 \r\n");  // set direction port0
  test_cmd_out("Test 6.3 SCPI Digital command", "DIG:DIR:PORT0?\r\n", "240", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H0F \r\n");  // set direction port1
  test_cmd_out("Test 6.4 SCPI Digital command", "DIG:DIR:PORT1?\r\n", "15", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT0 240 \r\n");
  TEST_SCPI_INPUT("DIG:OUT:PORT1 0 \r\n");
  test_cmd_out("Test 6.5 SCPI Digital command", "DIG:IN:PORT1?\r\n", "240", &c_test, &buffer);  // same number due to loopback
  test_cmd_out("Test 6.6 SCPI Digital command", "DIG:IN:PORT0?\r\n", "240", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT0 0 \r\n");
  TEST_SCPI_INPUT("DIG:OUT:PORT1 15 \r\n");
  test_cmd_out("Test 6.7 SCPI Digital command", "DIG:IN:PORT0?\r\n", "15", &c_test, &buffer);
  test_cmd_out("Test 6.8 SCPI Digital command", "DIG:IN:PORT1?\r\n", "15", &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");    // set direction port input(0)
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");    // set direction port output(1)
  TEST_SCPI_INPUT("DIG:DIR:PORT1:BIT0  1\r\n");  // set direction port output(1)
  TEST_SCPI_INPUT("DIG:DIR:PORT0:BIT0  0\r\n");  // set direction port input(0)
  test_cmd_out("Test 6.9 SCPI Digital command", "DIG:DIR:PORT1:BIT0?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 6.10 SCPI Digital command", "DIG:DIR:PORT0:BIT0?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT1:BIT0 1 \r\n");
  test_cmd_out("Test 6.11 SCPI Digital command", "DIG:IN:PORT0:BIT0?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("DIG:OUT:PORT1:BIT0 0 \r\n");
  test_cmd_out("Test 6.12 SCPI Digital command", "DIG:IN:PORT0:BIT0?\r\n", "0", &c_test, &buffer);

  // Test of GPIO Command
  // SYNC IO (GP22) will be used to validate most of the command.
  // Note: SYNC is for future use and no supported on this version

  gpio_init(22);

  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  1 \r\n");  // Set direction Gp22 as output DEV0 = Master Pico
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  0 \r\n");  // Set direction Gp22 as input DEV1 = Slave1 Pico
  TEST_SCPI_INPUT("GPIO:DIR:DEV2:GP22  0 \r\n");  // Set direction Gp22 as input DEV2 = Slave2 Pico
  TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  0 \r\n");  // Set direction Gp22 as input DEV3 = Slave3 Pico
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  1 \r\n");  // Set Gp22 = 1
  test_cmd_out("Test 7.0 SCPI GPIO command DEV1", "GPIO:IN:DEV1:GP22?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 7.1 SCPI GPIO command DEV2", "GPIO:IN:DEV2:GP22?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 7.2 SCPI GPIO command DEV3", "GPIO:IN:DEV3:GP22?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  0 \r\n");  // Set Gp22 = 0
  test_cmd_out("Test 7.3 SCPI GPIO command DEV1", "GPIO:IN:DEV1:GP22?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 7.4 SCPI GPIO command DEV2", "GPIO:IN:DEV2:GP22?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 7.5 SCPI GPIO command DEV3", "GPIO:IN:DEV3:GP22?\r\n", "0", &c_test, &buffer);

  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  0 \r\n");  // Set direction Gp22 as input DEV0 = Master Pico
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  0 \r\n");  // Set direction Gp22 as input DEV1 = Slave1 Pico
  TEST_SCPI_INPUT("GPIO:DIR:DEV2:GP22  0 \r\n");  // Set direction Gp22 as input DEV2 = Slave2 Pico
  TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  1 \r\n");  // Set direction Gp22 as output DEV3 = Slave3 Pico
  TEST_SCPI_INPUT("GPIO:OUT:DEV3:GP22  1 \r\n");  // Set Gp22 = 1
  test_cmd_out("Test 7.6 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 7.7 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n", "1", &c_test, &buffer);
  test_cmd_out("Test 7.8 SCPI GPIO command", "GPIO:IN:DEV2:GP22?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV3:GP22  0 \r\n");  // Set Gp22 = 0
  test_cmd_out("Test 7.9 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 7.10 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n", "0", &c_test, &buffer);
  test_cmd_out("Test 7.11 SCPI GPIO command", "GPIO:IN:DEV2:GP22?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  0 \r\n");  // Set direction Gp22 as input DEV3 = Slave3 Pico

  /** PAD REGISTER */
  /*  Bit 7   OD    Output disable   */
  /*  Bit 6   IE    Input  enable   */
  /*  Bit 5:4 DRIVE Strength   */
  /*          0x0   2 mA   */
  /*          0x1   4 mA   */
  /*          0x2   8 mA   */
  /*          0x3   12 mA   */
  /*  Bit 3   PUE   Pull up enable   */
  /*  Bit 2   PDE   Pull down enable   */
  /*  Bit 1   SCHT  Enable schmidt trigger   */
  /*  Bit 0   SLF   Slew rate control 1=fast 0 = slow */

  TEST_SCPI_INPUT("GPIO:SETP:DEV0:GP22 #H56 \r\n");  // init pad to know value
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  1 \r\n");     // Set direction Gp22 as Output DEV0 = Master Pico
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  1 \r\n");     // Set Gp22 = 1
  test_cmd_out("Test 7.12 SCPI GPIO command", "GPIO:GETP:DEV0:GP22?\r\n", "86", &c_test, &buffer);
  test_cmd_out("Test 7.13 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:SETP:DEV0:GP22 #H84 \r\n");  // Disable output and set pull down
  test_cmd_out("Test 7.14 SCPI GPIO command", "GPIO:GETP:DEV0:GP22?\r\n", "132", &c_test, &buffer);
  test_cmd_out("Test 7.15 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n", "0", &c_test, &buffer);  // read 0 due to output disable

  // Test with DEV1 to validate the I2C command
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  1 \r\n");  // Set direction Gp22 as Output DEV1 = Slave1 Pico
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP22  1 \r\n");  // Set Gp22 = 1
  test_cmd_out("Test 7.16 SCPI GPIO command", "GPIO:GETP:DEV1:GP22?\r\n", "86", &c_test, &buffer);
  test_cmd_out("Test 7.17 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:SETP:DEV1:GP22 #H84 \r\n");  // Disable output and set pull down
  test_cmd_out("Test 7.18 SCPI GPIO command", "GPIO:GETP:DEV1:GP22?\r\n", "132", &c_test, &buffer);
  test_cmd_out("Test 7.19 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n", "0", &c_test, &buffer);  // read 0 due to output disable

  // System command test

  TEST_SCPI_INPUT("SYST:BEEP\r\n");                                                            // Generate beep and check if error is raised
  test_cmd_out("Test 8.0 SCPI System command", "SYSTEM:LED:ERR?\r\n", "0", &c_test, &buffer);  // no error after beep
  TEST_SCPI_INPUT("SYST:LED:ERR 1\r\n");
  test_cmd_out("Test 8.1 SCPI System command", "SYSTEM:LED:ERR?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("SYST:LED:ERR 0\r\n");
  test_cmd_out("Test 8.2 SCPI System command", "SYSTEM:LED:ERR?\r\n", "0", &c_test, &buffer);

  // command to read firmware version of all Pico (Master, Slave1,Slave2,Slave3)
  test_cmd_out("Test 8.3 SCPI System command", "SYSTEM:DEV:VERS?\r\n", "\"1.1, 1.1, 1.1, 1.1\"", &c_test, &buffer);

  TEST_SCPI_INPUT("SYST:SLA OFF \r\n");
  test_cmd_out("Test 8.4 SCPI System command", "SYSTEM:SLA?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("SYST:SLA ON \r\n");
  test_cmd_out("Test 8.5 SCPI System command", "SYSTEM:SLA?\r\n", "1", &c_test, &buffer);

  TEST_SCPI_INPUT("SYST:OUT OFF \r\n");
  test_cmd_out("Test 8.6 SCPI System command", "SYSTEM:OUT?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("SYST:OUT ON \r\n");
  test_cmd_out("Test 8.7 SCPI System command", "SYSTEM:OUT?\r\n", "1", &c_test, &buffer);
  sleep_ms(300);  // wait for 5V power to stabilize

  // Command to read status byte of each salves (1,2,3)
  test_cmd_out("Test 8.8 SCPI System command", "SYSTEM:SLA:STA?\r\n", "\"Slave1: 0x0, Slave2: 0x0, Slave3: 0x0\"", &c_test, &buffer);

  // ANALOG Command Validation
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");   // set direction port 0 ouput
  TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");   // set direction port 1 input
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");   // set port 0 to 00
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP8 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP9 1 \r\n");  // set io to output
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1  0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n");
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9  0 \r\n");
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP18 1 \r\n");   // set to output
  TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP19 0 \r\n");   // set to input
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG output
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H40 \r\n");  // Close K2
  TEST_SCPI_INPUT("ANA:DAC:VOLT 3 \r\n");      // Set ouput to 3v
  sleep_ms(250);
  test_cmd_result("Test 9.0: Dac output @ 3Vdc, read ADC0", "ANA:ADC0:VOLT? \r\n", 3.0, "V", 0.4, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  1\r\n");  // Close K13
  test_cmd_result("Test 9.1: Dac output @ 3Vdc, read ADC1", "ANA:ADC1:VOLT? \r\n", 3.0, "V", 0.4, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("ANA:DAC:SAVE  2.5 \r\n");  // Validated with Multimeter after power OFF-ON

  test_cmd_result("Test 9.2: ADC, read VSYS", "ANA:ADC:Vsys? \r\n", 5.0, "V", 0.3, 0.4, &c_test, &buffer);
  test_cmd_result("Test 9.3: ADC, read TEMP", "ANA:ADC:Temp? \r\n", 50, "C", 30, 20, &c_test, &buffer);

  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // Open K2
  sleep_ms(100);                               // let time to relay to stabilize
  test_cmd_result("Test 9.4: PWR, read Bus Volt ", "ANA:PWR:Volt? \r\n", 5, "V", 0.3, 0.2, &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
  sleep_ms(100);                                  // let time to relay to stabilize
  test_cmd_result("Test 9.5: PWR, read Bus Volt ", "ANA:PWR:Volt? \r\n", 0.1, "V", 0.1, 0.2, &c_test, &buffer);
  test_cmd_result("Test 9.6: PWR, read Shunt mV ", "ANA:PWR:Shunt? \r\n", 50, "mV", 10, 10, &c_test, &buffer);
  test_cmd_result("Test 9.7: PWR, read Pmw", "ANA:PWR:Pmw? \r\n", 500, "mW", 200, 200, &c_test, &buffer);
  test_cmd_result("Test 9.8: PWR, read ImA ", "ANA:PWR:Ima? \r\n", 500, "mA", 100, 100, &c_test, &buffer);
  TEST_SCPI_INPUT("ANA:PWR:CAL 500,1000\r\n");  // False Calibration to have big difference
  test_cmd_result("Test 9.9: PWR, read ImA ", "ANA:PWR:Ima? \r\n", 1000, "mA", 200, 200, &c_test, &buffer);
  TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4

  ina219Init();  // reset PWR device and replace with good calibration data

  // EEprom  Command Check
  test_cmd_out("Test 10.0 SCPI EEprom command", "CFG:Read:EEPROM:STR?  'mod_option'\r\n", "DAC,PWR", &c_test, &buffer);
  TEST_SCPI_INPUT("CFG:Write:Eeprom:STR 'mod_option,test_mode'\r\n");
  test_cmd_out("Test 10.1 SCPI EEprom command", "CFG:Read:EEPROM:STR? 'mod_option'\r\n", "TEST_MODE", &c_test, &buffer);
  TEST_SCPI_INPUT("CFG:Write:Eeprom:Default \r\n");
  test_cmd_out("Test 10.2 SCPI EEprom command", "CFG:Read:EEPROM:STR?  'mod_option'\r\n", "DAC,PWR", &c_test, &buffer);
  TEST_SCPI_INPUT("CFG:Read:EEPROM:Full?\r\n");  // No test, just check result returned

  test_cmd_out("Test 10.3 SCPI Error command", "SYSTem:ERRor?\r\n", "0,\"No error\"", &c_test, &buffer);

  // SERIAL command check

  TEST_SCPI_INPUT("COM:INIT:DIS SERIAL\r\n");
  test_cmd_out("Test 11.0 SCPI SERIAL command", "COM:INIT:STAT? SERIAL\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:INIT:ENA SERIAL\r\n");  // Enable SPI
  test_cmd_out("Test 11.1 SCPI SERIAL command", "COM:INIT:STAT? SERIAL\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SERIAL:Baudrate 19200\r\n");
  test_cmd_out("Test 11.2 SCPI SERIAL command", "COM:SERIAL:Baudrate?\r\n", "19199", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SERIAL:Protocol N81\r\n");
  test_cmd_out("Test 11.3 SCPI SERIAL command", "COM:SERIAL:P?\r\n", "\"8N1\"", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SERIAL:Timeout 1000\r\n");
  test_cmd_out("Test 11.4 SCPI SERIAL command", "COM:SERIAL:T?\r\n", "1000", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SERIAL:Handshake ON\r\n");
  test_cmd_out("Test 11.4 SCPI SERIAL command", "COM:SERIAL:H?\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SERIAL:Handshake OFF\r\n");
  test_cmd_out("Test 11.5 SCPI SERIAL command", "COM:SERIAL:H?\r\n", "0", &c_test, &buffer);

  //  Test of Disable command, error -384 is raised for each command
  TEST_SCPI_INPUT("COM:INIT:DIS SERIAL\r\n");
  test_cmd_out("Test 11.6 SCPI SERIAL command", "COM:SERIAL:Write 'TEST\n'\r\n", "", &c_test, &buffer);
  test_cmd_out("Test 11.7 SCPI SERIAL command", "COM:SERIAL:Read? \r\n", "", &c_test, &buffer);

  // SPI  Command Check

  TEST_SCPI_INPUT("COM:INIT:DIS SPI\r\n");
  test_cmd_out("Test 12.0 SCPI SPI command", "COM:INIT:STAT? SPI\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:INIT:ENA SPI\r\n");  // Enable SPI
  test_cmd_out("Test 12.1 SCPI SPI command", "COM:INIT:STAT? SPI\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:D 8 \r\n");  // Set Databits to 8
  test_cmd_out("Test 12.2 SCPI SPI command", "COM:SPI:D?\r\n", "8", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:D 16 \r\n");  // Set Databits to 16
  test_cmd_out("Test 12.3 SCPI SPI command", "COM:SPI:D?\r\n", "16", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:M 0\r\n");
  test_cmd_out("Test 12.4 SCPI SPI command", "COM:SPI:M?\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:M 7\r\n");
  test_cmd_out("Test 12.5 SCPI SPI command", "COM:SPI:M?\r\n", "7", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:M 10\r\n");  // Raise ERROR
  test_cmd_out("Test 12.6 SCPI SPI command", "COM:SPI:M?\r\n", "7", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:Baudrate 1000000\r\n");
  test_cmd_out("Test 12.7 SCPI SPI command", "COM:SPI:Baudrate?\r\n", "1000000", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:CS 12\r\n");
  test_cmd_out("Test 12.8 SCPI SPI command", "COM:SPI:CS?\r\n", "12", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:SPI:D 8 \r\n");  // Set Databits to 8
  TEST_SCPI_INPUT("COM:SPI:CS 3\r\n");  // Raise ERROR
  test_cmd_out("Test 12.9 SCPI SPI command", "COM:SPI:CS?\r\n", "12", &c_test, &buffer);
  test_cmd_out("Test 12.10 SCPI SPI command", "COM:SPI:WRI #H00\r\n", "", &c_test, &buffer);
  test_cmd_out("Test 12.11 SCPI SPI command", "COM:SPI:READ:LEN1?\r\n", "255", &c_test, &buffer);
  test_cmd_out("Test 12.12 SCPI SPI command", "COM:SPI:READ:LEN2? #H55\r\n", "255,255", &c_test, &buffer);

  // I2C command check

  TEST_SCPI_INPUT("COM:INIT:DIS I2C\r\n");
  test_cmd_out("Test 13.0 SCPI I2C command", "COM:INIT:STAT? I2C\r\n", "0", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");  // Enable SPI
  test_cmd_out("Test 13.1 SCPI I2C command", "COM:INIT:STAT? I2C\r\n", "1", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:D 16 \r\n");  // Set Databits to 16
  test_cmd_out("Test 13.2 SCPI I2C command", "COM:I2C:D?\r\n", "16", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:D 8 \r\n");  // Set Databits to 8
  test_cmd_out("Test 13.3 SCPI I2C command", "COM:I2C:D?\r\n", "8", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:Baudrate 2000000\r\n");
  test_cmd_out("Test 13.4 SCPI I2C command", "COM:I2C:Baudrate?\r\n", "2000000", &c_test, &buffer);
  TEST_SCPI_INPUT("COM:I2C:ADDR #H21 \r\n");  // Set Address
  test_cmd_out("Test 13.5 SCPI I2C command", "COM:I2C:ADDR?\r\n", "33", &c_test, &buffer);

  // Check PWM on Selftest board
  TEST_SCPI_INPUT("COM:I2C:WRI 80,0\r\n");    // Set PWM OFF
  TEST_SCPI_INPUT("COM:I2C:WRI 80,1\r\n");    // Set PWM ON
  TEST_SCPI_INPUT("COM:I2C:WRI 81,1\r\n");    // Set PWM Freq to 1Khz
  TEST_SCPI_INPUT("COM:I2C:WRI 81,255\r\n");  // Set PWM Freq to 255Khz
  TEST_SCPI_INPUT("COM:I2C:WRI 80,0\r\n");    // Set PWM OFF

  // No test, because error is raise
  // test_cmd_out("Test 13.6 SCPI I2C command", "COM:I2C:WRI #H00\r\n"," ",&c_test, &buffer);
  // test_cmd_out("Test 13.7 SCPI I2C command", "COM:I2C:READ:LEN1? #H00\r\n","255",&c_test, &buffer);
  // test_cmd_out("Test 13.8 SCPI I2C command", "COM:I2C:READ:LEN2?\r\n","255,255",&c_test, &buffer);

  TEST_SCPI_INPUT("SYST:OUT OFF \r\n");  // turn OFF selftest power

  /** PRINT FINAL REPORT AFTER TEST COMPLETION*/
  fprintf(stdout, "\n\n\t SCPI COMMAND CHECK COMPLETED REPORT \n\n");
  fprintf(stdout, "\t Number of Tests performed:\t%d\r\n", c_test.total);
  fprintf(stdout, "\t Number of Tests PASS:\t\t%d\r\n", c_test.good);
  fprintf(stdout, "\t Number of Tests FAIL:\t\t%d\r\n", c_test.bad);
  fprintf(stdout, "\t Number of Tests ERROR:\t%d\r\n", c_test.error);

  // send result string to serial port
  sprintf(strval, "TEST COMMAND RESULTS: \n NbTotal: %d, NbGood: %d, NbBad: %d, NbError: %d\n", c_test.total, c_test.good, c_test.bad,
          c_test.error);       // build string to return
  uart_puts(UART_ID, strval);  // Send result

  //  // Print all stored failure messages
  if (c_test.bad > 0 || c_test.error > 0)
  {
    TEST_SCPI_INPUT("SYSTEM:LED:ERR ON \r\n");  // turn ON Error led
    fprintf(stdout, "\nStored bad messages:\n");

    print_messages(&buffer);
  }

  sprintf(strval, "TEST COMMAND COMPLETED \n");  // build string to return
  uart_puts(UART_ID, strval);                    // Send string
}

/*! @brief - Function not used during normal execution
 *    The function is used to validate the hardware after parts has been soldered
 *    and pico has firmware programmed
 *
 *   Normally run step by step with debugger
 *   and hardware verified by visual check or by measuring voltage
 */

bool test_ioboard()
{
  int result;

  fprintf(stdout, "Interconnect IO Board Hardware Test\n");  // send message to debug port

  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3\r\n");
  TEST_SCPI_INPUT("ROUT:STATE:OC? OC3 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");
  TEST_SCPI_INPUT("ROUT:STATE:OC? OC3 \r\n");

  TEST_SCPI_INPUT("SYST:BEEP\r\n");

  TEST_SCPI_INPUT("SYST:LED:ERR ON \r\n");
  TEST_SCPI_INPUT("SYST:LED:ERR? \r\n");
  TEST_SCPI_INPUT("SYST:LED:ERR OFF \r\n");

  TEST_SCPI_INPUT("SYST:SLA OFF\r\n");
  TEST_SCPI_INPUT("SYST:SLA?\r\n");
  TEST_SCPI_INPUT("SYST:OUT ON\r\n");
  TEST_SCPI_INPUT("SYST:OUT?\r\n");
  TEST_SCPI_INPUT("SYST:SLA ON\r\n");
  TEST_SCPI_INPUT("SYST:SLA?\r\n");
  TEST_SCPI_INPUT("SYST:OUT OFF\r\n");
  TEST_SCPI_INPUT("SYST:OUT?\r\n");

  // Test for Master PICO
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP11  1 \r\n");  // Beeper ON
  sleep_ms(10);
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP11  0 \r\n");  // Beeper OFF
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP19  1 \r\n");  // Red Led ON
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  1 \r\n");  // Green Led ON (OE)
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP19  0 \r\n");  // Red Led OFF
  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  0 \r\n");  // Green Led OFF (OE)

  TEST_SCPI_INPUT("SYST:LED:ERR ON \r\n");
  TEST_SCPI_INPUT("SYST:LED:ERR OFF \r\n");

  TEST_SCPI_INPUT("ROUT:CLOSE:PWR HPR1 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR2 \r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE:PWR SSR1 \r\n");

  TEST_SCPI_INPUT("ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1 \r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1,LPR2,HPR1,SSR1 \r\n");

  scan_i2c_bus(i2c0);

  // Test for Pico Slave 1
  TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n");  // set direction port 0
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #HAA \r\n");
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H55 \r\n");
  TEST_SCPI_INPUT("DIG:IN:PORT0? \r\n");

  TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n");  // set direction port 1
  TEST_SCPI_INPUT("DIG:OUT:PORT1 #H55 \r\n");
  TEST_SCPI_INPUT("DIG:OUT:PORT1 #HAA \r\n");
  TEST_SCPI_INPUT("DIG:IN:PORT1? \r\n");

  // Test for Pico Slave 2
  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1\r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE (@100)\r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK3\r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE (@300)\r\n");

  // Test for Pico Slave 3
  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK2\r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE (@200)\r\n");
  TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK4\r\n");
  TEST_SCPI_INPUT("ROUT:CLOSE (@400)\r\n");

  sleep_ms(300);
}

/**
 * @def PAGESIZE
 * @brief Size of a single page in the EEPROM.
 *
 * This macro defines the number of bytes in a single page of the 24LC32 EEPROM.
 * It is used to control how data is written and read from the EEPROM.
 */
#define PAGESIZE 32

/**
 * @def EEMODEL
 * @brief Model number for the 24LC32 EEPROM.
 *
 * This macro represents the model number of the EEPROM being used. In this case,
 * it indicates the 24LC32 EEPROM.
 */
#define EEMODEL 32

/**
 * @def RWTESTSIZE
 * @brief Number of bytes to read/write during the EEPROM validation test.
 *
 * This macro defines the size of data blocks to be used for read/write operations
 * during EEPROM testing.
 */
#define RWTESTSIZE 8

/*! @brief - Function not used during normal execution
 *    The function has been used to validate the eeprom read write by byte and by page
 *
 *    @return
 */
bool test_eeprom()
{
  at24cx_dev_t eeprom_1;

  at24cx_writedata_t dt;
  uint16_t ee_add = 0xfe0;

  volatile uint8_t w_data[PAGESIZE];
  volatile uint8_t rdata[PAGESIZE];
  bool err_flag;

  // register eeprom 24lc32
  at24cx_i2c_device_register(&eeprom_1, EEMODEL, I2C_ADDRESS_AT24CX);

  // Check if eeprom_1 is active
  fprintf(stdout, "\neeprom_1 is %s\n", eeprom_1.status ? "detected" : "not detected");
  if (eeprom_1.status == false) return false;

  fprintf(stdout, "\nWrite byte test\n\n");
  for (int i = 0; i < RWTESTSIZE; i++)
  {
    dt.address = ee_add + i;
    dt.data = i;
    w_data[i] = dt.data;  // save value for compare later

    if (at24cx_i2c_byte_write(eeprom_1, dt) == AT24CX_OK)
    {
      fprintf(stdout, "Writing at address 0x%02X: %d\n", dt.address, dt.data);
    }
    else
    {
      fprintf(stdout, "Device write byte error! \n");
      return false;
    }
  }

  fprintf(stdout, "\nRead byte test\n\n");
  for (int i = 0; i < RWTESTSIZE; i++)
  {
    dt.address = ee_add + i;
    if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK)
    {
      fprintf(stdout, "Reading at address 0x%02X: %d\n", dt.address, dt.data);
      rdata[i] = dt.data;
    }
    else
    {
      fprintf(stdout, "Device byte read error!\n");
      err_flag = true;
    }
  }

  // Compare value write to value read
  fprintf(stdout, "\nCompare Write and Read byte test\n\n");
  for (int i = 0; i < RWTESTSIZE; i++)
  {
    if (w_data[i] != rdata[i])
    {
      fprintf(stdout, "Error byte Write-read at address 0x%02X: write value 0x%02X: read value: 0x%02X\n", ee_add + i, w_data[i], rdata[i]);
      err_flag = true;
    }
  }

  fprintf(stdout, "\nWrite page test\n\n");
  dt.address = ee_add;
  for (int i = 0; i < PAGESIZE; i++)
  {
    dt.data_multi[i] = i;
    w_data[i] = dt.data_multi[i];

    fprintf(stdout, "Writing at page data at position 0x%02X: %d\n", dt.address + i, dt.data_multi[i]);
  }

  if (at24cx_i2c_page_write(eeprom_1, dt) == AT24CX_OK)
    fprintf(stdout, "Page Writing at address 0x%02X\n", dt.address);
  else
  {
    fprintf(stdout, "Device page write error!\n");
    return false;
  }

  fprintf(stdout, "\nRead page test\n\n");
  for (int i = 0; i < PAGESIZE; i++)
  {
    dt.address = ee_add + i;

    if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK)
    {
      fprintf(stdout, "Reading at address 0x%02X: %d\n", dt.address, dt.data);
      rdata[i] = dt.data;
    }
    else
    {
      fprintf(stdout, "Device page read error!\n");
      err_flag = true;
      ;
    }
  }

  // Compare value write to value read
  fprintf(stdout, "\nCompare Write and Read page test\n\n");
  for (int i = 0; i < PAGESIZE; i++)
  {
    if (w_data[i] != rdata[i])
    {
      fprintf(stdout, "Error byte Write-read at address 0x%02X: write value 0x%02X: read value: 0x%02X\n", ee_add + i, w_data[i], rdata[i]);
      err_flag = true;
    }
  }

  if (err_flag == true)
    return false;
  else
    return true;
}

/*! @brief - Function not used during normal execution
             function used to validate the onewire write and read
*
* @return int return 0 when completed
*/

int onewiretest()
{
  uint8_t valid;

  fprintf(stdout, "Initializing One-Wire bus\n");

  sleep_ms(10);

  const char* bd_info = "2D4CE282200000CC, 500-1010-020, 000001, J1";
  const char* bd_info2 = "2DC1C38220000059, 500-1010-020, 000001, J2";

  const char* bd_test2 = "2D4CE282200000CC, 12345678,J1";
  const char* bd_test22 = "2DC1C38220000059, ABCDEFGH, J2";

  char* owid = NULL;
  valid = onewire_read_info(&owid, ADDR_TEST, NB_TEST, 1);
  if (valid != 0)
  {
    fprintf(stdout, "\nERROR READ 1-WIRE, error # %d:\n", valid);
    return valid;
  }
  fprintf(stdout, "\nREAD TEST BEFORE: %s\n", owid);

  onewire_write_info(bd_test2, ADDR_TEST);
  onewire_write_info(bd_test22, ADDR_TEST);
  fprintf(stdout, "\nREAD TEST AFTER:\n");
  onewire_read_info(&owid, ADDR_TEST, NB_TEST, 1);

  fprintf(stdout, "\nRESULT TEST:\n");
  SCPI_ResultText(&scpi_context, owid);

  onewire_write_info(bd_info2, ADDR_INFO);
  fprintf(stdout, "\nREAD INFO:\n");
  onewire_read_info(&owid, ADDR_INFO, NB_INFO, 1);
  fprintf(stdout, "\nRESULT INFO:\n");
  SCPI_ResultText(&scpi_context, owid);
  free(owid);
  return 0;
}

/*! @brief - Function not used during normal execution
 *    The test_adc was used to test to get all the measure in same time from the Pico ADC
 *
 *    @return
 */
bool test_adc()
{
  float readv;

  sys_adc_init(ADC_CH_0);  // initialize ADC0
  sys_adc_init(ADC_CH_1);
  // sys_adc_init(ADC_CH_2); /** Connected to OE */
  sys_adc_init(ADC_CH_V);
  sys_adc_init(ADC_CH_T);

  readv = sys_adc_volt(ADC_CH_0);
  fprintf(stdout, "ADC_CH_0: %2.3f V\n", readv);

  readv = sys_adc_volt(ADC_CH_1);
  fprintf(stdout, "ADC_CH_1: %2.3f V\n", readv);

  // readv = sys_adc_volt(ADC_CH_2);
  // fprintf(stdout,"ADC_CH_2: %2.3f V\n", readv);

  readv = sys_adc_vsys();
  fprintf(stdout, "VSYS: %2.3f V\n", readv);

  readv = sys_adc_temp_c();
  fprintf(stdout, "TEMP C: %2.3f C\n", readv);
}

/*! @brief - Function not used during normal execution
 *    The function was used to test to get all the measure in same time from the INA219
 *
 *    @return
 */
void test_ina219(void)
{
  volatile float bus_v, i, p, s;
  ina219Init();

  bus_v = ina219GetBusVoltage() * 0.001;
  fprintf(stdout, "INA219 Bus voltage: %2.3f V\n", bus_v);
  s = ina219GetShuntVoltage() * 10E-3;
  fprintf(stdout, "INA219 Shunt voltage: %2.3f mV\n", s);
  i = ina219GetCurrent_mA();
  fprintf(stdout, "INA219 Current : %2.3f mA\n", i);
  p = ina219GetPower_mW();
  fprintf(stdout, "INA219 power: %2.3f mW\n", p);

  // ina219CalibrateCurrent_mA(i,404.35);
  // i = ina219GetCurrent_mA();
  // fprintf(stdout,"INA219 Cal Current : %2.3f mA\n", i);
}

/*! @brief - Function not used during normal execution
 *    The power_test was used to test the hardware of interconnect IO board
 *    connected to selftest board without using of SCPI command
 *
 *    @param mode Number to define which measurement need to be performed*
 *    @param expect_value   Value expected to be a valid value
 *    @param percentage_lo  Lower deviation from expect_value acceptable to PASS the test
 *    @param percentage_hi  Higher deviation from expect_value acceptable to PASS the test
 *    @return
 */
bool power_test(uint8_t mode, float expect_value, float percentage_lo, float percentage_hi)
{
  int16_t readv;
  uint8_t flag = false;
  float hi_limit, lo_limit;
  char meas[3];

  switch (mode)
  {
    case V:
      readv = ina219GetBusVoltage() * 0.001;
      strcpy(meas, "V");
      flag = true;
      break;
    case I:
      readv = ina219GetCurrent_mA();
      strcpy(meas, "mA");
      flag = true;
      break;
    case P:
      readv = ina219GetPower_mW();
      strcpy(meas, "mW");
      flag = true;
      break;
    case S:
      readv = ina219GetShuntVoltage() * 10E-3;
      strcpy(meas, "mV");
      flag = true;
      break;
  }
  if (flag)
  {  // if a value has been read

    if (expect_value > 0)
    {
      hi_limit = expect_value + (expect_value * percentage_hi / 100);
      lo_limit = expect_value - (expect_value * percentage_lo / 100);
    }
    else
    {
      hi_limit = expect_value + percentage_hi;
      lo_limit = expect_value - percentage_lo;
    }

    if (readv > hi_limit || readv < lo_limit)
    {
      fprintf(stdout, "---> FAIL <---VAL:%d %s, LL:%2.2f, HL:%2.2f  \n", readv, meas, lo_limit, hi_limit);
    }
    else
    {
      fprintf(stdout, "---> PASS <---VAL:%d %s LL:%2.2f, HL:%2.2f  \n", readv, meas, lo_limit, hi_limit);
    }
  }
}

/**
 * @brief Function used to set a value on the DAC device.
 *
 * This function sets the specified voltage value on the MCP4725 DAC device. It is primarily
 * for testing purposes and is not used during normal execution.
 *
 * @param value The voltage value (in volts) to set on the DAC device.
 *
 * @return true if the value was successfully set on the DAC.
 * @return false if an error occurred while setting the DAC value.
 */
bool test_dac(float value)
{
  if (!dev_mcp4725_set(i2c0, MCP4725_ADDR0, value))
  {
    fprintf(stdout, "DAC Error on set MCP4725\n");
    return false;
  }
  fprintf(stdout, "DAC voltage set to: %2.3f V\n", value);
  return true;
}

/*! @brief - Function not used during normal execution
 *    The adc_test was used to test the hardware of interconnect IO board
 *    connected to selftest board without using of SCPI command
 *
 *    @param channel Number to define which measurement channel is required
 *    @param expect_value   Value expected to be a valid value
 *    @param percentage_lo  Lower deviation from expect_value acceptable to PASS the test
 *    @param percentage_hi  Higher deviation from expect_value acceptable to PASS the test
 *    @return
 */
bool adc_test(uint8_t channel, float expect_value, float percentage_lo, float percentage_hi)
{
  float readv;
  uint8_t flag = false;
  float hi_limit, lo_limit;

  switch (channel)
  {
    case 0:
      readv = sys_adc_volt(ADC_CH_0);  // multiply * 2 due to voltage divider
      flag = true;
      break;
    case 1:
      readv = sys_adc_volt(ADC_CH_1);
      flag = true;
      break;
    case 2:
      readv = sys_adc_vsys();
      flag = true;
      break;
    case 4:
      readv = sys_adc_temp_c();
      flag = true;
      break;
  }

  if (flag)
  {  // if a value has been read

    hi_limit = expect_value + (expect_value * percentage_hi / 100);
    lo_limit = expect_value - (expect_value * percentage_lo / 100);

    if (readv > hi_limit || readv < lo_limit)
    {
      fprintf(stdout, "---> FAIL <--- CH: %1d VAL:%2.3f V, LL:%2.3f, HL:%2.3f  \n", channel, readv, lo_limit, hi_limit);
      return false;
    }
    else
    {
      fprintf(stdout, "---> PASS <--- CH: %1d VAL:%2.3f V, LL:%2.3f, HL:%2.3f  \n", channel, readv, lo_limit, hi_limit);
      return true;
    }
  }
  return false;
}

/**
 * @brief the software code below is not used by application but could be used, as example
 *        on how to work with SPI interface. This example was a proof of concept on SPI
 *        command available with a true device (ADXL345).
 *
 *        The SPI command available on Pico SDK are basic and do not set the read bit,
 *        normally defined as bit 7 = 1 for read and bit 7 = 0 for write. The user will
 *        need to set the bit as showed on code below.
 *
 */
void test_spi_adx()
{
  int result;
  fprintf(stdout, "Test of SPI with  ADXL345\r\n");

  TEST_SCPI_INPUT("COM:SPI:D 8 \r\n");      // Set Databits to 8
  TEST_SCPI_INPUT("COM:SPI:M 3\r\n");       // set mode to 3
  TEST_SCPI_INPUT("COM:INIT:ENA SPI\r\n");  // Enable SPI

  TEST_SCPI_INPUT("COM:SPI:WRI  #H00, #H00 \r\n");  // Dummy write to set clock to high as default state
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #H00 \r\n");
  TEST_SCPI_INPUT("COM:SPI:WRI  #H31, #H83 \r\n");  // set Selftest bit + 16g Range
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #HB1 \r\n");   //  Read bit 7 (1) + Register 0x31
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #H80 \r\n");

  // Test Of SPI with true device
  /*
    TEST_SCPI_INPUT("COM:SPI:WRI  #H1e, #H11 \r\n");   // Write bit 7 (0) + Register 0x1e
    TEST_SCPI_INPUT("COM:SPI:WRI  #H1f, #H22 \r\n");   // Write bit 7 (0) + Register 0x1f
    TEST_SCPI_INPUT("COM:SPI:WRI  #H20, #H33 \r\n");   // Write bit 7 (0) + Register 0x20
    TEST_SCPI_INPUT("COM:SPI:WRI  #H21, #H44 \r\n");   // Write bit 7 (0) + Register 0x20

   TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #H9e\r\n");      //  Read bit 7 (1) + Register 0x1e
   TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #H9f\r\n");      //  Read bit 7 (1) + Register 0x1e
   TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #HA0\r\n");      //  Read bit 7 (1) + Register 0x20
   TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #HA1\r\n");      //  Read bit 7 (1) + Register 0x20

  TEST_SCPI_INPUT("COM:SPI:REA:LEN4? #HDe\r\n");    // read multiple byte

  TEST_SCPI_INPUT("COM:SPI:D 16 \r\n"); // Set Databits to 16
  TEST_SCPI_INPUT("COM:SPI:REA:LEN2? #HDe00\r\n");    // read multiple byte Databits  = 16

  */

  TEST_SCPI_INPUT("COM:SPI:WRI  #H2d, #H08 \r\n");  // Write bit 7 (0) + Register 0x2d
  TEST_SCPI_INPUT("COM:SPI:WRI  #H1e, #H00 \r\n");  // Write bit 7 (0) + Register 0x1e
  TEST_SCPI_INPUT("COM:SPI:WRI  #H1f, #H00 \r\n");  // Write bit 7 (0) + Register 0x1f
  TEST_SCPI_INPUT("COM:SPI:WRI  #H20, #H05 \r\n");  // Write bit 7 (0) + Register 0x20

  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #H9e\r\n");  //  Read bit 7 (1) + Register 0x1e
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #HA0\r\n");  //  Read bit 7 (1) + Register 0x20

  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #H80\r\n");  // Read bit 7 (1) + Register 0x00
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #HAc\r\n");  // Read bit 7 (1) + Register 0x2c
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1? #HB0\r\n");  // Read bit 7 (1) + Register 0x30

  while (1)
  {
    output_buffer_clear();  // clear SCPI output result for get only results

    // launch command to read X,Y,Z acceleration
    TEST_SCPI_INPUT("COM:SPI:REA:LEN6? #Hf2\r\n");  // Read bit 7 (1) + Multiple byte bit 6 (1) + register 0x32

    uint8_t dta[6];
    memset(dta, 0, 6);

    // converting string answer located in output buffer to number
    int num = 0;
    size_t ind = 0;
    for (int i = 0; out_buffer[i] != '\0'; i++)
    {
      if (out_buffer[i] == ',' || out_buffer[i] == '\r' || out_buffer[i] == '\n')
      {
        dta[ind] = num;
        num = 0;
        ind++;
      }
      else
      {
        num = num * 10 + (out_buffer[i] - 48);  // if valid number add to the number
      }
    }

    int16_t accelerometer_dta[3];
    // Unpack data
    for (int j = 0; j < 3; ++j)
    {
      accelerometer_dta[j] = (int16_t)dta[2 * j] + (((int16_t)dta[2 * j + 1]) << 8);
      //  fprintf(stdout, "Accelerometer value j:%d, value: 0x%x\r\n",j,accelerometer_dta[j]);
    }

    fprintf(stdout, "Accelerometer X: %d\r\n", accelerometer_dta[0]);
    fprintf(stdout, "Accelerometer Y: %d\r\n", accelerometer_dta[1]);
    fprintf(stdout, "Accelerometer Z: %d\r\n", accelerometer_dta[2]);

    sleep_ms(1000 * 2);
  }
  fprintf(stdout, "End of SPI Test of ADXL345\r\n");
}

/**
 * @brief the software code below is not used by application but could be used, as example
 *        on how to work with I2C interface. This example was a proof of concept on I2C
 *        command available with a true device (ADXL345) connected to interconnect io board

 *
 */
void test_i2c_adx()
{
  volatile int result;
  fprintf(stdout, "Test of I2C with  ADXL345\r\n");

  result = 0xA6 >> 1;

  TEST_SCPI_INPUT("COM:I2C:D 16 \r\n");
  TEST_SCPI_INPUT("COM:I2C:B 200000 \r\n");
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");
  TEST_SCPI_INPUT("COM:I2C:ADDR #H53 \r\n");

  // Test of command for data = word
  TEST_SCPI_INPUT("COM:I2C:WRI  #H1e,#H1234 \r\n");  // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H1e \r\n");    // Check  write-read
  TEST_SCPI_INPUT("COM:I2C:WRI  #H20,#H5678 \r\n");  // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H20 \r\n");    // Check  write-read
  TEST_SCPI_INPUT("COM:I2C:WRI  #H1e \r\n");         // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN2? \r\n");         // Check read only

  // Test of command for data = byte
  TEST_SCPI_INPUT("COM:I2C:D 8 \r\n");
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H00 \r\n");  // Check  write-read, device id = 0xe5

  TEST_SCPI_INPUT("COM:I2C:WRI  #H1e,#H55 \r\n");  // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H1e \r\n");  // Check  write-read
  TEST_SCPI_INPUT("COM:I2C:WRI  #H1f,#H66 \r\n");  // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H1f \r\n");  // Check  write-read
  TEST_SCPI_INPUT("COM:I2C:WRI  #H20,#H77 \r\n");  // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H20 \r\n");  // Check  write-read
  TEST_SCPI_INPUT("COM:I2C:WRI  #H21,#H88 \r\n");  // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H21 \r\n");  // Check  write-read
  TEST_SCPI_INPUT("COM:I2C:WRI  #H1e \r\n");       // Check write only
  TEST_SCPI_INPUT("COM:I2C:REA:LEN4? \r\n");       // Check read only

  // Normal test for device
  TEST_SCPI_INPUT("COM:I2C:WRI  #H2d, #H08 \r\n");  //  Register 0x2d
  TEST_SCPI_INPUT("COM:I2C:WRI  #H1e, #H00 \r\n");  //  Register 0x1e
  TEST_SCPI_INPUT("COM:I2C:WRI #H1f, #H00 \r\n");   //  Register 0x1f
  TEST_SCPI_INPUT("COM:I2C:WRI #H20, #H05 \r\n");   // Register 0x20

  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H2d\r\n");  //   Register 0x1e
  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H20\r\n");  //  Register 0x20

  TEST_SCPI_INPUT("COM:I2C:REA:LEN1? #H00\r\n");  //   Register 0x00

  while (1)
  {
    output_buffer_clear();  // clear SCPI output result for get only results

    // launch command to read X,Y,Z acceleration
    TEST_SCPI_INPUT("COM:I2C:REA:LEN6? #H32\r\n");  // register 0x32

    uint8_t dta[6];
    memset(dta, 0, 6);

    // converting string answer located in output buffer to number
    int num = 0;
    size_t ind = 0;
    for (int i = 0; out_buffer[i] != '\0'; i++)
    {
      if (out_buffer[i] == ',' || out_buffer[i] == '\r' || out_buffer[i] == '\n')
      {
        dta[ind] = num;
        num = 0;
        ind++;
      }
      else
      {
        num = num * 10 + (out_buffer[i] - 48);  // if valid number add to the number
      }
    }

    int16_t accelerometer_dta[3];
    // Unpack data
    for (int j = 0; j < 3; ++j)
    {
      accelerometer_dta[j] = (int16_t)dta[2 * j] + (((int16_t)dta[2 * j + 1]) << 8);
      //  fprintf(stdout, "Accelerometer value j:%d, value: 0x%x\r\n",j,accelerometer_dta[j]);
    }

    fprintf(stdout, "Accelerometer X: %d\r\n", accelerometer_dta[0]);
    fprintf(stdout, "Accelerometer Y: %d\r\n", accelerometer_dta[1]);
    fprintf(stdout, "Accelerometer Z: %d\r\n", accelerometer_dta[2]);

    sleep_ms(1000 * 2);
  }
  fprintf(stdout, "End of I2C Test of ADXL345\r\n");
}

/**
 * @brief the software code below is not used by application but could be used, as example
 *        on how to work with I2C interface. This example was a proof of concept on I2C
 *        command available with a true device (INA219).
 *
 *        Mainly used to validate the reading of 16 bits register using I2C word
 */
void test_i2c_ina219()
{
  int result;

  fprintf(stdout, "Start of I2C Test of INA219\r\n");

  TEST_SCPI_INPUT("COM:I2C:D 8 \r\n");
  TEST_SCPI_INPUT("COM:I2C:B 200000 \r\n");
  TEST_SCPI_INPUT("COM:INIT:ENA I2C\r\n");
  TEST_SCPI_INPUT("COM:I2C:ADDR #H40 \r\n");

  // test using 8 bits
  TEST_SCPI_INPUT("COM:i2C:WRI #H00, #H80, #H00 \r\n");
  TEST_SCPI_INPUT("COM:i2C:REA:LEN2? #H00 \r\n");  // expect  399F

  // test using 16 bits
  TEST_SCPI_INPUT("COM:I2C:D 16 \r\n");
  TEST_SCPI_INPUT("COM:i2C:WRI #H00, #H8000 \r\n");  // good
  TEST_SCPI_INPUT("COM:i2C:REA:LEN1? #H00 \r\n");

  // TEST_SCPI_INPUT("COM:i2C:WRI:REG #H00, #H8000 \r\n"); // no good
  // TEST_SCPI_INPUT("COM:i2C:REA:LEN1? #H00 \r\n");

  TEST_SCPI_INPUT("COM:i2C:WRI #H0080, #H00 \r\n");  // good
  TEST_SCPI_INPUT("COM:i2C:REA:LEN1? #H00 \r\n");

  TEST_SCPI_INPUT("COM:I2C:ADDR #H40 \r\n");
  TEST_SCPI_INPUT("COM:i2C:REA:LEN2? #H00 \r\n");

  fprintf(stdout, "End of I2C Test of INA219\r\n");
}

/**
 * @brief Utility function to help debug of firmware
 *        Button is connected between pin 1 and 4 of CTRL INST connector on the
 *        back of interconnect IO
 *
 *        function exit when button pressed
 */
void wait_button_pressed()
{
  gpio_set_function(1, GPIO_FUNC_SIO);
  gpio_set_dir(1, GPIO_IN);
  gpio_pull_up(1);

  fprintf(stdout, "---------------> Press Button\r\n");

  while (gpio_get(1))
  {
    sleep_ms(100);
  }
}
