/**
 * @file    functadv.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Software code used to perform communication with the all the devices available
 *          on the board
 *
 * @details This module contains all software functions to execute low level action
 *          on device resource available on the board. The configuration
 *          eeprom is controlled by function on this module. A basic selftest of the board
 *          is also executed on this module
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include "pico/stdlib.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "pico/binary_info.h"
#include "include/functadv.h"
#include "stdio.h"
#include "string.h"
#include "include/scpi_user_config.h"
#include "include/master.h"
#include "include/fts_scpi.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "include/i2c_com.h"
#include "pico_lib2/src/dev/dev_ina219/dev_ina219.h"
#include "pico_lib2/src/dev/dev_mcp4725/dev_mcp4725.h"
#include "pico_lib2/src/dev/dev_24lc32/dev_24lc32.h"
#include "hardware/i2c.h"

// Initialize ADC function  if enable=1, else pin will be GPIO

/**
 * @brief Set the up ADC object. ADC is configured as GPIO when is disabled
 *
 * @param enable  True to use the ADC, False to configure as GPIO
 */
void setup_ADC(bool enable)
{
  if (enable)
  {  // ADC function

    adc_gpio_init(ADC0);
    adc_gpio_init(ADC1);
  }
  else
  {  // ADC0 and ADC1 are defined as GPIO function
    gpio_init(ADC0);
    gpio_init(ADC1);

    gpio_set_dir(ADC0, GPIO_IN);
    gpio_set_dir(ADC1, GPIO_IN);
  }
}

// function read ADC. Float value is returned
// Command used by SCPI
/**
 * @brief   Function to read ADC value and return the result. Function called by a
 *          SCPI command
 *
 * @param channel  ADC channel to read
 * @return float   Float value read from ADC channel
 */

float read_master_adc(uint8_t channel)
{
  const float cfactor = ADC_REF / (1 << 12);  // 12 Bits conversion
  uint16_t value;
  float adc_val;

  adc_select_input(channel);
  value = adc_read();  // read ADC
  adc_val = value * cfactor;

  switch (channel)
  {
    case 0:  // ADC channel 0
      fprintf(stdout, "ADC0: Raw value: 0x%03x, voltage: %f V\n", value, adc_val);
      break;
    case 1:  // ADC channel 1
      fprintf(stdout, "ADC1: Raw value: 0x%03x, voltage: %f V\n", value, adc_val);
      break;
    case 2:  // Not used as analog channel (only ADC0 and 1)
      adc_val = 0;
      fprintf(stdout, "ADC2: is not allowed \n");
      break;
    case 3:                   // Vsys value
      adc_val = adc_val * 3;  // Pico has voltage divider as input
      fprintf(stdout, "Raw value 3: 0x%03x, Vsys  voltage: %f V\n", value, adc_val);
      break;
    case 4:                                         // Master internal temperature
      adc_val = 27 - (adc_val - 0.706) / 0.001721;  // from RP2040 Datasheet
      fprintf(stdout, "Raw value 0: 0x%03x, Temperature: %f C\n", value, adc_val);
      break;
  }
  return adc_val;
}

// SCPI function to control the power device IN219
/**
 * @brief  function to read value from the I2C devices INA219 (current/power monitor). Function called by a
 *         SCPI command
 *
 * @param mode      Number to select which register value to read
 * @return float    Float value read from the device
 */
float read_power(uint8_t mode)
{
  int16_t readv;
  char meas[3] = {0, 0, 0};
  char rmd[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  switch (mode)
  {
    case 0:
      readv = ina219GetBusVoltage() * 0.001;  // read bus voltage
      strcpy(meas, "V");
      strcpy(rmd, "BUS V");
      break;

    case 1:
      readv = ina219GetCurrent_mA();  // read bus current.
      strcpy(meas, "mA");
      strcpy(rmd, "CURRENT");
      break;

    case 2:
      readv = ina219GetPower_mW();
      strcpy(meas, "mW");
      strcpy(rmd, "POWER");
      break;
    case 3:

      readv = ina219GetShuntVoltage() * 10E-3;
      strcpy(meas, "mV");
      strcpy(rmd, "SHUNT");
      break;
  }

  fprintf(stdout, "INA219,read: %s ,  value: %d %s \n", rmd, readv, meas);
  return readv;
}

/**
 * @brief SCPI function to calibrate the current on the power device INA219
 *
 * @param actual    Current value read with the hardware
 * @param expected  Expected value according to calculation
 */
void calibrate_power(float actual, float expected)
{
  bool flg;

  flg = ina219CalibrateCurrent_mA(actual, expected);
  if (flg)
  {
    fprintf(stdout, "INA219,calibration current, actual value: %.2f, expected value: %.2f \n", actual, expected);
  }
  else
  {
    fprintf(stdout, "INA219,calibration not performed, cal factor identical, actual value: %.2f, expected value: %.2f \n", actual, expected);
  }
}

/**
 * @brief Function used by SCPI command to set DAC voltage
 *        The voltage value is validated to be in the range of DAC before set the DAC
 *
 * @param value Value in volt to set the DAC
 * @param save  Flag to save voltage value as default at power ON
 * @return      Number to indicate success or error in the execution
 */

uint8_t dac_set(float value, bool save)
{
  uint16_t error;
  float ovalue;
  bool flag;

  ovalue = value;
  error = NOERR;

  // float value;

  if (value > MAXDACVOLT)
  {
    value = MAXDACVOLT;
    error = EOOR;  // raise error due to value outside maximum limit
  }
  if (value < MINDACVOLT)
  {
    value = MINDACVOLT;
    error = EOOR;
  }

  if (save)
  {
    flag = dev_mcp4725_save(i2c0, MCP4725_ADDR0, value);
  }
  else
  {
    flag = dev_mcp4725_set(i2c0, MCP4725_ADDR0, value);
  }

  if (!flag)
  {
    fprintf(stdout, "DAC Error on set MCP4725\n");
    error = EDE;
  }
  else
  {
    fprintf(stdout, "DAC voltage set to: %2.3f V\n", value);
  }
  return error;
}

/**
 * @brief This function check if the eeprom is detected and if the data is valid
 *
 * @param check_data Flag to indicate if the check number need to be validated
 * @param eeprom     Pointer to eeprom structure
 * @return uint8_t  Number to indicate success or error in the execution
 */

uint8_t eeprom_data_valid(bool check_data, at24cx_dev_t* eeprom)
{
  at24cx_writedata_t dt;

  // register eeprom 24lc32
  at24cx_i2c_device_register(eeprom, EEMODEL, I2C_ADDRESS_AT24CX);

  // Check if eeprom is active
  fprintf(stdout, "eeprom is %s\n", (*eeprom).status ? "detected" : "not detected");
  if ((*eeprom).status == false) return EDE;

  if (check_data)
  {  // if required to check data
    dt.address = ADD_EEPROM_BASE;
    if (at24cx_i2c_byte_read(*eeprom, &dt) == AT24CX_OK)
    {
      if (dt.data != EE_CHECK_CHAR)
      {  // Error Check byte not written
        fprintf(stdout, "Error Check Character do not match, expect: 0x%02X read: 0x%02X \n", EE_CHECK_CHAR, dt.data);
        return ECE;
      }
      else
      {
        fprintf(stdout, "EEprom check byte valid: 0x%02X \n", dt.data);
      }
    }
    else
    {
      fprintf(stdout, "Device byte read error!\n");
      return EBE;
    }
  }
  return NOERR;
}

// Write parameter to eeprom
/**
 * @brief  Function to read and write data on the I2C configuration eeprom
 *
 * @param mode      Character to define if we read (r) or write (w) data
 * @param eeaddr    Eeprom address to define the beginning of data string
 * @param eedatalen Number of character reserved for the data field on eeprom
 * @param data      Pointer to the data to read or write
 * @param datalen   Length of the data to read or write
 * @return uint8_t
 */

uint8_t cfg_eeprom_rw(char mode, uint32_t eeaddr, uint8_t eedatalen, char* data, uint8_t datalen)
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  uint8_t status;

  // EEprom check access and validity
  status = eeprom_data_valid(true, &eeprom_1);
  if (status != NOERR)
  {
    return status;
  }  // if error do not execute read or write on eeprom

  if (mode == 'w')
  {
    fprintf(stdout, "\nWrite Eeprom parameter\n");
    if (datalen > eedatalen)
    {  // if data is longer than reserved field
      fprintf(stdout, "Error, data to write is too long, field length 0x%02X : Data length 0x%02X \n", eedatalen, datalen);
      return EOOR;  // raise error due to value outside maximum limit
    }
    for (int i = 0; i < eedatalen; i++)
    {
      dt.address = ADD_EEPROM_BASE + eeaddr + i;  // physical address on eeprom
      dt.data = data[i];
      dt.data_multi[i] = data[i];  // save value on array to be compare after eeprom

      if (at24cx_i2c_byte_write(eeprom_1, dt) == AT24CX_OK)
      {
        fprintf(stdout, "Writing at address 0x%02X: 0x%02X , %c \n", dt.address, dt.data, dt.data);
      }
      else
      {
        fprintf(stdout, "EEprom device write byte error! \n");
        return EDE;
      }
    }
  }
  fprintf(stdout, "\nRead eeprom byte test\n");
  for (int i = 0; i < eedatalen; i++)
  {
    dt.address = ADD_EEPROM_BASE + eeaddr + i;
    if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK)
    {
      fprintf(stdout, "Reading at address 0x%02X: 0x%02X , %c \n", dt.address, dt.data, dt.data);
      data[i] = dt.data;  // save value on array
    }
    else
    {
      fprintf(stdout, "EEprom device byte read error!\n");
      return EDE;
    }
  }

  if (mode == 'w')
  {  // if data written on eeprom, compare value.
    fprintf(stdout, "\nCompare EEprom Write and read\n");
    for (int i = 0; i < eedatalen; i++)
    {
      dt.address = ADD_EEPROM_BASE + eeaddr + i;
      if (data[i] != dt.data_multi[i])
      {
        fprintf(stdout, "Error byte Write-read at address 0x%02X: write value 0x%02X: read value: 0x%02X\n", dt.address, dt.data_multi[i], data[i]);
        return ECE;
      }
    }
    fprintf(stdout, "Eeprom data match\n");
  }

  return NOERR;
}

/**
 * @brief Read the entire contents of the configuration eeprom and store data on eeprom config structure
 *        EEprom structure is global (defined on master.h)
 *
 * @return uint8_t Number to define success or failure
 */
uint8_t cfg_eeprom_read_full()
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  uint32_t datalen;
  uint8_t status;

  // EEprom check access and validity of the check byte
  status = eeprom_data_valid(true, &eeprom_1);
  if (status != NOERR)
  {
    return status;
  }  // if error do not execute read or write on eeprom

  datalen = sizeof(ee.cfg);  // read size of eeprom global structure

  fprintf(stdout, "\n--> Read full eeprom\n");
  for (int i = 0; i < datalen; i++)
  {
    dt.address = ADD_EEPROM_BASE + i;  // calculate physical address
    if (at24cx_i2c_byte_read(eeprom_1, &dt) == AT24CX_OK)
    {
      fprintf(stdout, "Full Eeprom reading byte #%d at address 0x%02X: 0x%02X,", i, dt.address, dt.data);
      if (dt.data == 0x00)
      {
        fprintf(stdout, "\n");
      }
      else
      {
        fprintf(stdout, "%c\n", dt.data);
      }
      ee.data[i] = dt.data;  // save value on eeprom structure
    }
    else
    {
      fprintf(stdout, "EEprom read full device byte read error!\n");
      return EDE;
    }
  }
  fprintf(stdout, "\n--> Completed read of full eeprom\n");
  return NOERR;
}

/**
 * @brief Write the default values on EEprom. EEprom is written by page
 *        EEprom structure is global (defined on master.h)
 *
 * @return uint8_t Number to define success or failure
 */

uint8_t cfg_eeprom_write_default()
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  uint32_t datalen;
  uint32_t addr;
  int i;
  uint8_t status;

  eep eed = DEF_EEPROM;  // Assign default value to structure eeprom

  // EEprom check access
  status = eeprom_data_valid(false, &eeprom_1);
  if (status != NOERR)
  {
    return status;
  }  // if error do not execute read or write on eeprom

  datalen = sizeof(ee.cfg);  // read size of eeprom global structure

  fprintf(stdout, "\n--> Write Default value on eeprom\n");
  dt.address = ADD_EEPROM_BASE;  // Set base address
  i = 0;                         // index for the location of data

  while (0 < datalen)  // loop to write eeprom data by page
  {
    uint32_t pagelen = (dt.address | (EE_PAGESIZE - 1)) - dt.address + 1;
    uint32_t writelen = min(datalen, pagelen);

    memcpy(&dt.data_multi[0], &eed.data[i], writelen);  // copy data in Eeprom array to write

    if (at24cx_i2c_page_write(eeprom_1, dt) == AT24CX_OK)
      fprintf(stdout, "Page Writing at address 0x%02X\n", dt.address);
    else
    {
      fprintf(stdout, "Device page write error!\n");
      return EDE;
    }

    dt.address += writelen;
    i += writelen;
    datalen -= writelen;
  }
  fprintf(stdout, "EEprom Writing Completed\n");
  return NOERR;
}

/**
 * @brief Function to extract number from string. Mainly used to convert EEprom Cfg string to number
 *
 * @param str    Pointer to string of character to transform
 * @param lgs    Length of the string
 * @param result Number converted from the string
 * @return uint8_t Number to define success or failure
 */

uint8_t stringtonumber(const char* str, size_t lgs, long* result)
{
  char strc[30];

  strncpy(strc, str, lgs);
  strc[lgs] = '\0';  // Null-terminate the copied string

  char* endPtr;

  errno = 0;  // Reset errno before calling strtol

  *result = strtol(strc, &endPtr, 10);

  // Check for errors during conversion
  if ((errno == ERANGE && (*result == LONG_MAX || *result == LONG_MIN)) || (errno != 0 && *result == 0))
  {
    return -1;  // Error occurred during conversion
  }

  // Check if the entire string was converted
  if (strlen(endPtr) > 0)
  {
    fprintf(stdout, "Error in string to number conversion, could not convert: %s\n", endPtr);
    return -1;  // one of the characters is not a number
  }

  return 0;  // Successful conversion
}

/**
 * @brief Perform Boot check by validating i2C device in the chain
 *
 * @return true  if success with the boot validation
 * @return false if fail the boot validation
 */
bool Boot_check()
{
  uint8_t ret;
  uint8_t rxdata;

  gpio_put(GPIO_RUN, 1);  // Start PICO Slave (if required)

  scan_i2c_bus(i2c0);  // send devices detected on the debug port (USB)
  ret = 0;
  ret += i2c_read_blocking(i2c0, I2C_ADDRESS_AT24CX, &rxdata, 1, false);   // check I2C com with eeprom
  ret += i2c_read_blocking(i2c0, PICO_PORT_ADDRESS, &rxdata, 1, false);    // check I2C com with Pico  Slave_1
  ret += i2c_read_blocking(i2c0, PICO_RELAY1_ADDRESS, &rxdata, 1, false);  // check I2C com with Pico  Slave_2
  ret += i2c_read_blocking(i2c0, PICO_RELAY2_ADDRESS, &rxdata, 1, false);  // check I2C com with Pico  Slave_3
  ret += i2c_read_blocking(i2c0, INA219_ADDRESS, &rxdata, 1, false);       // check I2C com with PWR device
  ret += i2c_read_blocking(i2c0, MCP4725_ADDR0, &rxdata, 1, false);        // check I2C com with DAC device

  if (ret == 6)
  {  // if all I2C device detected, return true, else return false
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * @brief  Run internal board tests to verifies hardware functionality,
 */
bool IOBoard_Selftest()
{
  at24cx_dev_t eeprom_1;
  at24cx_writedata_t dt;
  volatile uint8_t ret;
  char* err[12];       // contains error found during selftest
  uint16_t answer[3];  // container for slave device status test
  char pv[80];         // temporary container for error string
  bool result;
  uint8_t dataw;
  int datar;  // used on eeprom test
  double value, l, h;

  size_t i = 0;  // index for arrays of err

  /*******************************************************************************/
  // Selftest Master Pico internal parameters
  /*******************************************************************************/

  // Check master VSYS voltage. Raise error if value is too high or too low
  value = read_master_adc(3);  // Read VSYS,  Expect 5V
  result = (value > MAX_VSYS_VOLT || value < MIN_VSYS_VOLT) ? FALSE : TRUE;
  RegBitHdwrErr(VSYS_OUT, result);  // Set or clear Questionable register based on results
  if (!result)
  {
    l = MIN_VSYS_VOLT;
    h = MAX_VSYS_VOLT;  // get constant value
    sprintf(pv, "Pico Master VSYS out of limits, read: %2.2fV, Low: %2.2fV, High %2.2fV", value, l, h);
    err[i++] = strdup(pv);
  }

  // Check master temperature. Raise error if value is too high
  value = read_master_adc(4);  // Read TEMP,  Expect 25
  result = (value > MAX_PICO_TEMP) ? FALSE : TRUE;
  RegBitHdwrErr(MTEMP_HIGH, result);  // Set or clear Questionable register based on results
  if (!result)
  {
    h = MAX_PICO_TEMP;
    sprintf(pv, "Pico Master Temperature exceed the limits, read: %2.2f C, Max High %2.2f C", value, h);
    err[i++] = strdup(pv);
  }

  /*******************************************************************************/
  // Selftest Pico Slaves
  /*******************************************************************************/

  gpio_put(GPIO_RUN, 1);                  // Start PICO Slave (if required)
  result = system_execute(GSTA, answer);  // send selftest command to pico slaves
  if (result == TRUE)
  {
    if (answer[0] > 0)
    {
      sprintf(pv, "Pico Slave1 Selftest error, expect 0x0 read:  0x%x", answer[0]);
      err[i++] = strdup(pv);
    }
    if (answer[1] > 0)
    {
      sprintf(pv, "Pico Slave2 Selftest error, expect 0x0 read:  0x%x", answer[1]);
      err[i++] = strdup(pv);
    }
    if (answer[2] > 0)
    {
      sprintf(pv, "Pico Slave3 Selftest error, expect 0x0 read:  0x%x", answer[2]);
      err[i++] = strdup(pv);
    }
  }
  else
  {  // if fail to communicate, check i2c comm

    datar = i2c_read_blocking(i2c0, PICO_PORT_ADDRESS, &dataw, 1, false);  // check I2C com with Pico  Slave_1
    if (datar < 0)
    {
      sprintf(pv, "Pico Slave1 communication I2C error");
      err[i++] = strdup(pv);
    }

    datar = i2c_read_blocking(i2c0, PICO_RELAY1_ADDRESS, &dataw, 1, false);  // check I2C com with Pico  Slave_1
    if (datar < 0)
    {
      sprintf(pv, "Pico Slave2 communication I2C error");
      err[i++] = strdup(pv);
    }

    datar = i2c_read_blocking(i2c0, PICO_RELAY2_ADDRESS, &dataw, 1, false);  // check I2C com with Pico  Slave_1
    if (datar < 0)
    {
      sprintf(pv, "Pico Slave3 communication I2C error");
      err[i++] = strdup(pv);
    }

    if (i == 0)
    {
      sprintf(pv, "Pico Slave unknow error with com");
      err[i++] = strdup(pv);
    }
  }

  /*******************************************************************************/
  // Selftest EEprom
  /*******************************************************************************/

  ret = eeprom_data_valid(TRUE, &eeprom_1);
  switch (ret)
  {
    case EDE:  // EEprom not detected
      sprintf(pv, "Cfg EEprom not detected");
      err[i++] = strdup(pv);
      break;

    case ECE:  // EEprom Check character do not match
      sprintf(pv, "Cfg EEprom data corrupt");
      err[i++] = strdup(pv);
      break;

    case EBE:  // EEprom read byte error
      sprintf(pv, "Cfg EEprom byte error");
      err[i++] = strdup(pv);
      break;

    case NOERR:
      // Verify if possible to write and read data without error
      dataw = 0x5A;  // Byte to write to test R/W
      dt.data = dataw;
      dt.address = TEST_EEPROM_ADD;  // address 0 reserved for test data

      if (at24cx_i2c_byte_write(eeprom_1, dt) != AT24CX_OK)
      {
        sprintf(pv, "Cfg EEprom byte write error at test address 0x0000");
        err[i++] = strdup(pv);
      }
      if (at24cx_i2c_byte_read(eeprom_1, &dt) != AT24CX_OK)
      {
        sprintf(pv, "Cfg EEprom byte read error at test address 0x0000");
        err[i++] = strdup(pv);
      }

      if (dataw != dt.data)
      {  // verify if data written == data read
        sprintf(pv, "EEprom byte error. Byte Write 0x%x, Byte read 0x%x", dataw, dt.data);
        err[i++] = strdup(pv);
      }
  }

  /*******************************************************************************/
  // Selftest PWR (INA219)
  /*******************************************************************************/
  datar = i2c_read_blocking(i2c0, INA219_ADDRESS, &dataw, 1, false);  // check I2C com with Pico  Slave_1
  if (datar < 0)
  {
    sprintf(pv, "I2C com error with CURRENT MONITOR module (INA219)");
    err[i++] = strdup(pv);
  }

  answer[0] = ina219Init();
  int defina = DEFAULT_PWR_VAL;
  if (answer[0] != defina)
  {
    sprintf(pv, "CURRENT MONITOR module (INA219) Default value error, expect: 0x%x, read: 0x%x", defina, answer[0]);
    err[i++] = strdup(pv);
  }

  /*******************************************************************************/
  // Selftest DAC (MCP4725)
  /*******************************************************************************/
  float valuei;
  float valuew = 3.25;  // Dac value to use to program DAC for selftest
  float valuer;         // value read back

  datar = i2c_read_blocking(i2c0, MCP4725_ADDR0, &dataw, 1, false);  // check I2C com with Pico  Slave_1
  if (datar < 0)
  {
    sprintf(pv, "I2C com error with DAC module (MCP4725)");
    err[i++] = strdup(pv);
  }
  else
  {
    valuei = dev_mcp4725_get(i2c0, MCP4725_ADDR0);  // read actual value of DAC
    ret = dev_mcp4725_set(i2c0, MCP4725_ADDR0, valuew);
    if (ret != 1)
    {  // if error found
      sprintf(pv, "Error on set voltage using DAC module (MCP4725).  Error# %d", ret);
      err[i++] = strdup(pv);
    }
    else
    {
      valuer = dev_mcp4725_get(i2c0, MCP4725_ADDR0);  // read value programmed
      if (valuer < valuew - 0.05 || valuer > valuew + 0.05)
      {  // validate read back
        sprintf(pv, "DAC Error on set voltage, Set Volt: %2.3fV, readback: %2.3fV", valuew, valuer);
        err[i++] = strdup(pv);
      }
      else
      {
        dev_mcp4725_set(i2c0, MCP4725_ADDR0, valuei);
      }  // reprogrammed original value
    }
  }

  /*******************************************************************************/
  // Return Selftest results
  /*******************************************************************************/
  // in case of error, send message and raise register bits
  if (i > 0)
  {                                                                           // if error found
    SCPI_ErrorPush(&scpi_context, SELFTEST_FAIL);                             // push error
    SCPI_RegSetBits(&scpi_context, SCPI_REG_OPERC, 1 << OPER_SELFTEST_FAIL);  // ser register bits
    for (size_t j = 0; j < i; j++)
    {
      SCPI_ResultText(&scpi_context, err[j]);  // send error to operator
      free(err[j]);                            // free memory after error send to operator
    }
  }
  else
  {
    SCPI_ResultText(&scpi_context, "OK");
    SCPI_RegClearBits(&scpi_context, SCPI_REG_OPERC, 0 << OPER_SELFTEST_FAIL);
  }

  return TRUE;
}

/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Sweep through all 7-bit I2C addresses, to see if any slaves are present on
// the I2C bus. Print out a table that looks like this:
//
// I2C Bus Scan
//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
// 0
// 1       @
// 2
// 3             @
// 4
// 5
// 6
// 7
//
// E.g. if slave addresses 0x12 and 0x34 were acknowledged.

//#include <stdio.h>
//#include "pico/stdlib.h"
//#include "pico/binary_info.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx

/**
 * @brief   function to check if the I2C address required to check if a reserved address
 *
 * @param addr Address to validate
 * @return true Address is a reserved address
 * @return false Address is valid
 */

bool reserved_addr(uint8_t addr)
{
  return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

/**
 * @brief Function found on internet to scan the internal I2C bus.
 *        Function called during boot up of the master pico and information printed on USB.
 *
 * @param i2c Define which I2C is scanned
 */

void scan_i2c_bus(i2c_inst_t* i2c)
{
  printf("\nI2C Bus Scan\n");
  printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

  for (int addr = 0; addr < (1 << 7); ++addr)
  {
    if (addr % 16 == 0)
    {
      printf("%02x ", addr);
    }

    // Perform a 1-byte dummy read from the probe address. If a slave
    // acknowledges this address, the function returns the number of bytes
    // transferred. If the address byte is ignored, the function returns
    // -1.

    // Skip over any reserved addresses.
    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr))
      ret = PICO_ERROR_GENERIC;
    else
      ret = i2c_read_blocking(i2c, addr, &rxdata, 1, false);

    printf(ret < 0 ? "." : "*");
    printf(addr % 16 == 15 ? "\n" : "  ");
  }
  printf("Done.\n");
  // return 0;
}
