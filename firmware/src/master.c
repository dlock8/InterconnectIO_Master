/**
 * @file    master.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Main loop to control the Pico Master Device
 *
 * @details The Master Pico is the main controller for the InterconnectIO Box.
 * The master accept SCPI command the serial port and execute the command.
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include "include/master.h"

#include <stdio.h>
#include <stdlib.h>

#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#include "include/fts_scpi.h"
#include "include/functadv.h"
#include "include/i2c_com.h"
#include "include/test.h"
#include "lib/scpi-parser/libscpi/src/error.c"  // added to force X-macro to add on list the case (scpi_user.config.h)
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "pico_lib2/src/dev/dev_ds2431/dev_ds2431.h"
#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/sys/include/sys_adc.h"
#include "userconfig.h"  // contains Major and Minor version

// Major an Minor version are located on Cmakelist.txt with command
// set (IO_MASTER_VERSION_MAJOR x)
// set (IO_MASTER_VERSION_MINOR x)

// set default value for each pin
// if boot mask bit = 1, the pin will be initialized as GPIO
static const uint32_t GPIO_BOOT_MASK = 0b000111110010011111111110000000011;
// if direction mask bit = 1, the pin will be set to input or output
static const uint32_t GPIO_SET_DIR_MASK = 0b000111110010010111111110000000011;
// master set GPIO bit 1: Output, 0:Input
static const uint32_t GPIO_MASTER_OUT_MASK = 0b000111110010011111111110000000011;

// Message queue is used to save the SCPI command received by the serial port.

#define MESSAGE_SIZE 92  //!<  Maximum size of message to be send on the USB port for debug
#define QUEUE_SIZE 12    //!<  Maximum message received and not processed

/**
 * @struct  MESSAGE
 * @brief Structure who contains the message to be sent on debug port
 *
 */
typedef struct
{
  char data[MESSAGE_SIZE];  //!< Array of messages
} MESSAGE;

/**
 * @brief Global circular queue for storing messages.
 *
 * This variable manages a circular queue that holds messages and tracks their
 * sizes. It uses indices to mark the beginning and end of the queue, and keeps
 * track of the current number of messages (load).
 */
struct
{                                // Global structure for circular queue
  MESSAGE messages[QUEUE_SIZE];  //!< Array of messages stored in the queue.
  char size[QUEUE_SIZE];         //!< Array containing sizes of each message in the
                                 //!< queue.
  int begin;                     //!< Index of the first message in the queue.
  int end;                       //!< Index where the next message will be added.
  int current_load;              //!< Current number of messages in the queue.
} queue;

/**
 * @brief Adds a message to the queue.
 *
 * This function enqueues a message into a circular queue if there is space
 * available. It updates the queue's `end` index and `current_load`, managing
 * wrap-around when the queue reaches its maximum size.
 *
 * @param message Pointer to the message to be enqueued.
 * @param mess_size Size of the message to be enqueued.
 *
 * @return true if the message was successfully added to the queue.
 * @return false if the queue is full and the message could not be added.
 */
bool enque(MESSAGE* message, char mess_size)
{
  if (queue.current_load < QUEUE_SIZE)
  {
    if (queue.end == QUEUE_SIZE)
    {
      queue.end = 0;  // Wrap around if the queue end reaches the max size
    }
    queue.messages[queue.end] = *message;  // Add the message to the queue
    queue.size[queue.end] = mess_size;     // Store the message size
    queue.end++;                           // Move to the next position
    queue.current_load++;                  // Increment the current load
    return true;                           // Return success
  }
  else
  {
    return false;  // Queue is full, return failure
  }
}

/**
 * @brief Initializes the circular queue.
 *
 * This function sets the `begin`, `end`, and `current_load` indices of the
 * queue to zero, indicating that the queue is empty. It also clears the message
 * storage by setting all entries to zero.
 */
void init_queue()
{
  queue.begin = 0;
  queue.end = 0;
  queue.current_load = 0;
  memset(&queue.messages[0], 0, QUEUE_SIZE * sizeof(MESSAGE_SIZE));
}

/**
 * @brief Removes a message from the front of the queue.
 *
 * This function dequeues a message from the circular queue if there is at least
 * one message present. It retrieves the message and its size, resets the
 * corresponding storage, and updates the `begin` index to point to the next
 * message.
 *
 * @param message Pointer to the structure where the dequeued message will be
 * stored.
 * @param size Pointer to a variable where the size of the dequeued message will
 * be stored.
 *
 * @return true if the message was successfully removed from the queue; false if
 * the queue is empty.
 */
bool deque(MESSAGE* message, char* size)
{
  if (queue.current_load > 0)
  {
    *message = queue.messages[queue.begin];
    *size = queue.size[queue.begin];
    memset(&queue.messages[queue.begin], 0, sizeof(MESSAGE));
    queue.begin = (queue.begin + 1) % QUEUE_SIZE;
    queue.size[queue.current_load] = 0;
    queue.current_load--;

    return true;
  }
  else
  {
    return false;
  }
}

/**
 * @brief Structure for handling received serial data.
 *
 * This static structure holds information related to the
 * received messages through the serial interface. It includes
 * the received message and a character counter to track the
 * number of characters received.
 */
static struct
{
  MESSAGE rx;  ///< The received message containing character data.
  uint8_t ch;  ///< Character counter to track the number of characters received.
} rxser;       ///< Global instance for handling received serial data

eep ee;  ///< Global variable representing the EEPROM data

/**
 * @brief RX Main communication interrupt handler
 *
 */
void on_uart_rx()
{
  char eol;
  while (uart_is_readable(UART_ID))
  {
    uart_read_blocking(UART_ID, &rxser.rx.data[rxser.ch],
                       1);  // read one character and save on array
    // Can we send it back?
    if (uart_is_writable(UART_ID))
    {
      // send back
      uart_putc(UART_ID, rxser.rx.data[rxser.ch]);  // Send ECHO
    }
    // if line feed received or carriage return
    if (rxser.rx.data[rxser.ch] == 0x0a || rxser.rx.data[rxser.ch] == 0x0d)
    {  // if line feed received
      enque(&rxser.rx,
            rxser.ch + 1);  // save received data & size on message queue

      if (rxser.rx.data[rxser.ch] == 0x0a)
      {
        eol = 0x0d;
      }
      else
      {
        eol = 0x0a;
      }
      if (uart_is_writable(UART_ID))
      {
        // send carriage return or line feed to terminal for be sure to
        // start on a newline
        uart_putc(UART_ID, eol);  // Send character
      }
      rxser.ch = 0;  // Message received, clear counter
    }
    else
    {
      rxser.ch++;
    }
  }
}
/**
 * @brief 
 * 
 */

/**
 * @brief Initialisation of the main communication uart (serial port)
 *        the SCPI command are received by the serial port.
 *        Baud rate is read from the Configuration EEPROM
 *        In case of error, default baud rate will be use
 *
 * @return int actual Baudrate
 *
 */
int init_main_com()
{
  // Communication UART initialization. The UART is used to receive SCPI
  // command Set up our UART with a basic baud rate.
  long numval;
  uint8_t valid;

  valid = stringtonumber(ee.cfg.com_ser_speed, sizeof(ee.cfg.com_ser_speed),&numval);  // read communication speed on EEprom
  if (valid != 0)
  {
    numval = 115200;
  }  // if value is not valid use default speed

  uart_init(UART_ID, numval);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Actually, we want a different speed
  // The call will return the actual baud rate selected, which will be as
  // close as possible to that requested
  int actual = uart_set_baudrate(UART_ID, numval);

  // Enable stdio (printf) over selected uart, good for debug
  // stdio_uart_init_full(UART_ID,numval,UART_TX_PIN,UART_RX_PIN);

  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(UART_ID, false, false);

  // Set our data format
  uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

  // Turn off FIFO's - we want to do this character by character
  // Turn ON FIFO's - we want to use the fifo
  uart_set_fifo_enabled(UART_ID, false);

  // Set up a RX interrupt
  // We need to set up the handler first
  // Select correct interrupt for the UART we are using
  int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

  // And set up and enable the interrupt handlers
  irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
  irq_set_enabled(UART_IRQ, true);

  // Now enable the UART to send interrupts - RX only
  uart_set_irq_enables(UART_ID, true, false);

  rxser.ch = 0;  // reset global character counter

  return actual;  // return actual Baudrate
}
/**
 * @brief Initializes the hardware to default settings.
 *
 * This function is called by the main program and when the SCPI command
 * *RST is received. It sets up the necessary hardware configurations to
 * ensure that the system starts in a known state.
 *
 * @param None
 *
 * @return None
 */
void Hardware_Default_Setting()
{
  uint8_t valid;
  double value;
  bool status;

  reg_info_index_t reg;

  gpio_init_mask(GPIO_BOOT_MASK);  // set which lines will be GPIO
  gpio_set_dir_masked(GPIO_SET_DIR_MASK,
                      GPIO_MASTER_OUT_MASK);  // set which lines will be GPIO

  ina219Init();                    // Initialize power function
  adc_init();                      // initialize ADC function
  adc_set_temp_sensor_enabled(1);  // Enable read of internal temperature sensor
  gpio_init(ADC0);                 // ADC0 gpio
  gpio_set_dir(ADC0, GPIO_IN);     // set has input
  gpio_disable_pulls(ADC0);        // remove pullup and down for accurate reading of ADC0

  gpio_init(ADC1);              // ADC1 gpio
  gpio_set_dir(ADC1, GPIO_IN);  // set has input
  gpio_disable_pulls(ADC1);     // remove pullup and down for accurate reading of ADC1

  gpio_init(ADC3);              // VSYS gpio
  gpio_set_dir(ADC3, GPIO_IN);  // set has input
  gpio_disable_pulls(ADC3);     // remove pullup and down for accurate reading of VSYS

  gpio_put(GPIO_LED, 0);  // Turn OFF led Error

  valid = Boot_check();  // basic check of internal I2C  and power
  RegBitHdwrErr(BOOT_I2C,
                valid);  // Set or clear Questionable register based on results

  if (valid == true)
  {  // if no error on boot Check, validate eeprom
    // Read EEprom  configuration
    // cfg_eeprom_write_default();  // if new parameter added, write on
    // eeprom
    int result = cfg_eeprom_read_full();  // read configuration eeprom
    status = (result == 0) ? TRUE : FALSE;
    RegBitHdwrErr(EEPROM_ERROR,
                  status);  // Set or clear Questionable register based on results
  }

  // Check master VSYS voltage. Raise error if value is too high or too low
  value = read_master_adc(3);  // Read VSYS,  Expect 5V
  status = (value > MAX_VSYS_VOLT || value < MIN_VSYS_VOLT) ? FALSE : TRUE;
  RegBitHdwrErr(VSYS_OUT, status);  // Set or clear Questionable register based on results

  // Check master temperature. Raise error if value is too high
  value = read_master_adc(4);  // Read VSYS,  Expect 5V
  status = (value > MAX_PICO_TEMP) ? FALSE : TRUE;
  RegBitHdwrErr(MTEMP_HIGH, status);  // Set or clear Questionable register based on results

  // RUN_EN setup
  // We setup the output before the direction for being sure of the RUN_EN= 1
  // when we change the pin from input to output. Run_EN =0 disconnect the USB
  // port on the slave, not practical for debug Pico slaves firmware

  gpio_put(GPIO_RUN, 1);             // Set RUN_EN =1, pico slaves are actives
  gpio_set_dir(GPIO_RUN, GPIO_OUT);  // Set pin at output

  long v_num;

  valid = stringtonumber(ee.cfg.slave_force_run, sizeof(ee.cfg.slave_force_run),
                         &v_num);  // read if toggle RUN_EN is enabled
  if (valid == 0)
  {  // if value is valid
    if (v_num == 0)
    {                         // if permit, toggle RUN_EN to Reset the PIco Slaves
      gpio_put(GPIO_RUN, 0);  // Reset PICO Slave
      fprintf(stdout, "PICO Slave in Reset\r\n");
      sleep_ms(100);
      gpio_put(GPIO_RUN, 1);  // Start PICO Slave
      sleep_ms(100);
    }
  }

  // fprintf(stdout,"Hardware Default setting completed \r\n");
  return;
}

/**
 * @brief Firmware for the Master Pico device running on interconnectIO board
 *
 * @return int
 */
int main(void)
{
  MESSAGE rec;
  char nb_char;
  int result;
  int serspeed;
  float adcv[4];   //
  uint16_t ctr;    // counter used for flashing pico led
  uint16_t pulse;  // limit for flashing led frequency
  bool valid;

  eep eed = DEF_EEPROM;  // Assign default value to structure eeprom
  pulse = 200;           // slow led flashing frequency

  stdio_init_all();

  // Required only if want to use serial ports as stdio
 // stdio_uart_init_full(UART_ID,PICO_DEFAULT_UART_BAUD_RATE,PICO_DEFAULT_UART_TX_PIN,PICO_DEFAULT_UART_RX_PIN);

  init_scpi();
  setup_master();  // Initialize of internal I2C_communication

  Hardware_Default_Setting();

  serspeed =init_main_com();  // Setup serial communication parameter


  uart_puts(UART_ID, "FTS> ");  // Send ready messages
  fprintf(stdout, "Master Version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);

  valid = watchdog_caused_reboot();  // Check if reboot come from watchdog
  if (valid)
  {
    pulse = 50;  // fast flashing led to indicate watchdog trig
    RegBitHdwrErr(WATCH_TRIG,
                  FALSE);  // Set Questionable event register based on results
  }

  watchdog_enable(WATCHDOG_TIMEOUT,
                  1);  // Enable the watchdog with the timeout and auto-reset

  init_queue();  // initialise queue for SCPI message received


  rxser.ch = 0;  // reset global character counter
  ctr = 0;
  int mess = 0;
  bool led_on = false;

  while (1)
  {  // infinite loop, waiting for SCPI command from serial port

    watchdog_update(); /** refresh watchdog */
    sleep_ms(10);
    ctr++; /** counter for the heartbeat led */
    mess++;

 
 

    /** Flashing led */
    if (ctr > pulse)
    {
      led_on = !led_on;                        // Toggle the LED state
      gpio_put(PICO_DEFAULT_LED_PIN, led_on);  // Turn ON or OFF Pico board led
      ctr = 0;                                 // reset the counter
    }
    /** Heartbeat message on debug port*/
    if (mess > 1500)
    {
<<<<<<< HEAD
      fprintf(stdout, "Heartbeat Master,Baudrate: %d, version: %d.%d\n", serspeed,IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);
=======
      fprintf(stdout, "Heartbeat Master, version: %d.%d\n\r", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);
>>>>>>> f06e9b7 (build with Visual Code pico extension and with sdk 2.1.0)
      mess = 0;
    }

    // loop to execute  SCPI command when char received by interrupt
    while (deque(&rec, &nb_char))
    {
      watchdog_update(); /** refresh watchdog */
      gpio_put(PICO_DEFAULT_LED_PIN,0);  // Turn OFF board led to show message reading

<<<<<<< HEAD
      result = SCPI_Input(&scpi_context, &rec.data[0], nb_char);  // send command to SCPI parser
      fprintf(stdout, "SCPI Command: %s\n",&rec.data[0]);  // send message to debug port
=======
      result = SCPI_Input(&scpi_context, &rec.data[0],
                          nb_char);  // send command to SCPI parser
      fprintf(stdout, "SCPI Command: %s\n\r",
              &rec.data[0]);  // send message to debug port
>>>>>>> f06e9b7 (build with Visual Code pico extension and with sdk 2.1.0)
      sleep_ms(50);
      gpio_put(PICO_DEFAULT_LED_PIN, 1);  // Turn ON board led
    }
  }

  fprintf(stdout, "program terminated\r"); /**Never pass by here*/
}
