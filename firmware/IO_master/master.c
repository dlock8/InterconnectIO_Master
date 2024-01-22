#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/master.h"
#include "include/test.h"
#include "include/functadv.h"
#include "hardware/watchdog.h"
#include "userconfig.h"         // contains Major and Minor version

#include "lib/scpi-parser/libscpi/src/error.c" // added to force X-macro to add on list the case (scpi_user.config.h)

#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "pico_lib2/src/sys/include/sys_adc.h"


// Major an Minor version are located on Cmakelist.txt with command
//set (IO_MASTER_VERSION_MAJOR x)
//set (IO_MASTER_VERSION_MINOR x)

// set default value for each pin
static const uint32_t GPIO_BOOT_MASK = 0b000111110010011111111111100000000;
static const uint32_t GPIO_SET_DIR_MASK = 0b000111110010010111111111100000000;
static const uint32_t GPIO_MASTER_OUT_MASK = 0b000111110010011111111111100000000;


// Messsage queue is used to save the SCPI command received by the serial port.

#define MESSAGE_SIZE 64
#define QUEUE_SIZE 12    // maximum message received and not exectuted


typedef struct {
char data[MESSAGE_SIZE];
} MESSAGE;

struct {   // global structure
    MESSAGE messages[QUEUE_SIZE];
    char size[QUEUE_SIZE];
    int begin;
    int end;
    int current_load;
} queue;

bool enque(MESSAGE *message, char mess_size) {
    if (queue.current_load < QUEUE_SIZE) {
        if (queue.end == QUEUE_SIZE) {
            queue.end = 0;
        }
        queue.messages[queue.end] = *message;
        queue.size[queue.end] = mess_size;
        queue.end++;
        queue.current_load++;
        return true;
    } else {
        return false;
    }
}

void init_queue() {
    queue.begin = 0;
    queue.end = 0;
    queue.current_load = 0;
    memset(&queue.messages[0], 0, QUEUE_SIZE * sizeof(MESSAGE_SIZE));
}


bool deque(MESSAGE *message, char *size) {
    if (queue.current_load > 0) {
        *message = queue.messages[queue.begin];
        *size = queue.size[queue.begin];
        memset(&queue.messages[queue.begin], 0, sizeof(MESSAGE));
        queue.begin = (queue.begin + 1) % QUEUE_SIZE;
        queue.size[queue.current_load] = 0;
        queue.current_load--;
        
        return true;
    } else {
        return false;
    }
}



eep ee;  // Declaration of global EEprom structure

static struct 
{
  MESSAGE rx;    //char data[MESSAGE_SIZE];  // contains character received
  uint8_t ch;    //  character counter
} rxser;

// RX Main communication interrupt handler
void on_uart_rx() {
    MESSAGE rxrec;
    char eol;
    while (uart_is_readable(UART_ID)) {
        uart_read_blocking(UART_ID,&rxser.rx.data[rxser.ch],1); // read one character and save on array
        // Can we send it back?
        if (uart_is_writable(UART_ID)) {
            // send back
            uart_putc(UART_ID, rxser.rx.data[rxser.ch]);  // Send ECHO
           }
        // if line feed received or carriage return
        if (rxser.rx.data[rxser.ch] ==  0x0a || rxser.rx.data[rxser.ch] ==  0x0d) {  // if line feed received
          enque(&rxser.rx,rxser.ch+1); // save received data & size on message queue

        if (rxser.rx.data[rxser.ch] ==  0x0a) { eol = 0x0d;} else {eol = 0x0a;}
        if (uart_is_writable(UART_ID)) {
            // send carriage return or line feed to terminal for be sure to start on a newline
            uart_putc(UART_ID, eol);  // Send character
           }
          rxser.ch=0;  // Message received, clear counter
        } else { 
        rxser.ch++;
        }
    }
}


// Initialisation of the main communication uart

void init_main_com() {
  // Communication UART inititialization. The UART is used to receive SCPI command
  // Set up our UART with a basic baud rate.
 long numval;
 uint8_t valid;

  valid =stringtonumber(ee.cfg.com_ser_speed,&numval);   // read communication speed on EEprom
  if (valid == 0) {    //if value is valid
      uart_init(UART_ID,numval );   // set speed
  } else {
      uart_init(UART_ID,19200);       // if error, set default speed
  }

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Actually, we want a different speed
  // The call will return the actual baud rate selected, which will be as close as
  // possible to that requested
  int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);
  
  // Set UART flow control CTS/RTS, we don't want these, so turn them off
  uart_set_hw_flow(UART_ID, false, false);

  // Set our data format
  uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

  // Turn off FIFO's - we want to do this character by character
  // Turn ON FIFO's - we want to use the fifo
  uart_set_fifo_enabled(UART_ID,false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    rxser.ch = 0; // reset global character counter
    //rxser.data[rxser.ch] = 0;
    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

  // Send information on the USB debug port
  fprintf(stdout,"\r\n Serial Baud rate %d \r\n", actual);
}

// Initialisation of the hardware
// Called by Main program and when SCPI command *RST is received
void Hardware_Default_Setting() {
  uint8_t valid;
  double value;
  bool status;

  reg_info_index_t  reg;
  
  gpio_init_mask(GPIO_BOOT_MASK); // set which lines will be GPIO 
  gpio_set_dir_masked(GPIO_SET_DIR_MASK,GPIO_MASTER_OUT_MASK); // set which lines will be GPIO 

  ina219Init(); // Initialize power function
  adc_init();  // initialize ADC function
  adc_set_temp_sensor_enabled(1); //Enable read of internal temperature sensor
  gpio_init(ADC0);  // ADC0 gpio
  gpio_set_dir(ADC0,GPIO_IN); // set has input
  gpio_disable_pulls(ADC0); // remove pullup and down for accurate reading of ADC0

  gpio_init(ADC1);  // ADC1 gpio
  gpio_set_dir(ADC1,GPIO_IN); // set has input
  gpio_disable_pulls(ADC1); // remove pullup and down for accurate reading of ADC1

  gpio_init(ADC3);  // VSYS gpio
  gpio_set_dir(ADC3,GPIO_IN); // set has input
  gpio_disable_pulls(ADC3); // remove pullup and down for accurate reading of VSYS

  gpio_put(GPIO_LED, 0); // Turn OFF led Error

  valid = Boot_check();   // basic check of internal I2C  and power
  RegBitHdwrErr(BOOT_I2C, valid);  // Set or clear Questionable register based on results

  if (valid == true) {  // if no error on boot Check, validate eeprom
    // Read EEprom  configuration
    int result = cfg_eeprom_read_full(); // read configuration eeprom
    status = (result == 0)? TRUE: FALSE;
    RegBitHdwrErr(EEPROM_ERROR,status);  // Set or clear Questionable register based on results
  }
  
   
  // Check master VSYS voltage. Raise error if value is too high or too low
  value = read_master_adc(3);   // Read VSYS,  Expect 5V
  status = (value > MAX_VSYS_VOLT || value < MIN_VSYS_VOLT)? FALSE:TRUE;
  RegBitHdwrErr(VSYS_OUT,status);  // Set or clear Questionable register based on results
    

  // Check master temperature. Raise error if value is too high
  value = read_master_adc(4);   // Read VSYS,  Expect 5V
  status = (value > MAX_PICO_TEMP)? FALSE:TRUE;
  RegBitHdwrErr(MTEMP_HIGH,status);  // Set or clear Questionable register based on results
 
  // RUN_EN setup
  // We setup the output before the direction for being sure of the RUN_EN= 1 when 
  // we change the pin from input to output.
  // Run_EN =0 disconnect the USB port on the slave, not pratical for debug Pico slaves firmware

  gpio_put(GPIO_RUN, 1); // Set RUN_EN =1, pico slaves are actives
  gpio_set_dir(GPIO_RUN, GPIO_OUT);   // Set pin at output

  long  vnum;

  valid =stringtonumber(ee.cfg.slave_force_run,&vnum);   // read if toogle RUN_EN is enabled
  if (valid == 0) {    //if value is valid
    if (vnum == 0) {  // if permit, toogle RUN_EN to Reset the PIco Slaves
        gpio_put(GPIO_RUN, 0); // Reset PICO Slave 
        fprintf(stdout, "PICO Slave in Reset\r\n");
        sleep_ms(100);
        gpio_put(GPIO_RUN, 1); // Start PICO Slave 
        //gpio_set_dir(GPIO_RUN, GPIO_IN); // Set GPIO in Input for security 
        sleep_ms(100);
    }
  }

  fprintf(stdout,"Hardware Default setting completed \r\n");
  return;
}


// Master Pico Main  program
int main(void) {

  MESSAGE rec;
  char nbchar;
  int result;
  float adcv[4];
  uint16_t ctr;  // counter used for flashing pico led
  uint16_t pulse;  // limit for flashing led frequency
  bool valid;

  eep eed = DEF_EEPROM; // Assign default value to structure eeprom
  pulse = 200;    // slow led flashing frequency

	stdio_init_all();
  init_scpi();
  setup_master();  // Initialize of internal I2C_communication
  Hardware_Default_Setting();
  
  init_main_com();  // Setup serial communication parameter

  init_queue(); // initialise queue for message received

  if (watchdog_caused_reboot()) { 
      pulse = 50;  // fast flashing led to indicate watchdog trig
      RegBitHdwrErr(WATCH_TRIG,FALSE);  // Set Questionable event register based on results
  }

  



//test_dac(2.5);
//test_adc();

fprintf(stdout,"Master Version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);

ctr = 0;
int mess=0;

//test_selftest(); 

test_command(); 
 
 while (1) {  // infinite loop, waiting for SCPI command from serial port

    watchdog_update(); /** refresh watchdog */
    sleep_ms(10);
    ctr++;
    mess++;
   
    /** Flashing led */
    if (ctr > pulse) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF Pico board led
      sleep_ms(200);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON Pico board led
      ctr = 0;
    }
    /** Heartbeat message on debug port*/
   if (mess > 1500) {
      fprintf(stdout,"Heartbeat Master, version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);
      mess = 0;
   }

  
  // loop to execute  SCPI command when char received by interrupt
    while (deque(&rec, &nbchar)) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led to show message reading

    //result = SCPI_Input(&scpi_context, cmd, strlen(cmd))
      result = SCPI_Input(&scpi_context, &rec.data[0], nbchar); // send command to SCPI parser
      printf("SCPI Command: %s\n",&rec.data[0]);  // send message to debug port
      sleep_ms(50);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
    }
  }

  fprintf(stdout,"program terminated\r"); /**Never pass by here*/
}
