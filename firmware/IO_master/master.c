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
#include "userconfig.h"


// Major an Minor version are located on Cmakelist.txt with command
//set (IO_MASTER_VERSION_MAJOR x)
//set (IO_MASTER_VERSION_MINOR x)

// set default value for each pin
static const uint32_t GPIO_BOOT_MASK = 0b000111110010011111111111100000000;
static const uint32_t GPIO_SET_DIR_MASK = 0b000111110010010111111111100000000;
static const uint32_t GPIO_MASTER_OUT_MASK = 0b000111110010011111111111100000000;


// Messsage queue is used to save the SCPI command received by the serial port.

#define MESSAGE_SIZE 64
#define QUEUE_SIZE 8    // maximum message received and not exectuted


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




static struct 
{
  MESSAGE rx;    //char data[MESSAGE_SIZE];  // contains character received
  uint8_t ch;              //  character counter
} rxser;

// RX interrupt handler
void on_uart_rx() {
    MESSAGE rxrec;
    char eol;
    while (uart_is_readable(UART_ID)) {
        
        //ch = uart_getc(UART_ID);
        
        //uart_read_blocking(UART_ID,&rxser.rxed[rxser.ch],1); // read one character and save on array
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


// Initialisation of the hardware
// Called by Main program and when SCPI command *RST is received
void Hardware_Default_Setting() {
  
  gpio_init_mask(GPIO_BOOT_MASK); // set which lines will be GPIO 
  gpio_set_dir_masked(GPIO_SET_DIR_MASK,GPIO_MASTER_OUT_MASK); // set which lines will be GPIO 

  // Set up our UART with the required speed.
  uart_init(UART_ID, BAUD_RATE);

  adc_init();  // initialize ADC function
  adc_set_temp_sensor_enabled(1); //Enable read of internal temperature sensor
  gpio_init(29);  // VSYS gpio
  gpio_set_dir(29,GPIO_IN); // set has input
  gpio_disable_pulls(29); // remove pullup and down for accurate reading of VSYS
  
  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // RUN_EN setup
  // We setup the output before the direction for being sure of the RUN_EN= 1 when 
  // we change the pin from input to output.
  // Run_EN =0 disconnect the USB port on the slave
  
  gpio_put(GPIO_RUN, 1); // Set RUN_EN =1
  gpio_set_dir(GPIO_RUN, GPIO_OUT);   // Set pin at output
/*
  gpio_put(GPIO_RUN, 0); // Reset PICO Slave 
  fprintf(stdout, "PICO Slave in Reset\r\n");
  sleep_ms(100);
  gpio_put(GPIO_RUN, 1); // Start PICO Slave 
  gpio_set_dir(GPIO_RUN, GPIO_IN); // Set GPIO in Input for security 
  sleep_ms(100);
 */


  // Communication UART inititializayion. Thr UART is used to receive SCPI command
  // Set up our UART with a basic baud rate.
  uart_init(UART_ID, 115200);

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

    rxser.ch = 0; // reset character counter
    //rxser.data[rxser.ch] = 0;
    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

  // Setup I2C used by the internal bus to commincate with the slave devices
  setup_master();  // I2C_communication

  // Send information on the USB debug port
  fprintf(stdout,"Serial Baud rate %d \r\n", actual);

  fprintf(stdout,"Hardware Default setting completed \r\n");
  return;
}

// Partial of hardware verification
// Called by Main program and when SCPI command *TST? is received
bool Selftest() {
  fprintf(stdout,"Selftest execute \r\n");

}


 int main() {

  MESSAGE rec;
  char nbchar;
  int result;
  float adcv[4];
  uint16_t ctr;  // counter used for flashing pico led
  uint16_t pulse;  // limit for flashing led frequency
  
 
  pulse = 200;    // slow led flashing frequency

	stdio_init_all();
  init_scpi();
  Hardware_Default_Setting();
  init_queue(); // initialise queue for message received

  if (watchdog_caused_reboot()) { 
      //status.watch = 1;
      pulse = 50;  // fast flashing led to indicate watchdog trig

  }

  
/*  // to debug the problem of error number
    int errnum;
    errnum = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;  
    fprintf(stdout,"Error number: %02d  \r\n",errnum );
    SCPI_ErrorPush(context, errnum);
*/

//#include "pico_lib2/src/sys/include/sys_i2c.h"

#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "pico_lib2/src/sys/include/sys_adc.h"


// #define TEST_SCPI_INPUT(cmd)  result = SCPI_Input(&scpi_context, cmd, strlen(cmd))

//test_eeprom();
//test_ioboard();  // Check io board function

test_selftest(); 

/*
#define TEST_SCPI_INPUT(cmd)  result = SCPI_Input(&scpi_context, cmd, strlen(cmd))
TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP28 1 \r\n");  //Get Gp22 direction
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  1 \r\n");  //Set Gp22 as output
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  0 \r\n");  //Set Gp22 as output
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  1 \r\n");  //Set Gp22 as output
*/


/*
read_int_ADC(adcv);
setup_ADC(1);
read_ADC(adcv);
sleep_ms(10);
scan_i2c_bus();
*/


//if (test_eeprom()) fprintf(stdout, "\n ---> ERROR ON EEPROM VALIDATION \n");
//test_ina219();

test_dac(2.5);
test_adc();

uint16_t rdata;

uint8_t buffer[2];

	buffer[0] = 81;
	buffer[1] = 0x55;

//  GPIO:DIRection:DEVice0:GP22 1   --> Set gpio 22 on Device 0 (Master) to direction out (1 = out) 
//  GPIO:Out:DEVice1:GP8 0  




/*
volatile float busv,i,p,s;
ina219Init();

busv = ina219GetBusVoltage() * 0.001;
fprintf(stdout,"INA219 Bus voltage: %2.3f V\n", busv);
s = ina219GetShuntVoltage() * 10E-3;
fprintf(stdout,"INA219 Shunt voltage: %2.3f mV\n", s);
i = ina219GetCurrent_mA();
fprintf(stdout,"INA219 Current : %2.3f mA\n", i);
p = ina219GetPower_mW();
fprintf(stdout,"INA219 power: %2.3f mW\n", p);

//ina219CalibrateCurrent_mA(i,404.35);

// i = ina219GetCurrent_mA();
// fprintf(stdout,"INA219 Cal Current : %2.3f mA\n", i);

*/


//sys_i2c_wbuf(i2c0, 0x21, buffer, sizeof(buffer)) == sizeof(buffer);
//TEST_SCPI_INPUT("DIG:OUT:PORT0 #H55 \r\n"); 

//send_master(0x21,81, 0x01, &rdata);



//read_ADC(adcv);

// fprintf(stdout,"Master Version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);

  fprintf(stdout,"Master Version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);

  ctr = 0;
  int mess=0;

  //uart_puts(UART_ID, "*IDN?\n");
  //uart_puts(UART_ID, "*IDN?\n");
 // uart_puts(UART_ID, "UART message 2\n");
  //uart_puts(UART_ID, "UART message 3\n");
  //uart_puts(UART_ID, "SYST:VERS?\n");


 
  while (1) {  // infinite loop, waiting for SCPI command from serial port

    watchdog_update();
    sleep_ms(10);
    ctr++;
    mess++;
   

    if (ctr > pulse) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led
      sleep_ms(200);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
      ctr = 0;
    }
   if (mess > 1500) {
      //printf("i2c add: 0x%02x\n", context.i2c_add); // for debug only
      fprintf(stdout,"Heartbeat Master, version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);
      mess = 0;
   }

  
  // loop to execute  SCPI command.
    while (deque(&rec, &nbchar)) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led

    //result = SCPI_Input(&scpi_context, cmd, strlen(cmd))
    result = SCPI_Input(&scpi_context, &rec.data[0], nbchar);
   // result = SCPI_Input(&scpi_context, "*IDNA?\n", 7);

      printf("SCPI Command: %s\n",&rec.data[0]);  // send message to debug port
      sleep_ms(50);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
    }
  }

       

		// Feed single characters into the SCPI library.
		// This is somewhat discouraged, since (according to the docs) it will
		// apparently cause the whole buffer to be re-parsed after each character.
		//* char c = fgetc(stdin);  // TODO: Error handling.
		//*SCPI_Input(&scpi_context, &c, 1);

	
    fprintf(stdout,"program terminated\r");
}
