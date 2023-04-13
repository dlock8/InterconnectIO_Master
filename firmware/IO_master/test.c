
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
