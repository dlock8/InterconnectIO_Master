/**
 * @file    dev_ds2431.c
 * @author  Daniel Lockhead
 * @date    2024
 *
 * @brief   Driver to perform communication with OneWire device
 *
 *
 * @details OneWire driver to read / write on the EEprom of the Onewire Device. Plan to be used
 *          to detect what's is connected to Interconnect IO Board before turning the main power.
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "dev_ds2431.h"

/**
 * @brief List of OneWire chip models.
 *
 * This array contains information about the various OneWire chip models, including their
 * unique identifier, name, memory size, number of pages, and a boolean indicating
 * whether they support a certain feature.
 */
model_type _chip_model_list[] = {
    {0x09, "DS2502", 4, 2, true},   /**< Chip model: DS2502, identifier: 0x09, pages: 4, size: 2 bytes, supports feature. */
    {0x0B, "DS2505", 64, 2, true},  /**< Chip model: DS2505, identifier: 0x0B, pages: 64, size: 2 bytes, supports feature. */
    {0x14, "DS2430", 1, 1, false},  /**< Chip model: DS2430, identifier: 0x14, pages: 1, size: 1 byte, does not support feature. */
    {0x2D, "DS2431", 4, 2, false},  /**< Chip model: DS2431, identifier: 0x2D, pages: 4, size: 2 bytes, does not support feature. */
    {0x23, "DS2433", 16, 2, false}, /**< Chip model: DS2433, identifier: 0x23, pages: 16, size: 2 bytes, does not support feature. */
    {0, 0, 0, 0, 0}                 /**< Terminator for the list. */
};

/**
 * @brief Global search state variables for OneWire devices.
 */

uint8_t ROM_NO[8];         /**< Array to store the ROM number of the found OneWire device. */
int LastDiscrepancy;       /**< Last discrepancy found during the ROM search process. */
int LastFamilyDiscrepancy; /**< Last discrepancy for the family code during the search. */
bool LastDeviceFlag;       /**< Flag indicating if the last device has been found in the search. */

/**
 * @brief Function to send a One-Wire reset pulse and detect if Onewire respond (presence)
 *
 *
 * @return true     Onewire device is present
 * @return false
 */
static bool onewire_reset()
{
  gpio_put(ONEWIRE_PIN, 0);                // Pull the bus low when GPIO = OUT
  sleep_us(2);                             // Hold for 2us
  gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);     // Release the bus and transmit the bit
  sleep_us(480);                           // Wait for 480us
  gpio_set_dir(ONEWIRE_PIN, GPIO_IN);      // Release the bus and wait for the device to pull it low
  sleep_us(65);                            // Wait for 60us
  bool presence = !gpio_get(ONEWIRE_PIN);  // Check if the device pulled the bus low
  sleep_us(440);                           // Wait for the remaining 420us
  return presence;
}

/**
 * @brief Function to send a One-Wire write bit
 *
 * @param bit   Value of bit to write
 */
static void onewire_write_bit(bool bit)
{
  gpio_put(ONEWIRE_PIN, 0);             // Preset to Pull the bus low when set to GPIO_OUT
  sleep_us(2);                          // Hold for 2us
  gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);  // Release the bus and transmit the bit
  sleep_us(bit ? 8 : 58);               // Write 0: Hold for 58us, Write 1: Hold for 8us
  gpio_set_dir(ONEWIRE_PIN, GPIO_IN);   // Release the bus and wait for the device to pull it low
  sleep_us(bit ? 64 : 10);              // Sample at 64us for recovery, 10us for the next bit
}

/**
 * @brief Function to send a One-Wire write byte
 *
 * @param byte   Byte to write
 */
static void onewire_write_byte(uint8_t byte)
{
  // loop to write each bit of the bytes
  for (int i = 0; i < 8; i++)
  {
    onewire_write_bit(byte & (1 << i));
  }
}

/**
 * @brief Function to read a OneWire bit
 *
 * @return true     Bit = 1
 * @return false    Bit = 0
 */
static bool onewire_read_bit()
{
  gpio_put(ONEWIRE_PIN, 0);             // Pull the bus low
  sleep_us(2);                          // Hold for 2us
  gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);  // Release the bus and transmit a read slot
  sleep_us(6);                          // Hold for 6us
  gpio_set_dir(ONEWIRE_PIN, GPIO_IN);   // Release the bus and sample the bit
  sleep_us(12);                         // Sample at 9us
  bool bit = gpio_get(ONEWIRE_PIN);     // Read the sampled bit
  sleep_us(55);                         // Wait for the next bit
  return bit;
}

/**
 * @brief Function to send a One-Wire read byte
 *
 * @return  Byte read
 */
static uint8_t onewire_read_byte()
{
  uint8_t byte = 0;
  for (int i = 0; i < 8; i++)
  {
    byte |= (onewire_read_bit() << i);
  }
  return byte;
}

/**
 * @brief Initialize the search state.
 *
 */
static void onewire_search_reset(void)
{
  LastDiscrepancy = 0;
  LastFamilyDiscrepancy = 0;
  LastDeviceFlag = false;
  for (int i = 0; i < 8; i++)
  {
    ROM_NO[i] = 0;
  }
}

/**
 * @brief Perform the OneWire Search Algorithm on the OneWire bus and return the next device found.
 *
 * @param  device_id    return next device found
 */
static bool onewire_search(uint8_t* device_id)
{
  int id_bit_number;
  int last_zero, rom_byte_number, search_result;
  bool id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;

  // Initialize for search
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;

  // If the last call was not the last one
  if (!LastDeviceFlag)
  {
    // OneWire reset
    if (!onewire_reset())
    {
      // Reset the search
      LastDiscrepancy = 0;
      LastFamilyDiscrepancy = 0;
      LastDeviceFlag = false;
      return false;
    }

    // Issue the search command
    onewire_write_byte(SEARCH_ROM);

    // Loop to do the search
    do
    {
      // Read a bit and its complement
      id_bit = onewire_read_bit();
      cmp_id_bit = onewire_read_bit();

      // Check for no devices on Onewire
      if (id_bit && cmp_id_bit)
      {
        break;
      }
      else
      {
        // All devices coupled have 0 or 1
        if (id_bit != cmp_id_bit)
        {
          search_direction = id_bit;  // Bit write value for search
        }
        else
        {
          // If this discrepancy is before the Last Discrepancy
          // on a previous next then pick the same as last time
          if (id_bit_number < LastDiscrepancy)
          {
            search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
          }
          else
          {
            // If equal to last pick 1, if not then pick 0
            search_direction = (id_bit_number == LastDiscrepancy);
          }
          // If 0 was picked then record its position in LastZero
          if (search_direction == 0)
          {
            last_zero = id_bit_number;

            // Check for Last discrepancy in family
            if (last_zero < 9)
            {
              LastFamilyDiscrepancy = last_zero;
            }
          }
        }

        // Set or clear the bit in the ROM byte rom_byte_number
        // with mask rom_byte_mask
        if (search_direction == 1)
        {
          ROM_NO[rom_byte_number] |= rom_byte_mask;
        }
        else
        {
          ROM_NO[rom_byte_number] &= ~rom_byte_mask;
        }

        // Serial number search direction write bit
        onewire_write_bit(search_direction);

        // Increment the byte counter id_bit_number
        // and shift the mask rom_byte_mask
        id_bit_number++;
        rom_byte_mask <<= 1;

        // If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
        if (rom_byte_mask == 0)
        {
          rom_byte_number++;
          rom_byte_mask = 1;
        }
      }
    } while (rom_byte_number < 8);  // Loop until through all ROM bytes 0-7

    // If the search was successful then
    if (!(id_bit_number < 65))
    {
      // Search successful so set LastDiscrepancy,LastDeviceFlag,search_result
      LastDiscrepancy = last_zero;

      // Check for last device
      if (LastDiscrepancy == 0)
      {
        LastDeviceFlag = true;
      }

      search_result = 1;
    }
  }

  // If no device found then reset counters so next 'search' will be like a first
  if (!search_result || !ROM_NO[0])
  {
    LastDiscrepancy = 0;
    LastFamilyDiscrepancy = 0;
    LastDeviceFlag = false;
    search_result = false;
  }
  else
  {
    for (int i = 0; i < 8; i++)
    {
      device_id[i] = ROM_NO[i];
    }
  }
  return search_result;
}

/**
 * @brief Function to initialize the GPIO used to read onewire device
 *
 *      GPIO is controlled like open collector device
 *      At initial start, we drive 1 for charge capacitors inside OneWire devices
 *      faster then using only pull-up method
 */
static void onewire_init()
{
  gpio_set_function(ONEWIRE_PIN, GPIO_FUNC_SIO);
  gpio_put(ONEWIRE_PIN, 1);
  gpio_set_dir(ONEWIRE_PIN, GPIO_OUT);
}

/**
 * @brief Selects the OneWire device by writing its 64-bit ID to the bus.
 *
 * This function sends the MATCHROM command followed by the device ID
 * bytes to select a specific OneWire device for subsequent read or
 * write operations.
 *
 * @param id Pointer to an array containing the 8 bytes of the device ID.
 */
void onewire_select(const uint8_t* id)
{
  onewire_write_byte(MATCHROM);

  for (int i = 0; i < 8; i++)
  {
    onewire_write_byte(id[i]);
  }
}

/**
 * @brief  Read OneWire lasered ROM ID
 *         Not normally used in normal operation
 *
 * @param id       Array who will contains the ID
 * @param device_num  Which device
 *
 * @return true     No Error
 * @return false    Error
 */
static bool read_eeprom_id(uint8_t id[])
{
  if (!onewire_reset())
  {
    printf("OneWire Device not found during read_id\n");
    return false;
  }
  onewire_write_byte(READROM);  // Read data command
  for (int i = 0; i < 8; i++)
  {
    id[i] = onewire_read_byte();  // Read ID bytes
  }
  return true;
}

/**
 * @brief function to read the Onewire device eeprom
 *
 * @param device_id     Array of 8 numbers who identify the device
 * @param data_buffer   The data buffer who contains the data read from eeprom
 * @param start_address The beginning address of the data to read
 * @param length        The length of data to read
 * @return true         no error found
 * @return false        error found
 */
static bool read_eeprom(uint8_t* device_id, uint8_t* data_buffer, int start_address, int length)
{
  fprintf(stdout, "Read Eeprom Address: 0x%02x, len: %d\n", start_address, length);

  if (!onewire_reset())
  {
    fprintf(stdout, "Device not found on read\n");
    return false;
  }

  if (device_id[0] == 0)
  {                                // if device id not defined, expect single device
    onewire_write_byte(SKIP_ROM);  // SKIP ROM command for DS2431
  }
  else
  {
    onewire_select(device_id);  // use device_id
  }
  onewire_write_byte(READ_MEMORY);                  // READ MEMORY command for DS2431
  onewire_write_byte(start_address & 0xFF);         // LSB of start address
  onewire_write_byte((start_address >> 8) & 0xFF);  // MSB of start address
  for (int i = 0; i < length; i++)
  {
    data_buffer[i] = onewire_read_byte();
  }
  fprintf(stdout, "buffer = %s\n", (char*)data_buffer);
  return true;
}

/**
 * @brief function to write on the Onewire device eeprom. Write is limited at 8 bytes by shot
 *
 * @param device_id     Array of 8 numbers who identify the device
 * @param data_buffer   The data buffer who contains the data to write on eeprom
 * @param start_address The beginning address of the data to read
 * @param length        The length of data to read
 * @return true         no error found
 * @return false        error found
 */
static uint8_t write_eeprom_8bytes(const uint8_t* device_id, const char* data_buffer, int start_address, int length)
{
  volatile uint8_t ta[3];
  volatile uint8_t crcv[2];
  volatile uint8_t data_read[8];

  // Write Scratchpad block
  if (!onewire_reset())
  {
    fprintf(stdout, "Device not found on write scratchpad\n");
    return DEVICE_DISCONNECTED;
  }
  if (device_id[0] == 0)
  {                                // if device id not defined, expect single device
    onewire_write_byte(SKIP_ROM);  // SKIP ROM command for DS2431
  }
  else
  {
    onewire_select(device_id);  // use device_id
  }
  onewire_write_byte(WRITE_SCRATCHPAD);             // WRITE SCRATCHPAD command for DS2431
  onewire_write_byte(start_address & 0xFF);         // LSB of start address
  onewire_write_byte((start_address >> 8) & 0xFF);  // MSB of start address
  for (int i = 0; i < length; i++)
  {
    onewire_write_byte(data_buffer[i]);
  }

  // Read Scratchpad block
  if (!onewire_reset())
  {
    fprintf(stdout, "Device not found on read Scratchpad \n");
    return DEVICE_DISCONNECTED;
  }
  if (device_id[0] == 0)
  {                                // if device id not defined, expect single device
    onewire_write_byte(SKIP_ROM);  // SKIP ROM command for DS2431
  }
  else
  {
    onewire_select(device_id);  // use device_id
  }
  onewire_write_byte(READ_SCRATCHPAD);  // Read SCRATCHPAD command for DS2431
  ta[0] = onewire_read_byte();          // Read TA1
  ta[1] = onewire_read_byte();          // Read TA2
  ta[2] = onewire_read_byte();          // Read ESTA1
  for (int i = 0; i < length; i++)
  {
    data_read[i] = onewire_read_byte();
  }
  crcv[0] = onewire_read_byte();  // Read CRC0
  crcv[1] = onewire_read_byte();  // Read CRC1

  fprintf(stdout, "scratchpad status,TA1: 0x%02x,TA2: 0x%02x, E/S: 0x%02x,CRC0: 0x%02x, CRC1: 0x%02x\n", ta[0], ta[1], ta[2], crcv[0], crcv[1]);
  // Compare write and read scratchpad
  for (int i = 0; i < length; i++)
  {
    if (data_read[i] != data_buffer[i])
    {  // if error found stop
      fprintf(stdout, "Error between read and write scratchpad at position %d, write 0x%02x,read 0x%02x\n", i, data_buffer[i], data_read[i]);
      return BAD_INTEGRITY;
    }
  }

  // Copy Scratchpad block
  if (!onewire_reset())
  {
    fprintf(stdout, "Device not found on read Scratchpad \n");
    return DEVICE_DISCONNECTED;
  }

  if (device_id[0] == 0)
  {                                // if device id not defined, expect single device
    onewire_write_byte(SKIP_ROM);  // SKIP ROM command for DS2431
  }
  else
  {
    onewire_select(device_id);  // use device_id
  }
  onewire_write_byte(COPY_SCRATCHPAD);              // COPY SCRATCHPAD command for DS2431
  onewire_write_byte(start_address & 0xFF);         // LSB of start address
  onewire_write_byte((start_address >> 8) & 0xFF);  // MSB of start address
  onewire_write_byte(0x07);                         // ES byte for DS2431, typically 0xA5
  // Wait for the write to complete
  sleep_ms(10);  // Delay for EEPROM write time
  return 0;      // no error
}

// Function to calculate the CRC-8 checksum (Dallas/Maxim) for a given data buffer
static uint8_t calculate_crc8(const uint8_t* data, size_t size)
{
  uint8_t crc = 0;
  for (size_t i = 0; i < size; i++)
  {
    uint8_t byte = data[i];
    for (int j = 0; j < 8; j++)
    {
      uint8_t mix = (crc ^ byte) & 0x01;
      crc >>= 1;
      if (mix)
      {
        crc ^= 0x8C;  // Polynomial: x^8 + x^5 + x^4 + 1 (0x8C)
      }
      byte >>= 1;
    }
  }
  // fprintf(stdout,"CRC checksum calculated: 0x%02X\n", crc);
  return crc;
}

/**
 * @brief Search OneWire devices and save the 64bits lasered ROM on a array
 *
 * @param struct rom            Contains device_id found during search on OneWire link
 * @param nb_devices_expected   Number of devices expected on OneWire link
 * @return true                 At least one device has been found
 */
static bool onewire_read_id(struct rom* romid, size_t nb_devices_expected)
{
  char romstr[17]; /**<   Contains device id number in string format */
  size_t nb;       /**<   Counter to number of device found */
  uint8_t id[8];   /**<   Array of bytes to contains the device id */
  bool valid;      /**<   Reset if error found during function execution */
  size_t ntry = 0; /**<   Counter to number of try to found the expected devices */

  onewire_init();  // initialize oneWire GPIO
  sleep_ms(100);   // let charge OneWire devices in the link

  fprintf(stdout, "Searching All OneWire devices\n");

  do
  {  // loop is required due to intermittence in detection od Onewire devices

    valid = onewire_reset();
    if (valid == true)
    {
      fprintf(stdout, "OneWire presence detected\n");
      nb = 0;
      onewire_search_reset();  // initialize search counter and variables
      while (onewire_search(id))
      {
        romstr[0] = '\0';  // initialize array first character at 0
        romid->ecode[nb] = 0;

        for (int i = 0; i < 8; i++)
        {
          // Add the hexadecimal number to the string
          sprintf(romstr + strlen(romstr), "%02X", id[i]);
          romid->id[nb][i] = id[i];  // fill info on structure
        }
        romstr[16] = '\0';
        fprintf(stdout, "ID of Device # %d: %s\n", nb, romstr);
        strcpy(&romid->idstr[nb][0], romstr);

        // Calculate CRC-8 checksum of the data
        uint8_t crc = calculate_crc8(id, 8);
        fprintf(stdout, "CRC checksum is 0x%02X\n", crc);
        if (crc != 0)
        {
          romid->ecode[nb] = CRC_MISMATCH;
        }
        nb++;  // increment number of device found
      }
      romid->nbid = nb;
      if (nb >= nb_devices_expected)
      {  // if nb of 1-wires detected reach the minimum
        return true;
      }
      fprintf(stdout, "Nb devices detection loop # %d, expect: %d, detected %d\n", ntry, nb_devices_expected, nb);
    }
    else
    {
      fprintf(stdout, "No OneWire devices are detected, loop # %d\n", ntry);
    }
    ntry++;
    sleep_ms(100);
  } while (ntry <= 5);

  return false;
}
/**
 * @brief Write string information to the One-Wire Device
 *        The string is write on oneWire device following this device_id
 *
 * @param info           String to write on OneWire
 * @param start_address  The beginning address of the data to write
 * @return uint8_t       Return error number (0 = No Error)
 */
uint8_t onewire_write_info(const char* info, int start_address)
{
  size_t nbinfo;      /**<   Number of character in string */
  uint8_t pinfo[9];   /**<   contain substring to write on Onewire */
  uint8_t id[8];      /**<   Array of 8 bytes of the Device_id */
  int epos;           /**<   Used to contains address to write on OneWire */
  size_t i, j;        /**<   Used in for-loop for extract device id from the string */
  int byte_index = 0; /**<   Counter to number of bytes received */

  nbinfo = strlen(info);
  fprintf(stdout, "OneWire Write eeprom,len: %d,address: 0x%02x, str: \n%s\n", nbinfo, start_address, info);

  onewire_init();  // initialize oneWire GPIO
  sleep_ms(100);   // let time to charge the one device

  // Loop through the string and extract the ID of the devices
  // According to the format, the first 16 characters of string are for the device_id
  for (i = 0; i < strlen(info); i += 2)
  {
    char hex_pair[3] = {info[i], info[i + 1], '\0'};  // build a string with 2 characters
    // Check if the hex pair is valid
    for (j = 0; j < 2; j++)
    {
      if (!isxdigit(hex_pair[j]))
      {
        printf("Invalid hexadecimal character at position %d, hex: 0x%02x char: %c\n", i, hex_pair[j], hex_pair[j]);
        return OW_NO_VALIDID;
      }
    }
    // Convert the hex pair to a byte
    id[byte_index++] = (unsigned char)strtol(hex_pair, NULL, 16);
    if (byte_index > 7)
    {
      break;
    }  // if device id is completed
  }

  // OneWire device could write a maximum of 8 bytes for single write
  // The loop cut the string by 8 bytes and send each group to write eeprom function
  for (i = 0; i < nbinfo; i += 8)
  {
    strncpy(pinfo, info + i, 8);
    epos = start_address + i;  // Build location Address to use to write on EEprom
    uint8_t valid = write_eeprom_8bytes(id, pinfo, epos, 8);
    if (valid != 0)
    {
      fprintf(stdout, "Error on writing EEprom, error # %d\n", valid);
      return OW_WRITE_FAIL;  // return error
    }
    pinfo[8] = '\0';
    fprintf(stdout, "Cut String address %d, str= %s\n", epos, pinfo);
  }

  return 0;  // No Error
}

/**
 * @brief   Main function to perform a read of all eeprom on 1-wire link
 *
 * @param rinfo             Pointer who will contains the string read on Devices
 * @param start_address     Start Address on EEprom memory to start to read
 * @param length            Number of characters to read
 * @param nbid              Number of expected device on the OneWire link
 *                          Used on read loop  to be sure we have read all expected devices
 * @return uint8_t
 */
uint8_t onewire_read_info(char** rinfo, int start_address, int length, size_t nbid)
{
  bool valid;              /**<  to get error from the function */
  uint8_t readstr[length]; /**<   string to contents the result of the OneWire read */
  struct rom idr;          /**<   Structure of device id */

  fprintf(stdout, "OneWire Read eeprom info \n");

  *rinfo = (char*)malloc(length * MAX_ONEWIRE);  // set size of pointer

  valid = onewire_read_id(&idr, nbid);
  if (valid == false)
  {
    if (idr.nbid == 0)
    {
      rinfo[0] = '\0';
      return OW_NO_ONEWIRE;
    }
    else
    {
      rinfo[0] = '\0';
      return OW_NB_ONEWIRE;
    }
  }

  for (size_t nb = 0; nb < idr.nbid; nb++)
  {
    if (idr.ecode[nb] == 0)
    {  // if no error detected
      valid = read_eeprom(&idr.id[nb][0], readstr, start_address, length);
      if (valid)
      {
        // Add each device info to the result string
        if (nb == 0)
        {
          strcpy(*rinfo, "[ ");  // enclose string with bracket
          strcat(*rinfo, readstr);
          strcat(*rinfo, " ]");
        }
        else
        {
          strcat(*rinfo, " [ ");
          strcat(*rinfo, readstr);
          strcat(*rinfo, " ]");
        }
      }
      else
      {
        fprintf(stdout, "Read error with device # %d\n", nb + 1);
        idr.ecode[nb] = OW_READ_FAIL;
      }
    }
  }

  // loop to see if error raised during the check of devices
  for (size_t e = 0; e < idr.nbid; e++)
  {
    if (idr.ecode[e] != 0)
    {
      return idr.ecode[e];
    }  // if error exist, return error
  }

  return 0;  // No error
}
/**
 * @brief Check presence of oneWire and check Read Write capacity
 *
 * @param owdata        Pointer who will contains the result string to return.
 *                      Util to know device_id available on the Onewire link
 * @param nbid          number of devices expected on oneWire link
 * @return uint8_t      Error number raised during operation
 */

uint8_t onewire_check_devices(char** owdata, size_t nbid)
{
  bool valid;            /**<  to get error from the function */
  bool found;            /**<  flag to indicate of at least one OneWire device has been found */
  char teststr[NB_TEST]; /**<  Contains test string to write on device */
  char readstr[NB_TEST]; /**<  Contains string read from device */
  struct rom idr;        /**<  Structure who contains result of the search devices */

  *owdata = (char*)malloc((16 + 16) * MAX_ONEWIRE);  // set size of pointer to contains the ID of device

  valid = onewire_read_id(&idr, nbid);  // search devices and read device id
  if (valid == false)
  {  // if error received exit
    if (idr.nbid == 0)
    {
      owdata[0] = '\0';
      return OW_NO_ONEWIRE;
    }
    else
    {
      owdata[0] = '\0';
      return OW_NB_ONEWIRE;
    }
  }

  found = false;                            // preset the flag found
  for (size_t nb = 0; nb < idr.nbid; nb++)  // loop for each devices found on the search devices
  {
    if (idr.ecode[nb] == 0)
    {  // if no error detected
      fprintf(stdout, "\nChecking OneWire device # %d\n", nb + 1);
      // Generate a pseudo random 5-digit number based on device identifier
      int randNb = idr.id[nb][1] % 90000 + 10000;  // Generates a random number between 10000 and 99999

      // Convert the random number to a string
      char randStr[5];  // 5 digits
      sprintf(randStr, "%d", randNb);

      for (int x = 0; x < NB_TEST; x++)
      {
        teststr[x] = '\0';
      }  // init teststr to NULL

      strncpy(teststr, &idr.idstr[nb][0], 16);                 // Start string with device id
      strcat(teststr, ", ");                                   // add comma
      strcat(teststr, randStr);                                // add random number
      idr.ecode[nb] = onewire_write_info(teststr, ADDR_TEST);  // Write string to OneWire
      if (idr.ecode[nb] == 0)
      {
        fprintf(stdout, "Write is success with device # %d\n", nb + 1);
        valid = read_eeprom(&idr.id[nb][0], readstr, ADDR_TEST, NB_TEST);  // Read string
        if (valid)
        {
          fprintf(stdout, "Read after Write is success with device # %d\n", nb + 1);
          // compare string
          int tnb = strlen(teststr);
          fprintf(stdout, "Test string: %s\nRead string %s\n", teststr, readstr);
          int rescp = strncmp(teststr, readstr, tnb);
          // Check if the strings are identical
          if (rescp == 0)
          {
            fprintf(stdout, "The strings are identical.\n");
            found = true;
            // build string to be returned
            if (nb == 0)
            {
              strcpy(*owdata, "VALID_OWID: ");
              strcat(*owdata, &idr.idstr[nb][0]);
            }
            else
            {
              strcat(*owdata, ", NEXT_OWID: ");
              strcat(*owdata, &idr.idstr[nb][0]);
            }
          }
          else
          {
            fprintf(stdout, "The strings are not identical\n");
            idr.ecode[nb] = OW_STR_NOT_IDENTICAL;
          }
        }
        else
        {
          fprintf(stdout, "Read after Write is failure with device # %d\n", nb);
          idr.ecode[nb] = OW_READ_WRITE_FAIL;
        }
      }
      else
      {
        fprintf(stdout, "Write is failure with device # %d, error: %d\n", nb, idr.ecode[nb]);
        idr.ecode[nb] = OW_WRITE_FAIL;
      }
    }
    else
    {
      return idr.ecode[nb];  // if error raised during search devices, return this error
    }
  }
  // loop to see if error raised during the check of devices
  for (size_t e = 0; e < idr.nbid; e++)
  {
    if (idr.ecode[e] != 0)
    {
      return idr.ecode[e];
    }  // if error exist, return error
  }

  if (!found)
  {
    fprintf(stdout, "No OneWire detected\n");
    owdata[0] = '\0';
    return OW_NO_ONEWIRE;
  }

  return 0;  // No Error
}
