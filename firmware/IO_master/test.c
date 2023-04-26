
#define TEST_SCPI_INPUT(cmd)  result = SCPI_Input(&scpi_context, cmd, strlen(cmd))

   // TEST_SCPI_INPUT("TEST:CHANnellist (@1!2:3!4,5!6)\r\n");
   
 //   send_master (PICO_PORT_ADDRESS,01,0x00); // test command 
    //send_master (PICO_PORT_ADDRESS,02,0x00); // test command 


   //TEST_SCPI_INPUT("*IDN?\r\n"); 

   //TEST_SCPI_INPUT("*ESE 1\r\n"); 
   //TEST_SCPI_INPUT("*ESE?\r\n"); 

  // TEST_SCPI_INPUT("*CLS\r\n"); 
  // TEST_SCPI_INPUT("*RST\r\n"); 

 // TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); // set direction port 1 = input
 // TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port 0 = output
 // TEST_SCPI_INPUT("DIG:IN:PORT1? \r\n");
 // TEST_SCPI_INPUT("DIG:OUT:PORT0 #H55 \r\n"); 
  //TEST_SCPI_INPUT("DIG:IN:PORT1? \r\n");
 // TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n"); 
 // TEST_SCPI_INPUT("DIG:IN:PORT1? \r\n");
 



 // TEST_SCPI_INPUT("SYST:LED:ERR ON \r\n");
 // TEST_SCPI_INPUT("SYST:LED:ERR OFF \r\n");
 // TEST_SCPI_INPUT("SYST:VERS? \r\n");

// TEST_SCPI_INPUT("SYST:DEV:VERS? \r\n");
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

  // TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  1 \r\n");    // MESSAGE rec;/Set Gp22 as output
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
  // MESSAGE rec;
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


  //TEST_SCPI_INPUT("SYST:ERR:COUNT?\r\n");
  //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
  //TEST_SCPI_INPUT("SYST:ERR?\r\n"); 
 
   
   
   
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
//Register device

#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "stdio.h"

bool test_eeprom() {

// 24LC32 validation test code
 #define  PAGESIZE 32
 #define  EEMODEL  32  // 24lc32 eeprom
 #define  RWTESTSIZE 8

  at24cx_dev_t eeprom_1;

  at24cx_writedata_t dt;
  uint16_t eeadd = 0xfe0;
  
 volatile uint8_t  wdata[PAGESIZE];
 volatile uint8_t  rdata[PAGESIZE];
 bool errflag;

 // register eeprom 24lc32
  at24cx_i2c_device_register(&eeprom_1, EEMODEL, I2C_ADDRESS_AT24CX);
  
  //Check if eeprom_1 is active
  fprintf(stdout,"\neeprom_1 is %s\n", eeprom_1.status ? "detected" : "not detected");
  if (eeprom_1.status == false) return false;
  

  fprintf(stdout,"\nWrite byte test\n\n");
  for(int i=0;i<RWTESTSIZE;i++)
    {
      dt.address = eeadd +i;
      dt.data = i;
      wdata[i] = dt.data;  // save value for compare later

      if (at24cx_i2c_byte_write(eeprom_1, dt) == AT24CX_OK) {
        fprintf(stdout,"Writing at address 0x%02X: %d\n", dt.address, dt.data);
      }else {
        fprintf(stdout,"Device write byte error! \n");
        return false;
      }
    }

    fprintf(stdout,"\nRead byte test\n\n");
    for(int i=0;i<RWTESTSIZE;i++)
    {
      dt.address = eeadd +i;
      if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK) {
         fprintf(stdout,"Reading at address 0x%02X: %d\n", dt.address, dt.data);
         rdata[i] = dt.data;
      }else {
        fprintf(stdout,"Device byte read error!\n");
        errflag = true;
      }
    }

    // Compare value write to value read
    fprintf(stdout,"\nCompare Write and Read byte test\n\n");
    for(int i=0;i<RWTESTSIZE;i++)
    {
      if (wdata[i] != rdata[i]) {
         fprintf(stdout,"Error byte Write-read at address 0x%02X: write value 0x%02X: read value: 0x%02X\n", eeadd +i, wdata[i], rdata[i]);
         errflag = true;
      }
    }
      



   // current read adress return bad value.
   // at24cx_i2c_current_address_read(eeprom_1, &dt);
   // fprintf(stdout,"Current read address 0x%02X\n", dt.data);


  fprintf(stdout,"\nWrite page test\n\n");
  dt.address = eeadd;
  for(int i=0;i<PAGESIZE;i++) 
    {
      dt.data_multi[i]= i;
      wdata[i] = dt.data_multi[i];

      fprintf(stdout,"Writing at page data at position 0x%02X: %d\n", dt.address + i, dt.data_multi[i]);
    }

    if (at24cx_i2c_page_write(eeprom_1, dt) == AT24CX_OK) fprintf(stdout,"Page Writing at address 0x%02X\n", dt.address);
    else { fprintf(stdout,"Device page write error!\n"); return false; }


    fprintf(stdout,"\nRead page test\n\n");
    for(int i=0;i<PAGESIZE;i++)
    {
      dt.address = eeadd+i;

       if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK) {
        fprintf(stdout,"Reading at address 0x%02X: %d\n", dt.address, dt.data);
        rdata[i] = dt.data;
        } else {fprintf(stdout,"Device page read error!\n"); errflag = true; ;}
    }

        // Compare value write to value read
    fprintf(stdout,"\nCompare Write and Read page test\n\n");
    for(int i=0;i<PAGESIZE;i++)
    {
      if (wdata[i] != rdata[i]) {
         fprintf(stdout,"Error byte Write-read at address 0x%02X: write value 0x%02X: read value: 0x%02X\n", eeadd +i, wdata[i], rdata[i]);
         errflag = true;
      }
    }

  if (errflag == true) return false;
  else return true;

}