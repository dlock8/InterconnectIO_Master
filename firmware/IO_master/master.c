#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/master.h"
#include "userconfig.h"





// Major an Minor version are located on Cmakelist.txt with command
//set (IO_MASTER_VERSION_MAJOR x)
//set (IO_MASTER_VERSION_MINOR x)

// set default value for each pin
static const uint32_t GPIO_BOOT_MASK = 0b000011110010011111111111100000000;
static const uint32_t GPIO_SET_DIR_MASK = 0b000011110010011111111111100000000;
static const uint32_t GPIO_MASTER_OUT_MASK = 0b000011110010011111111111100000000;




int main() {


   // MESSAGE rec;
    int result;

	//bi_decl(bi_program_description("This is a test binary, including the SCPI library."));

  gpio_init_mask(GPIO_BOOT_MASK); // set which lines will be GPIO 
  gpio_set_dir_masked(GPIO_SET_DIR_MASK,GPIO_MASTER_OUT_MASK); // set which lines will be GPIO 
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
    fprintf(stdout,"USB CONNECTED \r\n");

    #define LED_PIN 16
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN,GPIO_OUT);

    gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led

    setup_master();  // I2C_communication
   
/*  // to debug the problem of error number
    int errnum;
    errnum = SCPI_ERROR_ILLEGAL_PARAMETER_VALUE;  
    fprintf(stdout,"Error number: %02d  \r\n",errnum );
    SCPI_ErrorPush(context, errnum);
*/


 fprintf(stdout,"Master Version: %d.%d\n", IO_MASTER_VERSION_MAJOR, IO_MASTER_VERSION_MINOR);

#define TEST_SCPI_INPUT(cmd)  result = SCPI_Input(&scpi_context, cmd, strlen(cmd))

   // TEST_SCPI_INPUT("TEST:CHANnellist (@1!2:3!4,5!6)\r\n");
   
 //   send_master (PICO_PORT_ADDRESS,01,0x00); // test command 
    //send_master (PICO_PORT_ADDRESS,02,0x00); // test command 


   TEST_SCPI_INPUT("*IDN?\r\n"); 


  TEST_SCPI_INPUT("SYST:LED:ERR ON \r\n");
  TEST_SCPI_INPUT("SYST:LED:ERR OFF \r\n");
//TEST_SCPI_INPUT("SYST:VERS? \r\n");

//  TEST_SCPI_INPUT("SYST:DEV:VERS? \r\n");
//   TEST_SCPI_INPUT("SYST:BEEP \r\n");

//TEST_SCPI_INPUT("DIG:DIR:PORT1:BIT1 0\r\n"); 
//TEST_SCPI_INPUT("ROUT:OPEN (@70)\r\n"); 
//TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR5 \r\n");

//TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1,LPR2,HPR1,SSD1 \r\n");
//TEST_SCPI_INPUT("ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSD1 \r\n");
//TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1,LPR2,HPR1,SSD1 \r\n");
//TEST_SCPI_INPUT("ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSD1 \r\n");
//TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR2,HPR1 \r\n");
//TEST_SCPI_INPUT("ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSD1 \r\n");

 // TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n"); // set direction port 1
 // TEST_SCPI_INPUT("GPIO:GETP:DEV5:GP22?   \r\n");  //Set Gp22 as output
  //TEST_SCPI_INPUT("GPIO:SETP:DEV0:GP22  #H06 \r\n");  //Set Gp22 as output
 // TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  0 \r\n");  //Set Gp22 as output
 // TEST_SCPI_INPUT("GPIO:GETP:DEV1:GP22?   \r\n");  //Set Gp22 as output
 // TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  0 \r\n");  //Set Gp22 as output
 // TEST_SCPI_INPUT("GPIO:GETP:DEV0:GP22?   \r\n");  //Set Gp22 as output
  //TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  1 \r\n");  //Set Gp22 as output
  //TEST_SCPI_INPUT("GPIO:IN:DEV0:GP22?   \r\n");  //Set Gp22 as output
  //  TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  0 \r\n");  //Set Gp22 as output
  //TEST_SCPI_INPUT("GPIO:IN:DEV0:GP22?   \r\n");  //Set Gp22 as output

  // TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  1 \r\n");  //Set Gp22 as output
  // TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22?  \r\n");  //Get Gp22 direction
  // TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  1 \r\n");  //Set Gp22 as output
  // TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22?  \r\n");  //Get Gp22 direction

  // TEST_SCPI_INPUT("GPIO:DIR:DEV2:GP22  1 \r\n");  //Set Gp22 as output
  // TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  1 \r\n");  //Set Gp22 as output

   //TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n"); // set direction port 1
   //TEST_SCPI_INPUT("DIG:OUT:PORT1 #H55 \r\n");  
   //TEST_SCPI_INPUT("DIG:OUT:PORT1 #HAA \r\n"); 

   //TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); 
   //TEST_SCPI_INPUT("DIG:IN:PORT1? \r\n"); 

  // TEST_SCPI_INPUT("DIG:DIR:PORT0 #HAA \r\n"); 
  // TEST_SCPI_INPUT("DIG:DIR:PORT0? \r\n"); 
  // TEST_SCPI_INPUT("DIG:DIR:PORT1? \r\n"); 

   //TEST_SCPI_INPUT("DIG:DIR:PORT0 #Hf0 \r\n"); 
   //TEST_SCPI_INPUT("DIG:DIR:PORT0:BIT1? \r\n"); 
   //TEST_SCPI_INPUT("DIG:DIR:PORT0:BIT5? \r\n"); 

   //TEST_SCPI_INPUT("DIG:DIR:PORT1 #Hff \r\n");
  // TEST_SCPI_INPUT("DIG:OUT:PORT1:BIT0 1 \r\n");
   //TEST_SCPI_INPUT("DIG:OUT:PORT1:BIT0 0 \r\n");

  // TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n");
  // TEST_SCPI_INPUT("DIG:IN:PORT1:BIT0? \r\n");
  // TEST_SCPI_INPUT("DIG:IN:PORT1:BIT4? \r\n");


  // TEST_SCPI_INPUT("DIG:OUT:PORT1 #HAA \r\n"); 

   //TEST_SCPI_INPUT("DIG:DIR:PORT1:BIT1 0\r\n"); 

  //TEST_SCPI_INPUT("ROUT:CLOSE (@10)\r\n");  // test com i2c #22
  //TEST_SCPI_INPUT("ROUT:CLOSE (@30)\r\n");  // test com i2c #23


  TEST_SCPI_INPUT("SYST:ERR:COUNT?\r\n");
  TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
  TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
 
   
   
   
  // TEST_SCPI_INPUT("*IDN?\r\n"); 
   //TEST_SCPI_INPUT("*OPC?\r\n"); 
   //TEST_SCPI_INPUT("*WAI\r\n");      // do nothing
   //TEST_SCPI_INPUT("*RST\r\n");   // trig reset, to be added
   //TEST_SCPI_INPUT("SYST:VERS?\r\n");

   //TEST_SCPI_INPUT("ROUT:OPEN (@30)\r\n"); // error -11 if Pico #23 not present
   //TEST_SCPI_INPUT("ROUT:OPEN (@10!30)\r\n"); // error -12  2-dimension list
  // TEST_SCPI_INPUT("ROUT:OPEN (@10:18)\r\n"); // error -13
  //TEST_SCPI_INPUT("ROUT:CLOSE (@100:102)\r\n");


  // TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK3\r\n"); 

   //TEST_SCPI_INPUT("SYST:ERR:COUNT?\r\n");
   //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
   //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 

   //TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2\r\n"); 
 //  TEST_SCPI_INPUT("ROUT:SE:STAT? BANK1,BANK2 \r\n");
 //  TEST_SCPI_INPUT("ROUT:CLOSE (@100,208)\r\n");
  // TEST_SCPI_INPUT("ROUT:SE:STAT? BANK1,BANK2 \r\n");

  //TEST_SCPI_INPUT("ROUT:CLOSE (@100:105)\r\n");
  //TEST_SCPI_INPUT("ROUT:BANK:STAT? BANK1,BANK2 \r\n");
  //TEST_SCPI_INPUT("ROUT:SE:STAT? BANK1,BANK2 \r\n");
  //TEST_SCPI_INPUT("ROUT:OPEN (@10,17)\r\n"); 

  //TEST_SCPI_INPUT("ROUT:BANK:STAT? BANK1 \r\n");
  

  // TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK2\r\n");
//   TEST_SCPI_INPUT("ROUT:CHAN:STAT? (@20:27)\r\n");
//   TEST_SCPI_INPUT("ROUT:CLOSE (@20:27)\r\n");
   //TEST_SCPI_INPUT("ROUT:CLOSE (@100,102)\r\n");
   //TEST_SCPI_INPUT("ROUT:CLOSE (@100,102,115,202,215)\r\n");
   //TEST_SCPI_INPUT("ROUT:CHAN:STAT? (@10:17)\r\n");
   //TEST_SCPI_INPUT("ROUT:CHAN:STAT? (@20:27)\r\n"); 
   //TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2\r\n"); 
   //TEST_SCPI_INPUT("ROUT:CHAN:STAT? (@10:17)\r\n");
   //TEST_SCPI_INPUT("ROUT:CHAN:STAT? (@20:27)\r\n"); 

   //TEST_SCPI_INPUT("ROUT:OPEN (@200:207)\r\n"); 
   //TEST_SCPI_INPUT("ROUT:CHAN:STAT? (@200:207)\r\n"); 
   //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
   //TEST_SCPI_INPUT("SYST:ERR:COUNT?\r\n");
   //TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK2\r\n"); 
   //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
   //TEST_SCPI_INPUT("ROUT:OPEN:ALL \r\n");
   //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
   //TEST_SCPI_INPUT("ROUT:CLOSE (@104,106)\r\n"); 
   
   //TEST_SCPI_INPUT("ROUT:CLOSE:EXCL (@208)\r\n"); 
   //TEST_SCPI_INPUT("ROUT:OPEN (@104)\r\n"); 


   // TEST_SCPI_INPUT("TEST:CHANnellist (@100:105)\r\n");
   // TEST_SCPI_INPUT("TEST:CHANnellist:EXCL (@120:125)\r\n");

	while (1) {
        //printf("Hello, world!\n");
        uart_puts(UART_ID, "Hello UART!\n");
        fprintf(stdout,"USB CONNECTED \r\n");

        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(500);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(500);

       

		// Feed single characters into the SCPI library.
		// This is somewhat discouraged, since (according to the docs) it will
		// apparently cause the whole buffer to be re-parsed after each character.
		//* char c = fgetc(stdin);  // TODO: Error handling.
		//*SCPI_Input(&scpi_context, &c, 1);

/*
    while (deque(&rec)) {
      gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn OFF board led
      uart_puts(UART_ID, &rec.data[0]);
      //printf("Pico Master: %s\n",&rec.data[0]);  // send message to serial port
      sleep_ms(5);
      gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn ON board led
    }
    */
	}
	
    fprintf(stdout,"program terminated\r");
}
