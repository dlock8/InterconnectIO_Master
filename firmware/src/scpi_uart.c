/**
 * @file    scpi_uart.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Complete software code library to communicate to the user serial port using
 *          SCPI command
 *
 * @details This software setup and control the communication of the user serial port.
 *          Each SCPI command related to serial port is handled by this software.
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "scpi_uart.h"

/**
 * @brief structure to contents the parameters of the user UART
 *
 */
struct user_com
{
  uart_inst_t* uart_id;  //!< uart id, define as uart0 for user
  uint baudrate;         //!< uart baudrate
  uint8_t parity;        //!< uart parity  Odd, Even of None
  uint8_t data_bits;     //!< uart number of data bits,  expected between 5 to 8
  uint8_t stop_bits;     //!< uart number stop bits, expect to be 1 or 2
  bool handshake;        //!< uart handshake RTS-CTS flag
  bool status;           //!< uart enable or disable.  1 = enable
  uint actual_baud;      //!< uart actual true value of baudrate based on Pico clock
  uint32_t timeout_ms;   //!< uart Timeout for receive character
  uint8_t lastchr;       //!< content last character of a string. Used to detect the last character of the receive string
};

/**
 * @brief Global structure to hold user UART communication parameters.
 *
 * The structure `u_com` contains various configuration settings for the UART interface,
 * including baud rate, parity, data bits, and other communication settings. This structure
 * is initialized with default values.
 */
struct user_com u_com = {
    USER_UART,           //!< Default UART ID.
    DEF_USER_BAUD,       //!< Default baud rate.
    DEF_USER_PARITY,     //!< Default parity setting.
    DEF_USER_BITS,       //!< Default data bits.
    DEF_USER_STOP,       //!< Default stop bits.
    DEF_USER_HANDSHAKE,  //!< Default handshake setting.
    DEF_USER_STATUS,     //!< Default UART status (enabled/disabled).
    DEF_USER_BAUD,       //!< Default actual baud rate.
    DEF_TIMEOUT_MS,      //!< Default timeout for receiving data.
    DEF_LASTCHAR         //!< Default last received character.
};

/**
 * @brief  function to configure the parameters of the user uart based on the data set on the uart structure
 *
 */
void scpi_uart_enable()
{
  // Set up our UART with a basic baud rate.
  uart_init(u_com.uart_id, u_com.baudrate);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(USER_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(USER_RX_PIN, GPIO_FUNC_UART);

  if (u_com.handshake)
  {  // if handshake required
    gpio_set_function(USER_CTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(USER_RTS_PIN, GPIO_FUNC_UART);
  }

  // Actually, we want a different speed
  // The call will return the actual baud rate selected, which will be as close as
  // possible to that requested
  u_com.actual_baud = uart_set_baudrate(u_com.uart_id, u_com.baudrate);

  // Set UART flow control CTS/RTS,
  bool hand_sk = u_com.handshake;
  uart_set_hw_flow(u_com.uart_id, hand_sk, hand_sk);

  // Set our data format
  uart_set_format(u_com.uart_id, u_com.data_bits, u_com.stop_bits, u_com.parity);

  // Turn off FIFO's - we want to do this character by character
  uart_set_fifo_enabled(u_com.uart_id, true);
  u_com.status = 1;  // set flag to indicate of serial port is enabled
}

/**
 * @brief  function to disable the user uart and configure developed pins reserved by UART to normal GPIO, set as input.
 *
 */
void scpi_uart_disable()
{
  // Disable.
  uart_deinit(u_com.uart_id);

  // set pins used for uart to GPIO mode
  gpio_set_function(USER_TX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(USER_RX_PIN, GPIO_FUNC_SIO);
  gpio_set_function(USER_CTS_PIN, GPIO_FUNC_SIO);
  gpio_set_function(USER_RTS_PIN, GPIO_FUNC_SIO);

  bool mode = 0;                     // define GPIO as input
  gpio_set_dir(USER_TX_PIN, mode);   // set pins as input if mode = 0
  gpio_set_dir(USER_RX_PIN, mode);   // set pins as output if mode = 1
  gpio_set_dir(USER_CTS_PIN, mode);  // set pins as input
  gpio_set_dir(USER_RTS_PIN, mode);  // set pins as input

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(u_com.uart_id, false, false);

  // Turn off FIFO's - we want to do this character by character
  uart_set_fifo_enabled(u_com.uart_id, false);

  u_com.status = 0;  // Reset flag to indicate of serial port is disabled
}

/**
 * @brief Function who return if the user uart is enabled or not
 *
 *  @return  True  if uart is enabled
 */
bool scpi_uart_status()
{
  return u_com.status;
}

/**
 * @brief Function to update the baudrate speed on the structure, Baudrate will be updated with function uart enable
 *
 * @param speed Baudrate to save on structure
 */
void scpi_uart_set_baudrate(uint32_t speed)
{
  u_com.baudrate = speed;
  u_com.actual_baud = uart_set_baudrate(u_com.uart_id, u_com.baudrate);
}

/**
 * @brief function to return exact baudrate based on Pico clock and divider
 *
 * @return uint32_t actual exact baudrate
 */
uint32_t scpi_uart_get_baudrate()
{
  return u_com.actual_baud;
}

/**
 * @brief Function to update the timeout time register on the structure, Timeout is used to define the maximum time to wait
 *        for a new character received by uart.
 *
 * @param time  time in milliseconds to wait before raise an error
 */
void scpi_uart_set_timeout(uint32_t time)
{
  u_com.timeout_ms = time;
}

/**
 * @brief function to return the value of Timeout saved on user uart structure.
 *
 * @return uint32_t timeout value in ms
 */
uint32_t scpi_uart_get_timeout()
{
  return u_com.timeout_ms;
}

/**
 * @brief Function to set or reset the uart handshake RTS-CTS. When disable the pins reserved the handshake are set for GPIO
 *
 * @param val True to set handshake
 */
void scpi_uart_set_handshake(bool val)
{
  u_com.handshake = val;
  if (u_com.handshake)
  {  // if handshake required
    gpio_set_function(USER_CTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(USER_RTS_PIN, GPIO_FUNC_UART);
  }
  else
  {
    gpio_set_function(USER_CTS_PIN, GPIO_FUNC_SIO);
    gpio_set_function(USER_RTS_PIN, GPIO_FUNC_SIO);
  }
  // Set UART flow control CTS/RTS according to boolean value
  uart_set_hw_flow(u_com.uart_id, val, val);
}

/**
 * @brief function to return the value of handshake save on uart structure
 *
 * @return bool return if handshake is enabled or disabled
 */
bool scpi_uart_get_handshake()
{
  return u_com.handshake;
}

/**
 * @brief Function to set the uart protocol according to parameter. The order of each parameter is not important
 *
 * @param str  A string of 3 characters representing parity, data bits and stop bits
 * @return uint8_t  return error if one of parameter is not in expected limit.
 */
uint8_t scpi_uart_set_protocol(char* str)
{
  uint8_t err = 0;         //!< variable to contains error number. set to 0 (no error)
  char par;                //!< variable to contents the uppercase of letter
  uint8_t num;             //!< variable who content the numeric value of character number
  uint8_t pay, stp, data;  //!< variable who content the validated value of parity, stop and data bits
  bool p = false;          //!< flag to indicate of parity has been received
  bool s = false;          //!< flag to indicate of stop bits has been received
  bool d = false;          //!< flag to indicate of data bits has been received

  //!< loop to pass over each character of the string to extract the parity, stop bits and data bits.
  for (size_t i = 0; i < 3; i++)
  {
    if (isalpha(str[i]))
    {                         // if character is a letter for parity setting
      par = toupper(str[i]);  // set character to uppercase
      switch (par)
      {
        case 'N':
        {
          pay = UART_PARITY_NONE;
          p = true;
          break;
        }  //!< save number to define none parity
        case 'E':
        {
          pay = UART_PARITY_EVEN;
          p = true;
          break;
        }  //!< save number to define even parity
        case 'O':
        {
          pay = UART_PARITY_ODD;
          p = true;
          break;
        }  //!< save number to define odd parity
        default:
        {
          err = UART_PROT_LETTER_NOVALID;
          break;
        }  //!< set error if character is not what is expected
      }
    }
    if (isdigit(str[i]))
    {                      // if character is a number to set stop or data bits
      num = str[i] - '0';  // convert char to number
      if (num >= 5 && (num <= 8))
      {              // number of data bits is a number between 5 to 8
        data = num;  // save data bits number
        d = true;    // set flag to number received
      }
      else if (num >= 1 && num <= 2)
      {  // stop bits
        stp = num;
        s = true;
      }
      else
      {
        err = UART_PROT_NUM_NOTVALID;
      }  //!< return error if number are not in the expected limit
    }
  }

  if (err == 0)
  {  //!< if no error or  3 characters received
    if (p == true && d == true && s == true)
    {  // if all parameter are valid
      // Set our data format
      u_com.parity = pay;      //!< save parity value on structure
      u_com.data_bits = data;  //!< save data bits value on structure
      u_com.stop_bits = stp;   //!< save stops bits value on structure
      uart_set_format(u_com.uart_id, u_com.data_bits, u_com.stop_bits, u_com.parity);
      fprintf(stdout, "UART Protocol updated to  %3s\r\n", str);
      // uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    }
    else
    {
      err = UART_PROT_MISSING;  //!< if error not already found, define one
    }
  }
  return err;  //!< return error number
}

/**
 * @brief function who return the uart protocol used in communication. The string format is Data, parity, stop
 *
 * @return const char*  protocol string
 */
const char* scpi_uart_get_protocol()
{
  static char pr[4];

  pr[0] = u_com.data_bits + '0';  // return string data bits
  pr[2] = u_com.stop_bits + '0';  // return string stop bits

  switch (u_com.parity)
  {
    case UART_PARITY_NONE:
    {
      pr[1] = 'N';
      break;
    }
    case UART_PARITY_EVEN:
    {
      pr[1] = 'E';
      break;
    }
    case UART_PARITY_ODD:
    {
      pr[1] = 'O';
      break;
    }
    default:
    {
      pr[1] = 'X';
      break;
    }
  }
  pr[3] = '\0';  // add null character

  return pr;  //!< return protocol string
}

/**
 * @brief Utility function to empty the receive buffer before send new string
 *
 */
static void clear_receive_fifo()
{
  // Read and discard any available data in the FIFO
  while (uart_is_readable(u_com.uart_id))
  {
    char data = uart_getc(u_com.uart_id);  // Read and discard data
    printf("Receive fifo clear char: 0x%x\n", data);
  }
}

/**
 * @brief function to send character to uart if space is available on the fifo
 *
 * @param c character to send
 */
static void send_char(char c)
{
  // Wait for UART to be ready for TX
  while (!uart_is_writable(u_com.uart_id))
    ;

  // Send character
  uart_putc(u_com.uart_id, c);
}

/**
 * @brief function to receive char from uart. The char need to be received before timeout expire. End of string character is returned
 *        if timeout occur
 *
 * @param timeout_ms  timeout time in milliseconds
 * @return char received on uart
 */
static char receive_char_with_timeout(uint32_t timeout_ms)
{
  uint32_t start_time = time_us_32();

  while (!uart_is_readable(u_com.uart_id))
  {
    // Check timeout
    if ((time_us_32() - start_time) >= (timeout_ms * 1000))
    {
      return '\0';  // Return null character to indicate timeout
    }
  }

  // Read character and return
  return uart_getc(u_com.uart_id);
}

/**
 * @brief function to send data to uart without checking to receive data. The last character is saved for later.
 *
 * @param dwt   string to write on the uart
 * @return uint8_t Error code to success or error
 */
uint8_t scpi_uart_write_data(char* dwt)
{               // write data only
  int tcr = 0;  // Counter for transmit characters
  if (u_com.status == 0)
  {
    return UART_NOT_ENABLED;
  }  // if serial not enabled

  do
  {
    if (dwt[tcr] != '\0')
    {  // if string is not empty
      send_char(dwt[tcr]);
      fprintf(stdout, "Sent char #%d: 0x%x, %c\n", tcr, dwt[tcr], dwt[tcr]);
      tcr++;  // increment char pointer
    }
  } while (dwt[tcr] != '\0');  // loop until end of string

  u_com.lastchr = dwt[tcr - 1];  // identify the last valid character of string, expect CR or LF
  fprintf(stdout, "lastchar Tx only: 0x%x\n", dwt[tcr - 1]);
  return NOCERR;
}

/**
 * @brief Function to perform UART write and read operations.
 *
 * This function writes data to a UART interface and expects a response,
 * reading the response into the provided read buffer.
 *
 * @param dwt Pointer to the write buffer containing the data to be sent.
 * @param dread Pointer to the read buffer where the response will be stored.
 * @param rsize Size of the read buffer, which indicates the maximum number
 *              of bytes to read from the UART interface.
 *
 * @return uint8_t Error code indicating success (0) or error (non-zero).
 *
 */
uint8_t scpi_uart_write_read_data(char* dwt, char* dread, size_t rsize)
{  // write data, expect answer

  if (u_com.status == 0)
  {
    return UART_NOT_ENABLED;
  }  // if serial not enabled

  clear_receive_fifo();  // clear Rx fifo
  dread[0] = '\0';       // clear receive buffer

  uint32_t start_time = time_us_32();  // used by timeout

  int rtr = 0;  // Counter for received characters
  int tcr = 0;  // Counter for transmit characters

  // transmit loop, could receive character back is available
  do
  {
    if (dwt[tcr] != '\0')
    {
      send_char(dwt[tcr]);
      fprintf(stdout, "Sent char #%d: 0x%x, %c", tcr, dwt[tcr], dwt[tcr]);
      tcr++;
    }

    // read character if is one available
    if (uart_is_readable(u_com.uart_id))
    {                                         // if character received on uart fifo
      dread[rtr] = uart_getc(u_com.uart_id);  // save character on read array
      fprintf(stdout, "  Rcv char #%d: 0x%x, %c", rtr, dread[rtr], dread[rtr]);
      rtr++;
    }
    fprintf(stdout, "\n");     // print line return
  } while (dwt[tcr] != '\0');  // loop until all characters has been sent

  if (tcr > 0)
  {                                // if character send, save last character of the string
    u_com.lastchr = dwt[tcr - 1];  // identify the last valid character of string, expect CR or LF
    fprintf(stdout, "lastchar Tx: 0x%x, Rx:  0x%x\n", dwt[tcr - 1], dread[rtr - 1]);
  }

  if (dread[rtr - 1] != u_com.lastchr)
  {  // if more character to receive
    // receive loop with timeout
    do
    {
      // Receive character with timeout
      char received_char = receive_char_with_timeout(u_com.timeout_ms);
      if (received_char != '\0')
      {
        fprintf(stdout, "Rcv chr #%d: 0x%x, %c\n", rtr, received_char, received_char);
        dread[rtr++] = received_char;  // Store received character in dread buffer
        start_time = time_us_32();     // reset timeout
        if (received_char == u_com.lastchr)
        {
          // printf("Last char Received: 0x%x, %c\n", received_char);
          dread[rtr] = '\0';  // Null-terminate the received string
          return NOCERR;      // Return 1 to indicate success
        }
      }
      else
      {
        fprintf(stdout, "Timeout occurred in Receiver.\n");
        dread[rtr] = '\0';          // Null-terminate the received string
        return UART_RX_TIMEOUT_MS;  // Return 0 to indicate failure due to timeout
      }
    } while ((rtr < rsize - 1) && (time_us_32() - start_time) < (u_com.timeout_ms * 1000));
    dread[rtr++] = '\0';  // Null-terminate the received string
    if (rtr >= rsize)
    {
      fprintf(stdout, "UART Receive buffer overrun, receive string too long\n");
      return UART_BUFFER_FULL;
    }
    else
    {
      fprintf(stdout, "Lastchar never received, waiting for 0x%x\n", u_com.lastchr);
      return UART_LASTCHAR_TIMEOUT_MS;
    }
  }

  return NOCERR;  // return no error
}