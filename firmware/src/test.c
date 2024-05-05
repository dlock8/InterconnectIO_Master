
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "pico_lib2/src/dev/dev_ds2431/dev_ds2431.h"
#include "pico_lib2/src/sys/include/sys_adc.h"
#include "hardware/uart.h"
//#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "include/functadv.h"
#include "include/fts_scpi.h"
#include "include/i2c_com.h"
#include "include/test.h"
#include "include/scpi_spi.h"

#include "/home/pi/pico/pico-sdk/src/rp2_common/hardware_spi/include/hardware/spi.h"



/**> Global variables */
char out_buffer[1024];
size_t out_buffer_pos;



/*! @brief - Function not used during normal execution
*    The test_adc was used to test to get all the measure in same time from the Pico ADC
*
*    @return
*/
bool test_adc(){
  float readv;

sys_adc_init(ADC_CH_0); // initialize ADC0
sys_adc_init(ADC_CH_1);
//sys_adc_init(ADC_CH_2); /** Connected to OE */
sys_adc_init(ADC_CH_V);
sys_adc_init(ADC_CH_T);

readv = sys_adc_volt(ADC_CH_0);
fprintf(stdout,"ADC_CH_0: %2.3f V\n", readv);

readv = sys_adc_volt(ADC_CH_1);
fprintf(stdout,"ADC_CH_1: %2.3f V\n", readv);

//readv = sys_adc_volt(ADC_CH_2);
//fprintf(stdout,"ADC_CH_2: %2.3f V\n", readv);

readv = sys_adc_vsys();
fprintf(stdout,"VSYS: %2.3f V\n", readv);

readv = sys_adc_temp_c();
fprintf(stdout,"TEMP C: %2.3f C\n", readv);

}


/*! @brief - Function not used during normal execution
*    The function was used to test to get all the measure in same time from the INA219
*
*    @return
*/
void test_ina219(void){

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
}

/*! @brief - Function not used during normal execution
*    The power_test was used to test the hardware of interconnect IO board
*    connected to selftest board without using of SCPI command      
*
*    @param mode Number to define wich measurement need to be performed*    @param expect_value   Value expected to be a valid value
*    @param percentage_lo  Lower deviation from expect_value acceptable to PASS the test
*    @param percentage_hi  Higher deviation from expect_value acceptable to PASS the test
*    @return
*/
bool power_test(uint8_t mode, float expect_value, float percentage_lo, float percentage_hi){
  int16_t readv;
  uint8_t flag = false;
  float hilimit, lolimit;
  char meas[3];

  switch (mode){
    case V :
      readv = ina219GetBusVoltage() * 0.001;
      strcpy(meas,"V");
      flag =true;
      break;
    case I :
      readv = ina219GetCurrent_mA();
      strcpy(meas,"mA");
      flag =true;
      break;
    case P :
      readv = ina219GetPower_mW();
      strcpy(meas,"mW");
      flag =true;
      break;
    case S :
      readv = ina219GetShuntVoltage() * 10E-3;
      strcpy(meas,"mV");
      flag =true;
      break;
  }
  if (flag) {   // if a value has been read

    if (expect_value > 0 ) {
        
        hilimit = expect_value + (expect_value * percentage_hi / 100);
        lolimit = expect_value - (expect_value * percentage_lo / 100);
    }else {
        hilimit = expect_value + percentage_hi;
        lolimit = expect_value - percentage_lo;
    }

    if (readv > hilimit || readv < lolimit) {
        fprintf(stdout,"---> FAIL <---VAL:%d %s, LL:%2.2f, HL:%2.2f  \n",readv,meas,lolimit,hilimit);
    } else {
        fprintf(stdout,"---> PASS <---VAL:%d %s LL:%2.2f, HL:%2.2f  \n",readv,meas,lolimit,hilimit);
          
    }
  }
}


/*! @brief - Function not used during normal execution
*    The test_dac function is used to set a value on DAC device  
*
*    @param value  Value in Volt to set the DAC
*    @param expect_value   Value expected to be a valid value
*    @return false  if error detected during the set of a value
*            true   if no error detected 
*/
bool test_dac(float value){

 // float value;

 if (!dev_mcp4725_set(i2c0, MCP4725_ADDR0, value))
  {
     fprintf(stdout,"DAC Error on set MCP4725\n"); 
     return false;
  }
  fprintf(stdout,"DAC voltage set to: %2.3f V\n", value);
  return true; 
}


/*! @brief - Function not used during normal execution
*    The adc_test was used to test the hardware of interconnect IO board
*    connected to selftest board without using of SCPI command      
*
*    @param channel Number to define wich measurement channel is required
*    @param expect_value   Value expected to be a valid value
*    @param percentage_lo  Lower deviation from expect_value acceptable to PASS the test
*    @param percentage_hi  Higher deviation from expect_value acceptable to PASS the test
*    @return
*/
bool adc_test(uint8_t channel, float expect_value, float percentage_lo, float percentage_hi){
  float readv;
  uint8_t flag = false;
  float hilimit, lolimit;

  switch (channel){
    case 0 :
      readv = sys_adc_volt(ADC_CH_0);  // multiply * 2 due to voltage divider
      flag =true;
      break;
    case 1 :
      readv = sys_adc_volt(ADC_CH_1);
      flag =true;
      break;
    case 2 :
      readv = sys_adc_vsys();
      flag =true;
      break;
    case 4 :
      readv = sys_adc_temp_c();
      flag =true;
      break;
  }

  if (flag) {   // if a value has been read
        
        hilimit = expect_value + (expect_value * percentage_hi / 100);
        lolimit = expect_value - (expect_value * percentage_lo / 100);

        if (readv > hilimit || readv < lolimit) {
          fprintf(stdout,"---> FAIL <--- CH: %1d VAL:%2.3f V, LL:%2.3f, HL:%2.3f  \n",channel,readv,lolimit,hilimit);
          return false;
        } else {
          fprintf(stdout,"---> PASS <--- CH: %1d VAL:%2.3f V, LL:%2.3f, HL:%2.3f  \n", channel,readv,lolimit,hilimit);
          return true;
        }
  }
  return false;
}


/*! @brief - Function to clear the output_buffer
*   Output_buffer is used by selftest program to capture the response
*   form a SCPi command executex by the SCPI parser        
*   
*   @return 
*/
static void output_buffer_clear(void) {
    out_buffer[0] = '\0';
    out_buffer_pos = 0;
}

/*! @brief - ChatGPt function to remove CR and LF from a strng
*            
*
*   @param str String who contains the CR and LF to remove
*   @return  String without CR and LF
*/
void removeCRLF(char *str) {
    if (str == NULL) {
        // Handle NULL pointer gracefully
        return;
    }

    int len = strlen(str);
    int readIndex = 0;
    int writeIndex = 0;

    // Iterate through the string
    while (readIndex < len) {
        // If the character is not a carriage return or newline, keep it
        if (str[readIndex] != '\r' && str[readIndex] != '\n') {
            str[writeIndex] = str[readIndex];
            writeIndex++;
        }

        readIndex++;
    }

    // Null-terminate the modified string
    str[writeIndex] = '\0';
}



/*! 
* @struct TestResult
* @brief  Structure to count the test result during seltest sequence
*               
* This structure hold the counter to be able to display a resume at the end of selftest.
*/
struct TestResult {
  int total;    /**< Total of test performed. */
  int good;     /**< Total of PASS test performed. */
  int bad;      /**< Total of FAIL test performed. */
  int error;    /**< Total of Error captured. */
};



/*! @brief - Send SCPI command and validate the value for PASS or FAIL.
*            The analog value returned by command is validated to determine is between determined limit 
*
*    @param title Test title to be used to report the purpose of the measure
*    @param cmd   String command to send to the SCPI engine
*    @param expect_value   Value expected to be returned by the measure
*    @param unit     unit to use on printf
*    @param lolimit  Lower deviation from expect_value acceptable to PASS the test
*    @param hilimit  Higher deviation from expect_value acceptable to PASS the test
*    @param counter Structure who content the result of test. Be used by final report
*    @return
*/
void test_cmd_result(const char *title, const char *cmd, float expect_value, const char *unit, float lolimit, float hilimit, struct TestResult *counter){
   float  readv;
   float hl,ll;

   counter->total++; // increment counter
   output_buffer_clear(); // clear result before capture output

   SCPI_Input(&scpi_context, cmd, strlen(cmd));  /** Send command to SCPI engine*/
   // transform string received from command to number
   do { \
        if (sscanf((out_buffer), "%f", &(readv)) != 1) { 
            fprintf(stderr, "\t ERROR converting buffer to float, rvalue: %s\n", out_buffer);
            (readv) = -99.99; 
            counter->error++;
        } 
    } while (0);

    // on ADC measure,multiplication of value by 2 is required due to voltage divider on selftest board 
    if (strstr(cmd, "ADC0") != NULL || strstr(cmd, "ADC1") != NULL ) { readv = readv *2; }
    hl = expect_value + hilimit;
    ll = expect_value - lolimit;

    if (readv > hl || readv < ll) {
          fprintf(stdout,"%s  ---> FAIL  VAL:%.2f %s, LL:%.2f, HL:%.2f  \n",title,readv,unit,ll,hl);
          counter->bad++;
    } else {
          fprintf(stdout,"%s  ---> PASS  VAL:%.2f %s, LL:%.2f, HL:%.2f  \n",title,readv,unit,ll,hl);
          counter->good++;
    }
}

/*! @brief - Send SCPI command and validate the value for PASS or FAIL.
*            The analog value returned by command is validated to determine is between determined limit 
*
*    @param title Test title to be used to report the purpose of the measure
*    @param cmd   String command to send to the SCPI engine
*    @param expected_result   String tovalidate with the result returned by the commansd
*    @param per_lo  Lower percentage deviation from expect_value acceptable to PASS the test
*    @param per_hi  Higher percentage deviation from expect_value acceptable to PASS the test
*    @param counter Structure who content the result of test. Be used by final report
*    @return
*/
void test_cmd_out(const char *title, const char *cmd,char *expected_result, struct TestResult *counter){
   float  readv;
   bool valid = true;

   counter->total++; // increment counter
   output_buffer_clear(); // clear result before capture output
   SCPI_Input(&scpi_context, cmd, strlen(cmd));  /** Send command to SCPI engine*/
   removeCRLF(out_buffer); // remove \r\n from string before comparaison

  // validate if string are identical, raise flag if not identical
   do {
        if (strcmp((expected_result), (out_buffer)) != 0) { valid = false; } 
    } while (0);

  // report result of the comparaison
   if (!valid) {   
          fprintf(stdout,"%s  ---> FAIL  Expected: %s, Read: %s\n",title,expected_result,out_buffer);
          counter->bad++;
    } else {
          fprintf(stdout,"%s  ---> PASS  Read: %s\n",title,out_buffer);
          counter->good++;
    }
}


/*! @brief - Run test sequence to validate hardware with selftest board
*            
*   This function perform the selftest to validate the hardware using only internal instruments.
*   The test sequence follow the Validation_Test.xls Excel list
*
*   Report is printed at the end of the test
*
*    @return
*/   
bool test_selftest() {
 int result;
 bool good;
 float fnumber;
 char sval[40];  

 struct TestResult ctest = {0,0,0,0}; // reset counter structure
 out_buffer_pos =0;

#define TEST_SCPI_INPUT(cmd)  result = SCPI_Input(&scpi_context, cmd, strlen(cmd))

 fprintf(stdout,"\tSelftest Hardware Test\n");  // send message to debug port


 TEST_SCPI_INPUT("SYST:SLA OFF\r\n");       /** Disable slaves Pico to reset configuration*/
 sleep_ms(100);
 TEST_SCPI_INPUT("SYST:SLA ON\r\n");        /** Start slaves Pico after reset*/
 sleep_ms(500);
 TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF\r\n"); /** Turn OFF red led */


// initialisation of all ressources used on selftest board to toggle relays
// All digital IO are set as input after reset.
// All IO need to be set as output and initialized to 0 before the start of the test
// 
 TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port 0 ouput
 TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); // set direction port 1 input
 TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // set port 0 to 00
 TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 1 \r\n");  // set io to output
 TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 1 \r\n"); // set io to output
 TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP8 1 \r\n");  // set io to output
 TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP9 1 \r\n"); // set io to output
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0 \r\n"); 
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1  0 \r\n"); 
 TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n");  
 TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9  0 \r\n"); 
 TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP18 1 \r\n");  // set to output
 TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP19 0 \r\n");  // set to input
 TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG output
 TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP19  0 \r\n");  // CTRL Input
 TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
 TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");
 TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");
 

TEST_SCPI_INPUT("SYST:OUT ON\r\n");         /** Close Power Relay et apply 5V to Selftest board */
sleep_ms(250);  /** Wait to let time to relay to close and power the selftest board */
test_cmd_result("Test 1.1: 5VDC Check with ADC0","ANA:ADC0:VOLT? \r\n",5.0,"V", 0.3, 0.3, &ctest);


// Port 0 validation
 TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port 0 output
 TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); // set direction port 1 input
 TEST_SCPI_INPUT("DIG:OUT:PORT0 #H55 \r\n"); 
 test_cmd_out("Test 2.0: Port 0 = 0x55(85), Read Port 1 value ", "DIG:IN:PORT1?\r\n","85",&ctest);
 TEST_SCPI_INPUT("DIG:OUT:PORT0 #HAA \r\n"); 
 test_cmd_out("Test 2.1: Port 0 = 0xAA(170), Read Port 1 value ", "DIG:IN:PORT1?\r\n","170",&ctest);

 TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n"); // set direction port 1 output
 TEST_SCPI_INPUT("DIG:DIR:PORT0 #H00 \r\n"); // set direction port 1 input
 TEST_SCPI_INPUT("DIG:OUT:PORT1 #H33 \r\n"); 
 test_cmd_out("Test 2.2: Port 1 = 0x33(51), Read Port 0 value ", "DIG:IN:PORT0?\r\n","51",&ctest);
 TEST_SCPI_INPUT("DIG:OUT:PORT1 #HCC \r\n"); 
 test_cmd_out("Test 2.3: Port 1 = 0xCC(204), Read Port 0 value ", "DIG:IN:PORT0?\r\n","204",&ctest);

 // replace original value after test of digital port
 TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port 0 output
 TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); // set direction port 1 input
 TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n"); // reset digital output

// Test of handshake ctrl and flag
 TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // FLAG
 test_cmd_out("Test 2.4: Handshake Flag = 1, Read Ctrl bit ", "GPIO:IN:DEV1:GP19?\r\n","1",&ctest);
 TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG
 test_cmd_out("Test 2.4: Handshake Flag = 0, Read Ctrl bit ", "GPIO:IN:DEV1:GP19?\r\n","0",&ctest);
 TEST_SCPI_INPUT("GPIO:IN:DEV1:GP19?  \r\n");  // CTRL 


// Digital test 2.6 and 2.7 to verify each GPIO using Pico located on selftest board is TBD

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H20 \r\n");  // Close K16 (VM2)
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");  // Close OC1
test_cmd_result("Test 3.0: Output Collector OC1 Drive 1 (Active),read ADC0","ANA:ADC0:VOLT? \r\n",0.2,"V", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");  // Open OC1
test_cmd_result("Test 3.1: Output Collector OC1 Drive 0 (Inactive),read ADC0","ANA:ADC0:VOLT? \r\n",5, "V",0.2, 0.2, &ctest);

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H10 \r\n");  // Close K15 (VM3)
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");  // Close OC2
test_cmd_result("Test 3.2 Output Collector OC2 Drive 1 (Active),read ADC0","ANA:ADC0:VOLT? \r\n",0.2,"V", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");  // Open OC2
test_cmd_result("Test 3.3: Output Collector OC2 Drive 0 (Inactive),read ADC0","ANA:ADC0:VOLT? \r\n",5,"V", 0.2, 0.2, &ctest);

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H90 \r\n");  // Close K15,K9 (VM4)
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3 \r\n");  // Close OC3
test_cmd_result("Test 3.4 Output Collector OC3 Drive 1 (Active),read ADC0","ANA:ADC0:VOLT? \r\n",0.2,"V", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");  // Open OC3
test_cmd_result("Test 3.5: Output Collector OC3 Drive 0 (Inactive),read ADC0","ANA:ADC0:VOLT? \r\n",5,"V", 0.2, 0.2, &ctest);



TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  1 \r\n"); // Close K13 (ADC1)
test_cmd_result("Test 4.0: 5VDC Check with ADC1","ANA:ADC1:VOLT? \r\n",5.0,"V", 0.3, 0.3, &ctest);

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H40 \r\n");  // Close K2
TEST_SCPI_INPUT("ANA:DAC:VOLT 3 \r\n");  // Set ouput to 3v
sleep_ms(250);
test_cmd_result("Test 4.1: Dac output @ 3Vdc, read ADC1","ANA:ADC1:VOLT? \r\n",3.0,"V", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ANA:DAC:VOLT 0.25 \r\n");  // Set ouput to 0.25V
sleep_ms(250);
test_cmd_result("Test 4.2: Dac output @ 0.25Vdc, read ADC1","ANA:ADC1:VOLT? \r\n",0.25,"V", 0.02, 0.06, &ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n"); // Open K13 (ADC1)
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // Open K2


test_cmd_result("Test 5.0: PWR Module check Bus Voltage (5VDC), read Vdc:","ANA:PWR:V? \r\n",5,"V", 0.3, 0.3, &ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
test_cmd_result("Test 5.1: PWR Module check Shunt voltage , read mV:","ANA:PWR:S? \r\n",50,"mV", 7, 7, &ctest);
test_cmd_result("Test 5.2: PWR Module check current on 10 ohm(R2), read I(mA):","ANA:PWR:I? \r\n",500,"mA", 50, 50, &ctest);

// Perform Calibration of the PWR module INA219
float readv;  // contains the current value read from last command 
sscanf((out_buffer), "%f", &(readv));  // transform string in output_buffer to float number
sprintf(sval,"ANAlog:PWR:Cal %.2f, 500\r\n",readv);  // build calibration string to be used as command
TEST_SCPI_INPUT(sval);  // send command
test_cmd_result("Test 5.3: PWR Module check current on 10 ohm(R2), read I(mA):","ANA:PWR:I? \r\n",500,"mA", 5, 5, &ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4



TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
//TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 1 \r\n");  // Close K15
//TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP19  1 \r\n");  // Close K16 (VM5)
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H32 \r\n");  // Close K7,K15,K16 (VM5)
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");  // Close K11
test_cmd_result("Test 6.0: Resistor load (10 ohms) check,read ADC0","ANA:ADC0:VOLT? \r\n",2.5,"V", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");  // Open K11
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");// Open K7,K15,K16 (VM5)



TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");  // Close K10
TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR2 \r\n");
sleep_ms(100);
test_cmd_result("Test 7.0: Low Power Relay NC1 check,read I(mA)","ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR2 \r\n");
TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1 \r\n");
sleep_ms(100);
test_cmd_result("Test 7.1: Low Power Relay NO1 check,read I(mA)","ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
sleep_ms(100);
test_cmd_result("Test 7.2: Low Power Relay OPEN1 check,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2,0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");  // Open K10

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H01 \r\n");// Close K8 (PS3)
TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");
TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR2 \r\n");
sleep_ms(100);
test_cmd_result("Test 7.3: Low Power Relay NC2 check,read I(mA)","ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR2 \r\n");
TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1 \r\n");
sleep_ms(100);
test_cmd_result("Test 7.4: Low Power Relay NO2 check,read I(mA)","ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1 \r\n");

test_cmd_result("Test 7.5: Low Power Relay OPEN2 check,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2,0.2, &ctest);
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");// Open K8 (PS3)



TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H02 \r\n");// Close K7 (PS5)
TEST_SCPI_INPUT("ROUT:CLOSE:PWR HPR1 \r\n");
sleep_ms(100);
test_cmd_result("Test 8.0: High Power Relay CLOSE check,read I(mA)","ANA:PWR:I? \r\n",250,"mA", 15, 50, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:PWR HPR1 \r\n");
sleep_ms(100);
test_cmd_result("Test 8.1: High Power Relay OPEN check,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H01 \r\n");// Close K8
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1 \r\n");  // Close K10 (PS6)
TEST_SCPI_INPUT("ROUT:CLOSE:PWR SSR1 \r\n");
test_cmd_result("Test 9.0: Solid State Relay CLOSE check,read I(mA)","ANA:PWR:I? \r\n",250,"mA", 15, 50, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:PWR SSR1 \r\n");
test_cmd_result("Test 9.1: Solid State Relay CLOSE check,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");  // Open K10 (PS6)
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4



/** RELAY BANK TEST */
int i,j,t;
i = 0; // count index
j=  0;
t= 10;

/** Loop to test BK1-BK2 channel*/
for (i = 0; i < 8; i++) {
  /** Test the High side of relay*/
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");// Close K7,K8 (PS7)
  sprintf(sval,"ROUT:CLOSE (@10%d,20%d)\r\n",i,i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 CH%d-H Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@20%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 BK2_CH%d-H Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:CLOSE (@20%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 BK2_CH%d-H Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@10%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 BK1_CH%d-H Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:OPEN (@20%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);


  /** Test the LOW side of relay*/
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H0B \r\n");// Close K7,K8,K14 (PS8)
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3 \r\n");  // Close K12 (PS6)

  sprintf(sval,"ROUT:CLOSE (@10%d,20%d)\r\n",i,i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 CH%d-L Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@20%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 BK2_CH%d-L Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:CLOSE (@20%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 BK2_CH%d-L Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@10%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK1-BK2 BK1_CH%d-L Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:OPEN (@20%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");  // OPen K12 (PS6)
}  // end of loop for BK1-BK2 relay test 

// Check for the COM relay H Side
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H0B \r\n");// Close K7,K8,K14 (PS7)
TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n"); // Close BK1-Bk2 CH0 relays and SE relays
test_cmd_result("Test 11.0: Relay BK1-BK2 COM-H Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@208)\r\n"); // Open SE relay
test_cmd_result("Test 11.1: Relay BK1-BK2, BK2-COM-H Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n"); // Close BK1-Bk2 CH0 relays
test_cmd_result("Test 11.2: Relay BK1-BK2 COM-H Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@108)\r\n"); // Open SE relay
test_cmd_result("Test 11.3: Relay BK1-BK2, BK1-COM-H Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

// Check for the COM relay L Side
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");// Close K7,K8,K14 (PS7)
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC3 \r\n");  // Close K12 (PS6)
TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n"); // Close BK1-Bk2 CH0 relays and SE relays
test_cmd_result("Test 11.4: Relay BK1-BK2 COM-L Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@208)\r\n"); // Open SE relay
test_cmd_result("Test 11.5: Relay BK1-BK2, BK2-COM-L Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

TEST_SCPI_INPUT("ROUT:CLOSE (@108,208)\r\n"); // Close BK1-Bk2 CH0 relays
test_cmd_result("Test 11.6: Relay BK1-BK2 COM-L Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@108)\r\n"); // Open SE relay
test_cmd_result("Test 11.7: Relay BK1-BK2, BK1-COM-L Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");  // OPen K12 


i = 0; // count index
j=  0;
t= 12;

/** Loop to test BK3-BK4 channel*/
for (i = 0; i < 8; i++) {


  /** Test the High side of relay*/
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");// Close K7,K8
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");  // Close K11 (PS9)

  sprintf(sval,"ROUT:CLOSE (@30%d,40%d)\r\n",i,i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 CH%d-H Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@40%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 BK4_CH%d-H Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:CLOSE (@40%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 BK4_CH%d-H Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@30%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 BK3_CH%d-H Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:OPEN (@40%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n"); 


  /** Test the LOW side of relay*/
  TEST_SCPI_INPUT("DIG:OUT:PORT0 #H13 \r\n");// Close K7,K8,K11,K12 (PS10),K15
  TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2,OC3 \r\n");  // Close K11,K12

  sprintf(sval,"ROUT:CLOSE (@30%d,40%d)\r\n",i,i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 CH%d-L Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@40%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 BK4_CH%d-L Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:CLOSE (@40%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 BK4_CH%d-L Close Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",50,"mA", 5, 20, &ctest);

  sprintf(sval,"ROUT:OPEN (@30%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  sprintf(sval,"Test %d.%d Relay BK3-BK4 BK3_CH%d-L Open Test, read I(mA)",t,j++,i); // build verification string
  test_cmd_result(sval,"ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

  sprintf(sval,"ROUT:OPEN (@40%d)\r\n",i);// build channel string
  TEST_SCPI_INPUT(sval);
  TEST_SCPI_INPUT("ROUT:OPEN:OC OC2,OC3 \r\n");  // OPen K11,K12
}

// Check for the COM relay H Side
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H13 \r\n");// Close K7,K8,K15
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2 \r\n");  // Close K11 (PS9)
TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n"); // Close BK3-Bk4 CH0 relays and SE relays
test_cmd_result("Test 13.0: Relay BK3-BK4 COM-H Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@408)\r\n"); // Open SE relay
test_cmd_result("Test 13.1: Relay BK3-BK4, BK2-COM-H Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n"); // Close BK3-Bk4 CH0 relays
test_cmd_result("Test 13.2: Relay BK3-BK4 COM-H Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@308)\r\n"); // Open SE relay
test_cmd_result("Test 13.3: Relay BK3-BK4, BK3-COM-H Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

// Check for the COM relay L Side
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H03 \r\n");// Close K7,K8,
TEST_SCPI_INPUT("ROUT:CLOSE:OC OC2,OC3 \r\n");  // Close K11,K12 (PS10)
TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n"); // Close BK3-Bk4 CH0 relays and SE relays
test_cmd_result("Test 13.4: Relay BK3-BK4 COM-L Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@408)\r\n"); // Open SE relay
test_cmd_result("Test 13.5: Relay BK3-BK4, BK4-COM-L Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);

TEST_SCPI_INPUT("ROUT:CLOSE (@308,408)\r\n"); // Close BK3-Bk4 CH0 relays
test_cmd_result("Test 13.6: Relay BK3-BK4 COM-L Close Test,read I(mA)","ANA:PWR:I? \r\n",50,"mA",5, 20, &ctest);

TEST_SCPI_INPUT("ROUT:OPEN (@308)\r\n"); // Open SE relay
test_cmd_result("Test 13.7: Relay BK3-BK4, BK3-COM-L Open Test,read I(mA)","ANA:PWR:I? \r\n",0,"mA", 0.2, 0.2, &ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC2,OC3 \r\n");  // OPen K11,K12 


/** PRINT FINAL REPORT AFTER TEST COMPLETION*/
fprintf(stdout, "\n\n\t SELFTEST COMPLETED REPORT \n\n");
fprintf(stdout, "\t Number of Tests performed:\t%d\r\n", ctest.total);
fprintf(stdout, "\t Number of Tests PASS:\t\t%d\r\n", ctest.good);
fprintf(stdout, "\t Number of Tests FAIL:\t\t%d\r\n", ctest.bad);
fprintf(stdout, "\t Number of Tests ERROR:\t%d\r\n", ctest.error);

}

/*! @brief - Function not used during normal execution
*    The function has been used to test the scpi command
*    selftest board could be required to be present   
*/
void test_command(void){
int result;
int i = 0;

struct TestResult ctest = {0,0,0,0}; // reset counter structure
out_buffer_pos =0;

TEST_SCPI_INPUT("SYST:OUT ON\r\n");        // Power selftest board

test_cmd_out("Test 1.0 SCPI Base command", "*IDN?\r\n","FirstTestStation,InterconnectIO,2022A,1.0",&ctest);
test_cmd_out("Test 1.1 SCPI Base command", "*OPC?\r\n","1",&ctest);
test_cmd_out("Test 1.2 SCPI Base command", "SYST:VERS?\r\n","1999.0",&ctest);

test_cmd_out("Test 2.0 SCPI Register command", "*STB?\r\n","0",&ctest);
test_cmd_out("Test 2.1 SCPI Register command", "*ESE?\r\n","0",&ctest);

test_cmd_out("Test 2.2 SCPI Register command", "STATus:QUEStionable:CONDition?\r\n","0",&ctest);
test_cmd_out("Test 2.3 SCPI Register command", "STATus:OPER:CONDition?\r\n","0",&ctest);
SCPI_RegSetBits(&scpi_context,SCPI_REG_QUESC,1); //Set bit on Questionable Data Register
test_cmd_out("Test 2.4 SCPI Register command", "STATus:QUEStionable:CONDition?\r\n","1",&ctest);
TEST_SCPI_INPUT("STATus:QUEStionable:ENABle 255 \n");
test_cmd_out("Test 2.5 SCPI Register command", "STATus:QUEStionable:ENABle?\r\n","255",&ctest); 
test_cmd_out("Test 2.6 SCPI Register command", "*STB?\r\n","8",&ctest);
test_cmd_out("Test 2.7 SCPI Register command", "STATus:QUEStionable:Event?\r\n","1",&ctest); 
test_cmd_out("Test 2.8 SCPI Register command", "*STB?\r\n","0",&ctest);
SCPI_RegClearBits(&scpi_context,SCPI_REG_QUESC,1); //Clear bit on Questionable Data Register

SCPI_RegSetBits(&scpi_context,SCPI_REG_OPERC,2); //Set bit on Operation Data Register
test_cmd_out("Test 2.9 SCPI Register command", "STAT:OPER:COND?\r\n","2",&ctest);
TEST_SCPI_INPUT("STATus:OPER:ENABle 2 \n"); 
test_cmd_out("Test 2.10 SCPI Register command", "STAT:OPER:ENAB?\r\n","2",&ctest);
test_cmd_out("Test 2.11 SCPI Register command", "*STB?\r\n","128",&ctest);
test_cmd_out("Test 2.12 SCPI Register command", "STATus:OPER:Event?\r\n","2",&ctest);
test_cmd_out("Test 2.13 SCPI Register command", "*STB?\r\n","0",&ctest); 
SCPI_RegClearBits(&scpi_context,SCPI_REG_OPERC,2); //Clear bit on Operation Data Register

TEST_SCPI_INPUT("SYST:SLA OFF\r\n");       // Set bit 6 of ESR register
TEST_SCPI_INPUT("SYST:OUT ON\r\n");        // Set bit 7 of ESR register
TEST_SCPI_INPUT("*ESE 255 \r\n");
test_cmd_out("Test 2.14 SCPI Register command", "*ESE?\r\n","255",&ctest); 
test_cmd_out("Test 2.15 SCPI Register command", "*STB?\r\n","32",&ctest);
test_cmd_out("Test 2.16 SCPI Register command", "*ESR?\r\n","192",&ctest); // Clear register after read
test_cmd_out("Test 2.17 SCPI Register command","*ESR?\r\n" ,"0",&ctest);   // Check register clear
TEST_SCPI_INPUT("SYST:SLA ON\r\n");       // Clear bit 6 of ESR register
TEST_SCPI_INPUT("SYST:OUT OFF\r\n");      // Clear bit 7 of ESR register

TEST_SCPI_INPUT("STATus:QUES:ENABle 7 \n"); 
SCPI_RegSetBits(&scpi_context,SCPI_REG_QUESC,1); //Set bit on Questionable Data Register
TEST_SCPI_INPUT("STATus:OPER:ENABle 7 \n"); 
SCPI_RegSetBits(&scpi_context,SCPI_REG_OPERC,2); //Set bit on Operation Data Register
test_cmd_out("Test 2.18 SCPI Register command", "*STB?\r\n","136",&ctest);
TEST_SCPI_INPUT("*CLS\r\n"); // Clear event register of Questionnable and Operation 
test_cmd_out("Test 2.19 SCPI Register command", "*STB?\r\n","0",&ctest);
SCPI_RegClearBits(&scpi_context,SCPI_REG_QUESC,1); //Clear bit on Questionable Data Register
SCPI_RegClearBits(&scpi_context,SCPI_REG_OPERC,2); //Clear bit on Operation Data Register

SCPI_RegSetBits(&scpi_context,SCPI_REG_QUESC,1); //Set bit on Questionable Data Register
test_cmd_out("Test 2.20 SCPI Register command", "*STB?\r\n","8",&ctest);
TEST_SCPI_INPUT("STAT:PRES\r\n"); // Clear event register of Questionnable
test_cmd_out("Test 2.21 SCPI Register command", "*STB?\r\n","0",&ctest);
SCPI_RegClearBits(&scpi_context,SCPI_REG_QUESC,1); //Clear bit on Questionable Data Register

test_cmd_out("Test 3.0 SCPI Error command", "SYSTEM:LED:ERR?\r\n","0",&ctest);
test_cmd_out("Test 3.1 SCPI Error command", "SYST:ERR:COUN?\r\n","0",&ctest);
SCPI_ErrorPush(&scpi_context, SCPI_ERROR_TIME_OUT); // raise error to verify error command and led
test_cmd_out("Test 3.2 SCPI Error command", "SYSTEM:LED:ERR?\r\n","1",&ctest);
test_cmd_out("Test 3.3 SCPI Error command", "SYST:ERR:COUN?\r\n","1",&ctest);
SCPI_ErrorPush(&scpi_context, SCPI_ERROR_QUERY_ERROR); // raise error to verify error command and led
test_cmd_out("Test 3.4 SCPI Error command", "SYST:ERR:COUN?\r\n","2",&ctest);
test_cmd_out("Test 3.5 SCPI Error command", "SYST:ERR?\r\n","-365,\"Time out error\"",&ctest);
test_cmd_out("Test 3.6 SCPI Error command", "SYST:ERR:NEXT?\r\n","-400,\"Query error\"",&ctest);
test_cmd_out("Test 3.7 SCPI Error command", "SYSTEM:LED:ERR?\r\n","0",&ctest);
TEST_SCPI_INPUT("SYSTEM:LED:ERR ON\r\n"); /** Turn ON red led */
test_cmd_out("Test 3.8 SCPI Error command", "SYSTEM:LED:ERR?\r\n","1",&ctest);
TEST_SCPI_INPUT("SYSTEM:LED:ERR OFF\r\n"); /** Turn OFF red led */
test_cmd_out("Test 3.9 SCPI Error command", "SYSTEM:LED:ERR?\r\n","0",&ctest);

// RElay command Check

TEST_SCPI_INPUT("ROUT:CLOSE (@100:102,201:204,303:306,404:407)\r\n");
test_cmd_out("Test 4.0 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","7,30,120,240",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN (@100,201,303,404)\r\n");
test_cmd_out("Test 4.1 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","6,28,112,224",&ctest);
TEST_SCPI_INPUT("ROUT:CLOSE:EXCL (@100,201,303,404)\r\n");
test_cmd_out("Test 4.2 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","1,2,8,16",&ctest);
TEST_SCPI_INPUT("ROUT:CLOSE (@115,215,315,415)\r\n");
test_cmd_out("Test 4.3 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","129,130,136,144",&ctest);
test_cmd_out("Test 4.4 SCPI Relay command", "ROUT:CHAN:STAT? (@115,215,315,415)\r\n","1,1,1,1",&ctest);

TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2,BANK3,BANK4\r\n");
test_cmd_out("Test 4.5 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","0,0,0,0",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN (@115,215,315,415)\r\n");
test_cmd_out("Test 4.6 SCPI Relay command", "ROUT:CHAN:STAT? (@115,215,315,415)\r\n","0,0,0,0",&ctest);
test_cmd_out("Test 4.7 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","0,0,0,0",&ctest);
TEST_SCPI_INPUT("ROUT:CLOSE (@108:115,208:215,308:315,408:415)\r\n");
test_cmd_out("Test 4.8 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","255,255,255,255",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2,BANK3,BANK4\r\n");
TEST_SCPI_INPUT("ROUT:OPEN (@115,215,315,415)\r\n"); // temporary to fix bug
test_cmd_out("Test 4.9 SCPI Relay command", "ROUT:BANK:STAT? BANK1,BANK2,BANK3,BANK4\r\n","0,0,0,0",&ctest);
TEST_SCPI_INPUT("ROUT:CLOSE:Rev BANK1,BANK2,BANK3,BANK4\r\n");
test_cmd_out("Test 4.10 SCPI Relay command", "ROUT:REV:STAT? BANK1,BANK2,BANK3,BANK4\r\n","1,1,1,1",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:Rev BANK2,BANK4\r\n"); 
test_cmd_out("Test 4.11 SCPI Relay command", "ROUT:REV:STAT? BANK1,BANK2,BANK3,BANK4\r\n","1,0,1,0",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:ALL BANK1,BANK2,BANK3,BANK4\r\n");
test_cmd_out("Test 4.12 SCPI Relay command", "ROUT:REV:STAT? BANK1,BANK2,BANK3,BANK4\r\n","0,0,0,0",&ctest);

TEST_SCPI_INPUT("ROUT:CLOSE:PWR LPR1,LPR2,HPR1,SSR1 \r\n");
test_cmd_out("Test 4.13 SCPI PWR Relay command", "ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1\r\n","1,1,1,1",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR2,SSR1 \r\n");
test_cmd_out("Test 4.14 SCPI PWR Relay command", "ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1\r\n","1,0,1,0",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:PWR LPR1,HPR1 \r\n");
test_cmd_out("Test 4.15 SCPI PWR Relay command", "ROUT:STATE:PWR? LPR1,LPR2,HPR1,SSR1\r\n","0,0,0,0",&ctest);


TEST_SCPI_INPUT("ROUT:CLOSE:OC OC1,OC2,OC3 \r\n");
test_cmd_out("Test 5.0 SCPI Open Collector command", "ROUT:STATE:OC? OC1,OC2,OC3\r\n","1,1,1",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
test_cmd_out("Test 5.1 SCPI Open Collector command", "ROUT:STATE:OC? OC1,OC2,OC3\r\n","0,1,1",&ctest);
TEST_SCPI_INPUT("ROUT:OPEN:OC OC2,OC3 \r\n");
test_cmd_out("Test 5.2 SCPI Open Collector command", "ROUT:STATE:OC? OC1,OC2,OC3\r\n","0,0,0",&ctest);


// Digital command Check
// Require presence of the selftest board ( need to be powered)

TEST_SCPI_INPUT("SYST:OUT ON\r\n");        // Power selftest board

TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n"); // set direction port0 output(1)
TEST_SCPI_INPUT("DIG:DIR:PORT0 #H00 \r\n"); // set direction port1 input(0)
TEST_SCPI_INPUT("DIG:OUT:PORT1 #H55 \r\n");
test_cmd_out("Test 6.1 SCPI Digital command", "DIG:IN:PORT0?\r\n","85",&ctest);
TEST_SCPI_INPUT("DIG:OUT:PORT1 #HAA \r\n");
test_cmd_out("Test 6.2 SCPI Digital command", "DIG:IN:PORT0?\r\n","170",&ctest);

TEST_SCPI_INPUT("DIG:DIR:PORT0 #HF0 \r\n"); // set direction port0 
test_cmd_out("Test 6.3 SCPI Digital command", "DIG:DIR:PORT0?\r\n","240",&ctest);
TEST_SCPI_INPUT("DIG:DIR:PORT1 #H0F \r\n"); // set direction port1
test_cmd_out("Test 6.4 SCPI Digital command", "DIG:DIR:PORT1?\r\n","15",&ctest);
TEST_SCPI_INPUT("DIG:OUT:PORT0 240 \r\n");
TEST_SCPI_INPUT("DIG:OUT:PORT1 0 \r\n");
test_cmd_out("Test 6.5 SCPI Digital command", "DIG:IN:PORT1?\r\n","240",&ctest); // same number due to loopback
test_cmd_out("Test 6.6 SCPI Digital command", "DIG:IN:PORT0?\r\n","240",&ctest);
TEST_SCPI_INPUT("DIG:OUT:PORT0 0 \r\n");
TEST_SCPI_INPUT("DIG:OUT:PORT1 15 \r\n");
test_cmd_out("Test 6.7 SCPI Digital command", "DIG:IN:PORT0?\r\n","15",&ctest);
test_cmd_out("Test 6.8 SCPI Digital command", "DIG:IN:PORT1?\r\n","15",&ctest);

TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); // set direction port input(0)
TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port output(1)
TEST_SCPI_INPUT("DIG:DIR:PORT1:BIT0  1\r\n"); // set direction port output(1)
TEST_SCPI_INPUT("DIG:DIR:PORT0:BIT0  0\r\n"); // set direction port input(0)
test_cmd_out("Test 6.9 SCPI Digital command", "DIG:DIR:PORT1:BIT0?\r\n","1",&ctest);
test_cmd_out("Test 6.10 SCPI Digital command", "DIG:DIR:PORT0:BIT0?\r\n","0",&ctest);
TEST_SCPI_INPUT("DIG:OUT:PORT1:BIT0 1 \r\n");
test_cmd_out("Test 6.11 SCPI Digital command", "DIG:IN:PORT0:BIT0?\r\n","1",&ctest);
TEST_SCPI_INPUT("DIG:OUT:PORT1:BIT0 0 \r\n");
test_cmd_out("Test 6.12 SCPI Digital command", "DIG:IN:PORT0:BIT0?\r\n","0",&ctest);



// Test of GPIO Command
// SYNC IO (GP22) will be used to validate most of the command.
// Note: SYNC is for future use and no supported on this version

TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  1 \r\n");  //Set direction Gp22 as output DEV0 = Master Pico
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  0 \r\n");  //Set direction Gp22 as input DEV1 = Slave1 Pico
TEST_SCPI_INPUT("GPIO:DIR:DEV2:GP22  0 \r\n");  //Set direction Gp22 as input DEV2 = Slave2 Pico
TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  0 \r\n");  //Set direction Gp22 as input DEV3 = Slave3 Pico
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  1 \r\n");  //Set Gp22 = 1
test_cmd_out("Test 7.0 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n","1",&ctest);
test_cmd_out("Test 7.1 SCPI GPIO command", "GPIO:IN:DEV2:GP22?\r\n","1",&ctest);
test_cmd_out("Test 7.2 SCPI GPIO command", "GPIO:IN:DEV3:GP22?\r\n","1",&ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  0 \r\n");  //Set Gp22 = 0
test_cmd_out("Test 7.3 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n","0",&ctest);
test_cmd_out("Test 7.4 SCPI GPIO command", "GPIO:IN:DEV2:GP22?\r\n","0",&ctest);
test_cmd_out("Test 7.5 SCPI GPIO command", "GPIO:IN:DEV3:GP22?\r\n","0",&ctest);


TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  0 \r\n");  //Set direction Gp22 as input DEV0 = Master Pico
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  0 \r\n");  //Set direction Gp22 as input DEV1 = Slave1 Pico
TEST_SCPI_INPUT("GPIO:DIR:DEV2:GP22  0 \r\n");  //Set direction Gp22 as input DEV2 = Slave2 Pico
TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  1 \r\n");  //Set direction Gp22 as output DEV3 = Slave3 Pico
TEST_SCPI_INPUT("GPIO:OUT:DEV3:GP22  1 \r\n");  //Set Gp22 = 1
test_cmd_out("Test 7.6 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n","1",&ctest); 
test_cmd_out("Test 7.7 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n","1",&ctest);
test_cmd_out("Test 7.8 SCPI GPIO command", "GPIO:IN:DEV2:GP22?\r\n","1",&ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV3:GP22  0 \r\n");  //Set Gp22 = 0
test_cmd_out("Test 7.9 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n","0",&ctest);
test_cmd_out("Test 7.10 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n","0",&ctest);
test_cmd_out("Test 7.11 SCPI GPIO command", "GPIO:IN:DEV2:GP22?\r\n","0",&ctest);
TEST_SCPI_INPUT("GPIO:DIR:DEV3:GP22  0 \r\n");  //Set direction Gp22 as input DEV3 = Slave3 Pico

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
/*  Bit 1   SCHT  Enable schnitt trigger   */
/*  Bit 0   SLF   Slew rate control 1=fast 0 = slow */

TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP22  1 \r\n");  //Set direction Gp22 as Output DEV0 = Master Pico
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP22  1 \r\n");  //Set Gp22 = 1
test_cmd_out("Test 7.12 SCPI GPIO command", "GPIO:GETP:DEV0:GP22?\r\n","86",&ctest);
test_cmd_out("Test 7.13 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n","1",&ctest); 
TEST_SCPI_INPUT("GPIO:SETP:DEV0:GP22 #H84 \r\n");  //Disable output and set pull down
test_cmd_out("Test 7.14 SCPI GPIO command", "GPIO:GETP:DEV0:GP22?\r\n","132",&ctest);
test_cmd_out("Test 7.15 SCPI GPIO command", "GPIO:IN:DEV0:GP22?\r\n","0",&ctest); // read 0 due to output disable

// Test with DEV1 to validate the I2C command
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP22  1 \r\n");  //Set direction Gp22 as Output DEV1 = Slave1 Pico
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP22  1 \r\n");  //Set Gp22 = 1
test_cmd_out("Test 7.16 SCPI GPIO command", "GPIO:GETP:DEV1:GP22?\r\n","86",&ctest);
test_cmd_out("Test 7.17 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n","1",&ctest); 
TEST_SCPI_INPUT("GPIO:SETP:DEV1:GP22 #H84 \r\n");  //Disable output and set pull down
test_cmd_out("Test 7.18 SCPI GPIO command", "GPIO:GETP:DEV1:GP22?\r\n","132",&ctest);
test_cmd_out("Test 7.19 SCPI GPIO command", "GPIO:IN:DEV1:GP22?\r\n","0",&ctest); // read 0 due to output disable


// System command test

TEST_SCPI_INPUT("SYST:BEEP\r\n");  // Generate beep and check if error is raised
test_cmd_out("Test 8.0 SCPI System command", "SYSTEM:LED:ERR?\r\n","0",&ctest); // no error after beep
TEST_SCPI_INPUT("SYST:LED:ERR 1\r\n"); 
test_cmd_out("Test 8.1 SCPI System command", "SYSTEM:LED:ERR?\r\n","1",&ctest); 
TEST_SCPI_INPUT("SYST:LED:ERR 0\r\n"); 
test_cmd_out("Test 8.2 SCPI System command", "SYSTEM:LED:ERR?\r\n","0",&ctest); 

// command to read firmware version of all Pico (Master, Slave1,Slave2,Slave3)
test_cmd_out("Test 8.3 SCPI System command", "SYSTEM:DEV:VERS?\r\n","\"1.8, 1.5, 1.5, 1.5\"",&ctest);

TEST_SCPI_INPUT("SYST:SLA OFF \r\n"); 
test_cmd_out("Test 8.4 SCPI System command", "SYSTEM:SLA?\r\n","0",&ctest);
TEST_SCPI_INPUT("SYST:SLA ON \r\n"); 
test_cmd_out("Test 8.5 SCPI System command", "SYSTEM:SLA?\r\n","1",&ctest);

TEST_SCPI_INPUT("SYST:OUT OFF \r\n"); 
test_cmd_out("Test 8.6 SCPI System command", "SYSTEM:OUT?\r\n","0",&ctest);
TEST_SCPI_INPUT("SYST:OUT ON \r\n"); 
test_cmd_out("Test 8.7 SCPI System command", "SYSTEM:OUT?\r\n","1",&ctest);

// Command to read status byte of each salves (1,2,3)
test_cmd_out("Test 8.8 SCPI System command", "SYSTEM:SLA:STA?\r\n","\"Slave1: 0x0, Slave2: 0x0, Slave3: 0x0\"",&ctest);

// ANALOG Command Validation
TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port 0 ouput
TEST_SCPI_INPUT("DIG:DIR:PORT1 #H00 \r\n"); // set direction port 1 input
TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // set port 0 to 00
TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP0 1 \r\n");  // set io to output
TEST_SCPI_INPUT("GPIO:DIR:DEV0:GP1 1 \r\n"); // set io to output
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP8 1 \r\n");  // set io to output
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP9 1 \r\n"); // set io to output
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP0 0 \r\n"); 
TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP1  0 \r\n"); 
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  0 \r\n");  
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP9  0 \r\n"); 
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP18 1 \r\n");  // set to output
TEST_SCPI_INPUT("GPIO:DIR:DEV1:GP19 0 \r\n");  // set to input
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // FLAG output
TEST_SCPI_INPUT("ROUT:OPEN:OC OC1 \r\n");
TEST_SCPI_INPUT("ROUT:OPEN:OC OC2 \r\n");
TEST_SCPI_INPUT("ROUT:OPEN:OC OC3 \r\n");

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H40 \r\n");  // Close K2
TEST_SCPI_INPUT("ANA:DAC:VOLT 3 \r\n");  // Set ouput to 3v
sleep_ms(250);
test_cmd_result("Test 9.0: Dac output @ 3Vdc, read ADC0","ANA:ADC0:VOLT? \r\n",3.0,"V", 0.4, 0.2, &ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP8  1\r\n"); // Close K13
test_cmd_result("Test 9.1: Dac output @ 3Vdc, read ADC1","ANA:ADC1:VOLT? \r\n",3.0,"V", 0.4, 0.2, &ctest);
TEST_SCPI_INPUT("ANA:DAC:SAVE  2.5 \r\n");  // Validated with Multimeter after power OFF-ON

test_cmd_result("Test 9.2: ADC, read VSYS","ANA:ADC:Vsys? \r\n",5.0,"V", 0.3, 0.4, &ctest);
test_cmd_result("Test 9.3: ADC, read TEMP","ANA:ADC:Temp? \r\n",50,"V", 30, 20, &ctest);

TEST_SCPI_INPUT("DIG:OUT:PORT0 #H00 \r\n");  // Open K2
test_cmd_result("Test 9.4: PWR, read Bus Volt ","ANA:PWR:Volt? \r\n",5,"V", 0.3, 0.2, &ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  1 \r\n");  // Close K4
test_cmd_result("Test 9.5: PWR, read Bus Volt ","ANA:PWR:Volt? \r\n",0.1,"V", 0.1, 0.2, &ctest);
test_cmd_result("Test 9.6: PWR, read Shunt mV ","ANA:PWR:Shunt? \r\n",50,"V", 10, 10, &ctest);
test_cmd_result("Test 9.7: PWR, read Pmw","ANA:PWR:Pmw? \r\n",500,"V",200, 200, &ctest);
test_cmd_result("Test 9.8: PWR, read ImA ","ANA:PWR:Ima? \r\n",500,"V", 100, 100, &ctest);
TEST_SCPI_INPUT("ANA:PWR:CAL 500,1000\r\n"); // False Calibration to have big difference
test_cmd_result("Test 9.9: PWR, read ImA ","ANA:PWR:Ima? \r\n",1000,"V", 200, 200, &ctest);
TEST_SCPI_INPUT("GPIO:OUT:DEV1:GP18  0 \r\n");  // Open K4

ina219Init(); // reset PWR device and replace good calibration data

// EEprom  Command Check

test_cmd_out("Test 10.0 SCPI EEprom command", "CFG:Read:EEPROM:STR?  'mod_option'\r\n","DAC,PWR",&ctest);
TEST_SCPI_INPUT("CFG:Write:Eeprom:STR 'mod_option,testmo'\r\n");
test_cmd_out("Test 10.1 SCPI EEprom command", "CFG:Read:EEPROM:STR? 'mod_option'\r\n","TESTMO",&ctest);
TEST_SCPI_INPUT("CFG:Write:Eeprom:Default \r\n");
test_cmd_out("Test 10.2 SCPI EEprom command", "CFG:Read:EEPROM:STR?  'mod_option'\r\n","DAC,PWR",&ctest);
TEST_SCPI_INPUT("CFG:Read:EEPROM:Full?\r\n");  // No test, just check result returned

test_cmd_out("Test 11.0 SCPI Error command", "SYSTem:ERRor?\r\n","0,\"No error\"",&ctest);

TEST_SCPI_INPUT("SYST:OUT OFF \r\n"); // turn OFF selftest power

/** PRINT FINAL REPORT AFTER TEST COMPLETION*/
fprintf(stdout, "\n\n\t SCPI COMMAND CHECK COMPLETED REPORT \n\n");
fprintf(stdout, "\t Number of Tests performed:\t%d\r\n", ctest.total);
fprintf(stdout, "\t Number of Tests PASS:\t\t%d\r\n", ctest.good);
fprintf(stdout, "\t Number of Tests FAIL:\t\t%d\r\n", ctest.bad);
fprintf(stdout, "\t Number of Tests ERROR:\t%d\r\n", ctest.error);



}

/*! @brief - Function not used during normal execution
*    The function is used to validate the hardware after parts has been soldered
*    and pico has firmware programmed      
*
*   Normally run step by step with debugger
*   and hardware verified by visual check or by measuring voltage
*/

bool test_ioboard() {
 int result;

 fprintf(stdout,"Interconnect IO Board Hardware Test\n");  // send message to debug port


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
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP11  1 \r\n");  //Beeper ON
 sleep_ms(10);
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP11  0 \r\n");  //Beeper OFF
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP19  1 \r\n");  //Red Led ON
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  1 \r\n");  //Green Led ON (OE)
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP19  0 \r\n");  //Red Led OFF
 TEST_SCPI_INPUT("GPIO:OUT:DEV0:GP28  0 \r\n");  //Green Led OFF (OE)

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
 TEST_SCPI_INPUT("DIG:DIR:PORT0 #HFF \r\n"); // set direction port 0
 TEST_SCPI_INPUT("DIG:OUT:PORT0 #HAA \r\n"); 
 TEST_SCPI_INPUT("DIG:OUT:PORT0 #H55 \r\n");  
 TEST_SCPI_INPUT("DIG:IN:PORT0? \r\n"); 

 TEST_SCPI_INPUT("DIG:DIR:PORT1 #HFF \r\n"); // set direction port 1
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

/*! @brief - Function not used during normal execution
*    The function has been used to validate the eeprom read write
*     
*    @return
*/
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

 void printbuf(uint8_t buf[], size_t len) {
  int i;
  for (i = 0; i < len; ++i) {
  if (i % 16 == 15)
      printf("%02x\n", buf[i]);
  else
      printf("%02x ", buf[i]);
  }

  // append trailing newline if there isn't one
  if (i % 16) {
  putchar('\n');
  }
 }




void test_design(void){
  int result;
  uint16_t answer[1];  // will contains the answer returned by command
  uint16_t rdata;
  uint8_t gpio = 22;
  uint8_t value = 1;


  onewiretest();

    at24cx_dev_t eeprom;

  TEST_SCPI_INPUT("SYST:OUT ON\r\n"); 
  sleep_ms(300);

  // test de SPI

  printf("SPI master example\n");


#define PICO_MASTER_SPI_SCK_PIN   2
#define PICO_MASTER_SPI_TX_PIN    3
#define PICO_MASTER_SPI_RX_PIN    4
#define PICO_MASTER_SPI_CSN_PIN   5

#define BUF_LEN 8

 // Enable SPI 0 at 1 MHz and connect to GPIOs
 spi_init(spi0, 1000 * 1000);
 gpio_set_function(PICO_MASTER_SPI_RX_PIN, GPIO_FUNC_SPI);
 gpio_set_function(PICO_MASTER_SPI_SCK_PIN, GPIO_FUNC_SPI);
 gpio_set_function(PICO_MASTER_SPI_TX_PIN, GPIO_FUNC_SPI);
 gpio_set_function(PICO_MASTER_SPI_CSN_PIN, GPIO_FUNC_SPI);

 
 //uint8_t outb[BUF_LEN] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x55,0xf0,0xaa,0x0f,0x5a,0x00,0xff,0x33};
 uint8_t outb[BUF_LEN] = {0x55,0xf0,0xaa,0x0f,0x5a,0x00,0xff,0x33};
 
 uint8_t inb[BUF_LEN*2]; // size is double to get the data out of FIFO


  
 //send and read data
 while(1){
  printf("+");
  spi_write_read_blocking(spi_default, outb, inb, BUF_LEN);
  sleep_ms(100); // wait to be sure of slave RX interrupt is complete
  spi_write_read_blocking(spi_default, 0, inb, BUF_LEN*2);
 }

bool id = true;

volatile uint8_t rev;
// search in the receive array for the first value different than 0
// not always at the same position depending of the sequence used
uint8_t pos;

  for (pos = 0; pos < BUF_LEN; pos++) {
          if (inb[pos] > 0) { break;} // first character found
  }

 // Iterate through each element of the arrays, starting at first value found on the last loop
  for (int i = 0; i < BUF_LEN; i++) {
        // If any corresponding elements are not equal, arrays are not identical
        rev = ~inb[i+pos];  // reverse data rceived from SPI slave
        if (outb[i] != rev) {  // chack validity
            fprintf(stdout,"SPI test send X to rec ~Y FAIL:  X:0x%02x, Y:0x%02x\n",outb[i],inb[i+pos]);
            id =false;
        } else {
          fprintf(stdout,"SPI test send X to rec ~Y PASS:  X:0x%02x, Y:0x%02x\n",outb[i],inb[i+pos]);
        }
  }

if (id) {
        fprintf(stdout,"SPI The data send and received are identical.\n");
} else {
        fprintf(stdout,"SPI The data send and received are different.\n");
}



 printf("UART communication example\n");
  // uart test
  // mieux de verifier pin a pin en mode GPIO avant de faire le test de communication 

  uart_init(uart0, 115200);
  gpio_set_function(12, GPIO_FUNC_UART);
  gpio_set_function(13, GPIO_FUNC_UART);
  gpio_set_function(14, GPIO_FUNC_UART); // better to test as GPIO
  gpio_set_function(15, GPIO_FUNC_UART); // better to test as GPIO
  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
  
  uart_set_hw_flow(uart0,true,true);
  uart_set_fifo_enabled(uart0,true);

  uint8_t SendData[37] = "abcdefghijklmnopqrstuvwxyz0123456789\0";
  //uint8_t SendData[] = "abcdefghijk";
  uint8_t RecData[37];
  static int crx = 0;
  static int ctx = 0;
  uint8_t ch,cb;
  uint8_t SD[] = "Test uart1\r\n";

 uart_puts(uart1,SD); // send answer to serial port
  uart_set_fifo_enabled(uart0,false); // flush fifo
  uart_set_fifo_enabled(uart0,true);


 uart_write_blocking(uart0, SendData, 37);
 uart_default_tx_wait_blocking(); // wait to complete TX

 while (uart_is_readable(uart0)) {
    RecData[ctx++] = uart_getc(uart0);
    sleep_us(50); // let time to retrieve character;
 } 

//printf("Nbrec: %d, %s\n",ctx, RecData);

// Compare the two strings (tramsmit and receive)
if (strcmp(SendData, RecData) == 0) {
        printf("The strings are identical.\n");
} else {
        printf("The strings are different.\n");
}

// end of the uart test


// i2c is the communication channel to talk to the Pico on selftest board. if communication fail,
//everything will fail.
 printf("I2C communication example\n");


  //setup_i2c_extern();
  //scan_i2c_bus(i2c0);
  scan_i2c_bus(i2c1);

  send_master(i2c1,PICO_SELFTEST_ADDRESS, DIR_GP_OUT, gpio,answer);
  send_master(i2c1,PICO_SELFTEST_ADDRESS, DIG_GP_OUT_SET, gpio,&rdata); // send command to close K17
  answer[0] = rdata;  // return read value
  send_master(i2c1,PICO_SELFTEST_ADDRESS, DIG_GP_OUT_CLEAR, gpio,&rdata); // send command to open K17
   // register eeprom 24lc32
 //at24cx_i2c_device_register(&eeprom, EEMODEL, I2C_ADDRESS_AT24CX);
}






int onewiretest() {
  uint8_t valid;
    //stdio_init_all();
   // sleep_ms(1000); // Delay to allow serial console to initialize

    fprintf(stdout,"Initializing One-Wire bus\n");
   
    sleep_ms(10);



    const char* bdinfo = "2D4CE282200000CC, 500-1010-020, 000001, J1";
    const char* bdinfo2 = "2DC1C38220000059, 500-1010-020, 000001, J2";

    const char* bdtest = "2D4CE282200000CC, 12345678,J1";
    const char* bdtest2 = "2DC1C38220000059, ABCDEFGH, J2";
  
   char* owid = NULL;
  valid = onewire_read_info(&owid,ADDR_TEST,NB_TEST,1); 
  if (valid != 0) {
     fprintf(stdout,"\nERROR READ 1-WIRE, error # %d:\n", valid);
     return valid;
  } 
 fprintf(stdout,"\nREAD TEST BEFORE: %s\n",owid);

  onewire_write_info(bdtest,ADDR_TEST);
  onewire_write_info(bdtest2,ADDR_TEST);
  fprintf(stdout,"\nREAD TEST AFTER:\n");
  onewire_read_info(&owid,ADDR_TEST,NB_TEST,1); 
  
  fprintf(stdout,"\nRESULT TEST:\n");
  SCPI_ResultText(&scpi_context,owid);
    
  onewire_write_info(bdinfo2,ADDR_INFO);
  fprintf(stdout,"\nREAD INFO:\n");
  onewire_read_info(&owid,ADDR_INFO,NB_INFO,1);
  fprintf(stdout,"\nRESULT INFO:\n");
  SCPI_ResultText(&scpi_context,owid);


  free(owid);
  

    return 0;
}

void test_com_command(void){
   int result;

   uint16_t rdata;
   uint16_t answer[1];  // will contains the answer returned by command

/*
  TEST_SCPI_INPUT("COM:OW:CHECK? 2\r\n");
  TEST_SCPI_INPUT("COM:OWIRE:WRITE '2D4CE282200000CC, 500-1010-020, 000001, J1' \r\n");
  TEST_SCPI_INPUT("COM:OWIRE:READ? 2 \r\n");
  TEST_SCPI_INPUT("COM:OWIRE:WRITE '2DC1C38220000059, 500-1010-020, 000001, J2' \r\n");
  TEST_SCPI_INPUT("COM:OWIRE:READ? 2 \r\n");
*/

  fprintf(stdout,"\nON TEST COMMAND\n");


 // test_spi_adx();

// i2c is the communication channel to talk to the Pico on selftest board. if communication fail,
//everything will fail.
 printf("I2C communication example\n");




  //setup_i2c_extern();
  //scan_i2c_bus(i2c0);
 // scan_i2c_bus(i2c1);


  printf("SPI communication example\n");

  TEST_SCPI_INPUT("SYST:OUT ON\r\n"); 
  sleep_ms(300);

//test_spi_adx();


  TEST_SCPI_INPUT("COM:SPI:D 8 \r\n");
  TEST_SCPI_INPUT("COM:SPI:M 4 \r\n");  // Mode 0 + CS at each byte
  TEST_SCPI_INPUT("COM:INIT:ENA SPI\r\n");

  send_master(i2c1,PICO_SELFTEST_ADDRESS, SET_SPI_CFG , 0b00010000,&rdata); // set SPI to 8 bit
   answer[0] = rdata;  // return read value

 // setup selftest board SPI
  send_master(i2c1,PICO_SELFTEST_ADDRESS, GET_SPI_CFG , 0,&rdata); // send command to get config
  answer[0] = rdata;  // return read value
  fprintf(stdout,"\nSPI config: 0x%x\n", answer[0]);

  send_master(i2c1,PICO_SELFTEST_ADDRESS, ENABLE_SPI , 0,&rdata); // send command to close K17
  answer[0] = rdata;  // return read value



  TEST_SCPI_INPUT("COM:SPI:WRI  #H12,#H34,#H56,#H78,#H90,#Hab,#Hcd,#Hef \r\n"); 
  TEST_SCPI_INPUT("COM:SPI:REA:LEN7 #H21 \r\n");  // return 7 values
  TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN3 #H43 \r\n"); // return 4 values



 // Test of 16 bits SPI size
  
  TEST_SCPI_INPUT("COM:SPI:D 16 \r\n");
  TEST_SCPI_INPUT("COM:SPI:M 4 \r\n");  // Mode 0 + CS at each byte
  TEST_SCPI_INPUT("COM:INIT:ENA SPI\r\n");
  send_master(i2c1,PICO_SELFTEST_ADDRESS, SET_SPI_CFG , 0b00011000,&rdata); // set SPI to 16 bit
   answer[0] = rdata;  // return read value

 // setup selftest board SPI
  send_master(i2c1,PICO_SELFTEST_ADDRESS, GET_SPI_CFG , 0,&rdata); // send command to get config
  answer[0] = rdata;  // return read value
  fprintf(stdout,"\nSPI config: 0x%x\n", answer[0]);

  send_master(i2c1,PICO_SELFTEST_ADDRESS, ENABLE_SPI , 0,&rdata); // send command to close K17
  answer[0] = rdata;  // return read value



  TEST_SCPI_INPUT("COM:SPI:WRI  #H1234,#H5678,#H90ab,#Hcdef,#H5555,#H6666,#H7777,#H8888 \r\n"); 
  TEST_SCPI_INPUT("COM:SPI:REA:LEN7 #H1234 \r\n");  // return 7 values
  TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN3 #H1234 \r\n"); // return 4 values




// TEST_SCPI_INPUT("COM:SPI:WRI 6,10\r\n");
 // TEST_SCPI_INPUT("COM:SPI:WRI #H15, 12\r\n");
gpio_set_function(1, GPIO_FUNC_SIO);
gpio_set_dir(1, GPIO_IN);
gpio_pull_up(1); 




while(1) {

// fprintf(stdout, "---------------> Press Button\r\n");

// while (!gpio_get(1)) {
 //  sleep_ms(300);
// }

 //TEST_SCPI_INPUT("COM:SPI:WRI  #H0100 \r\n");



//   TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN0 #203ABC \r\n");

 //TEST_SCPI_INPUT("COM:SPI:WRI  #H1111,#H2222,#H3333,#H4444\r\n");

 // TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN8  #H12,#H34,#H56,#H78,#H90,#Hab,#Hcd,#Hef \r\n"); 
 // TEST_SCPI_INPUT("COM:SPI:WRI  #H12,#H34,#H56,#H78,#H90,#Hab,#Hcd,#Hef \r\n");
 /// sleep_ms(1000); 
 // TEST_SCPI_INPUT("COM:SPI:WRI  #H12, #H34 \r\n");
 // sleep_ms(1000); 
 // TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN8  #H4321, #H8765\r\n");
 //TEST_SCPI_INPUT("COM:SPI:REA:LEN7 #H1234 \r\n");
 TEST_SCPI_INPUT("COM:SPI:REA:LEN3 \r\n");

  TEST_SCPI_INPUT("COM:SPI:REA:LEN0 \r\n");
 // sleep_ms(1000); 
 // TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN1 #56");
 
  sleep_ms(1000*2);

}

  TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN0 #204ABCD,#HEF,'allo',#H1234,#H567890,#Haabbccdd, #H1122334455667788  \r\n");
  TEST_SCPI_INPUT("COM:SPI:WRI:REA:LEN24?  #204ABCD \r\n");
  TEST_SCPI_INPUT("COM:SPI:WRI #06789\r\n");

  TEST_SCPI_INPUT("COM:SPI:WRITE:BYTE\r\n");
  TEST_SCPI_INPUT("COM:SPI:WRITE:WORD\r\n");






 printf("SERIAL communication example\n");

 
 // setup selftest board uart 8N1
  send_master(i2c1,PICO_SELFTEST_ADDRESS, SET_UART_PROT, 0b11001100,&rdata); // send command to close K17
  answer[0] = rdata;  // return read value
  send_master(i2c1,PICO_SELFTEST_ADDRESS, GET_UART_CFG, 0,&rdata); // send command to close K17
  answer[0] = rdata;  // return read value
  //send_master(i2c1,PICO_SELFTEST_ADDRESS,  ENABLE_UART , 0,&rdata); // send command to close K17

  send_master(i2c1,PICO_SELFTEST_ADDRESS,  DISABLE_UART , 0,&rdata); // send command to close K17

  TEST_SCPI_INPUT("COM:SERIAL:B 57600\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:P 8N1\r\n"); // error number
  TEST_SCPI_INPUT("COM:SERIAL:H OFF\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:H?\r\n");

  TEST_SCPI_INPUT("COM:INIT:ENA UART\r\n");

  TEST_SCPI_INPUT("COM:SERIAL:T 4000\n");
  TEST_SCPI_INPUT("COM:SERIAL:T?\r\n");

  TEST_SCPI_INPUT("COM:SERIAL:W  'ABCDEFGHIJKLPMNOP\n' \r\n");
  TEST_SCPI_INPUT("COM:SERIAL:R?  'test\r' \r\n");
  TEST_SCPI_INPUT("COM:SERIAL:R?  'test1\r' \r\n");
  TEST_SCPI_INPUT("COM:SERIAL:R?  'test2\r' \r\n");
  TEST_SCPI_INPUT("COM:SERIAL:R?  'test3\r' \r\n");



  TEST_SCPI_INPUT("COM:SERIAL:P 5O2\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:P?\r\n");

  TEST_SCPI_INPUT("COM:SERIAL:P 7N\r\n"); // missing value
  TEST_SCPI_INPUT("COM:SERIAL:P?\r\n");

  TEST_SCPI_INPUT("COM:SERIAL:P 9N1\r\n"); // error number
  TEST_SCPI_INPUT("COM:SERIAL:P 8,N,1\r\n"); // error number
  TEST_SCPI_INPUT("COM:SERIAL:P 6N4\r\n"); // error number
  TEST_SCPI_INPUT("COM:SERIAL:P 8D2\r\n"); // error letter





  TEST_SCPI_INPUT("COM:SERIAL:H ON\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:H?\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:H OFF\r\n");
  TEST_SCPI_INPUT("COM:SERIAL:H?\r\n");


   TEST_SCPI_INPUT("COM:SERIAL:B 230000\r\n");
   TEST_SCPI_INPUT("COM:SERIAL:B?\r\n");

  TEST_SCPI_INPUT("COM:INIT:ENA UART\r\n");
  TEST_SCPI_INPUT("COM:INIT:STAT? UART\r\n");
  TEST_SCPI_INPUT("COM:INIT:DIS UART\r\n");
  TEST_SCPI_INPUT("COM:INIT:STAT? UART\r\n");
  
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
void test_spi_adx(){

  int result;
  fprintf(stdout, "Test of SPI with  ADXL345\r\n");

  TEST_SCPI_INPUT("COM:SPI:D 8 \r\n"); // Set Databits to 8
  TEST_SCPI_INPUT("COM:SPI:M 3\r\n");  // set mode to 3
  TEST_SCPI_INPUT("COM:INIT:ENA SPI\r\n");  // Enable SPI


  TEST_SCPI_INPUT("COM:SPI:WRI  #H00, #H00 \r\n"); // Dummy write to set clock to high as default state
  TEST_SCPI_INPUT("COM:SPI:WRI  #H31, #H83 \r\n"); // set Selftest bit + 16g Range
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1 #HB1 \r\n");   //  Read bit 7 (1) + Register 0x31 


  TEST_SCPI_INPUT("COM:SPI:WRI  #H2d, #H08 \r\n");   // Write bit 7 (0) + Register 0x2d
  TEST_SCPI_INPUT("COM:SPI:WRI  #H1e, #H00 \r\n");   // Write bit 7 (0) + Register 0x1e
  TEST_SCPI_INPUT("COM:SPI:WRI  #H1f, #H00 \r\n");   // Write bit 7 (0) + Register 0x1f
  TEST_SCPI_INPUT("COM:SPI:WRI  #H20, #H05 \r\n");   // Write bit 7 (0) + Register 0x20

  TEST_SCPI_INPUT("COM:SPI:REA:LEN1 #H9e\r\n");      //  Read bit 7 (1) + Register 0x1e
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1 #HA0\r\n");      //  Read bit 7 (1) + Register 0x20

  TEST_SCPI_INPUT("COM:SPI:REA:LEN1 #H80\r\n");   // Read bit 7 (1) + Register 0x00
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1 #HAc\r\n");   // Read bit 7 (1) + Register 0x2c
  TEST_SCPI_INPUT("COM:SPI:REA:LEN1 #HB0\r\n");   // Read bit 7 (1) + Register 0x30

  while(1){

    output_buffer_clear();      // clear SCPI output result for get only results

    // launch command to read X,Y,Z acceleration
    TEST_SCPI_INPUT("COM:SPI:REA:LEN6 #Hf2\r\n"); // Read bit 7 (1) + Multiple byte bit 6 (1) + register 0x32

    uint8_t dta[6];
    memset(dta,0,6);

    // converting string answer located in output buffer to number
      int num = 0;
      size_t ind = 0;
      for (int i = 0; out_buffer[i] != '\0'; i++) { 
          if (out_buffer[i] == ',' || out_buffer[i] == '\r' || out_buffer[i] == '\n' ) {
            dta[ind] = num;
            num = 0;
            ind++;
          } else {
            num = num * 10 + (out_buffer[i] - 48); // if valid number add to the nuber
          }
      } 

    int16_t accelerometer_dta[3];
    // Unpack data
      for (int j=0;j<3;++j) {
          accelerometer_dta[j] = (int16_t)dta[2*j] + (((int16_t)dta[2*j + 1]) << 8);
        //  fprintf(stdout, "Accelerometer value j:%d, value: 0x%x\r\n",j,accelerometer_dta[j]);
      }

      fprintf(stdout, "Accelerometer X: %d\r\n",accelerometer_dta[0]);
      fprintf(stdout, "Accelerometer Y: %d\r\n",accelerometer_dta[1]);
      fprintf(stdout, "Accelerometer Z: %d\r\n",accelerometer_dta[2]);

      sleep_ms(1000 *2);

  }
  fprintf(stdout, "End of SPI Test of ADXL345\r\n");
}
