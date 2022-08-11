#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "scpi/scpi.h"

#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "include/fts_scpi.h"
#include "include/master.h"



static const uint MAJOR_VERSION = 1;    // Major version of Pico_Master
static const uint MINOR_VERSION = 2;    // Minor version of Pico_Master

static const uint PICO_PORT_ADDRESS = 0x22;  // Pico address where port is sused
static const uint REG_STATUS = 100;  // Register used to report Status

static const uint I2C_BAUDRATE = 100000; // 100 k
static const uint I2C_MASTER_SDA_PIN = 6;
static const uint I2C_MASTER_SCL_PIN = 7;


/*
Portable array-based cyclic FIFO queue.
Copy from Internet
*/

#define MESSAGE_SIZE 64
#define QUEUE_SIZE 64 // set high for development

typedef struct {
char data[MESSAGE_SIZE];
} MESSAGE;

struct {   // global structure
    MESSAGE messages[QUEUE_SIZE];
    int begin;
    int end;
    int current_load;
} queue;

bool enque(MESSAGE *message) {
    if (queue.current_load < QUEUE_SIZE) {
        if (queue.end == QUEUE_SIZE) {
            queue.end = 0;
        }
        queue.messages[queue.end] = *message;
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


bool deque(MESSAGE *message) {
    if (queue.current_load > 0) {
        *message = queue.messages[queue.begin];
        memset(&queue.messages[queue.begin], 0, sizeof(MESSAGE));
        queue.begin = (queue.begin + 1) % QUEUE_SIZE;
        queue.current_load--;
        return true;
    } else {
        return false;
    }
}


static void setup_master() {
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);

    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
}

static void send_master(uint8_t i2c_add,uint8_t cmd, uint16_t wdata)  {

    // Writing to A register
    int count;
    uint8_t buf[3];
    int buflgth;   // contains size of the buffer

    buflgth = 2; 
    buf[0] = cmd;       // command      
    buf[1] = wdata;    // gpio



    count = i2c_write_blocking(i2c1, i2c_add, buf, buflgth, false);
    if (count < 0) {
        puts("Couldn't write Register to slave");
        return;
    }
    printf("MAS: Write at register 0x%02d: %02d\n", buf[0], buf[1]);


    uint8_t ird[2];
    i2c_write_blocking(i2c1, i2c_add, buf, 1, false);
    i2c_read_blocking(i2c1, i2c_add, ird, buflgth-1, false);

        
    printf("MAS:Read Register 0x%02d = %d \r\n", cmd,ird[0]);
        
          
}


int main() {

    MESSAGE rec;
    int result;

	bi_decl(bi_program_description("This is a test binary, including the SCPI library."));

	stdio_init_all();
    init_scpi();


// Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);
  //  stdout_uart_init();
  //  stdio_usb_init();

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);


    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, "UART Connected!\n");
    printf("USB CONNECTED \r\n");

    #define LED_PIN 16
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    setup_master();  // I2C_communication
   

#define TEST_SCPI_INPUT(cmd)  result = SCPI_Input(&scpi_context, cmd, strlen(cmd))

   // TEST_SCPI_INPUT("TEST:CHANnellist (@1!2:3!4,5!6)\r\n");
   
    send_master (PICO_PORT_ADDRESS,01,0x00); // test command 
    send_master (PICO_PORT_ADDRESS,02,0x00); // test command 

    TEST_SCPI_INPUT("*IDN?\r\n"); 
    TEST_SCPI_INPUT("ROUT:CLOSE:EXCL (@120:125)\r\n"); 
    TEST_SCPI_INPUT("ROUT:CLOSE (@100:105)\r\n"); 
    

    
   // TEST_SCPI_INPUT("TEST:CHANnellist (@100:105)\r\n");
   // TEST_SCPI_INPUT("TEST:CHANnellist:EXCL (@120:125)\r\n");

	while (1) {
        printf("Hello, world!\n");
        uart_puts(UART_ID, "Hello UART!\n");
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);

		// Feed single characters into the SCPI library.
		// This is somewhat discouraged, since (according to the docs) it will
		// apparently cause the whole buffer to be re-parsed after each character.
		//* char c = fgetc(stdin);  // TODO: Error handling.
		//*SCPI_Input(&scpi_context, &c, 1);

    while (deque(&rec)) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led
      uart_puts(UART_ID, &rec.data[0]);
      //printf("Pico Master: %s\n",&rec.data[0]);  // send message to serial port
      sleep_ms(5);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
    }
	}
	
    printf("program terminated\r");
}
