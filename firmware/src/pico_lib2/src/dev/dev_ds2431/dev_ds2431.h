/**
 * @file    dev_ds2431.h
 * @author  Daniel Lockhead
 * @date    2024
 * 
 * @brief   Header to perform communication with OneWire device
 *   
 * 
 * @details OneWire driver to read / write on the EEprom of the Onewire Device. Plan to be used
  *         to detect what's is connected to Interconnect IO Board before turning the main power. 
 *
 *
 * @copyright Copyright (c) 2024, D.Lockhead. All rights reserved.
 *
 * This software is licensed under the BSD 3-Clause License.
 * See the LICENSE file for more details.
 */
 
#ifndef dev_ds2431
#define dev_ds2431

/** 
 * @brief GPIO pin connected to the One-Wire bus.
 */
#define ONEWIRE_PIN 10      

/** 
 * @brief Start address of the information string on EEPROM. 
 */
#define ADDR_INFO   0       

/** 
 * @brief Maximum characters allowed in the info field. 
 */
#define NB_INFO     48      

/** 
 * @brief Test address available to check read/write during testing.
 */
#define ADDR_TEST   0x60    

/** 
 * @brief Size of the self-test field. 
 */
#define NB_TEST     32       

/** 
 * @brief Maximum number of devices on the 1-Wire bus at the same time.
 */
#define MAX_ONEWIRE 2        


/** 
 * @brief Structure to define chip model attributes.
 */
typedef struct {
    uint8_t id;          /**< Chip ID. */
    const char *name;    /**< Name of the chip. */
    uint8_t pages;       /**< Number of pages. */
    uint8_t size;        /**< Size of the chip. */
    bool available;      /**< Availability status of the chip. */
} model_type; ///< Typedef for the model structure



/**
 * @brief Structure used to contain information about each OneWire device found on the link.
 */
struct rom {
    uint8_t id[MAX_ONEWIRE][8];         /**< Contains the device ID in byte format. */
    char idstr[MAX_ONEWIRE][16 + 1];    /**< Contains string representation of the device ID + null character. */
    int ecode[MAX_ONEWIRE];             /**< Contains error codes for each device on the link. */
    size_t nbid;                        /**< Counter for the number of devices found on the OneWire link. */
};




/**
 * @brief  Format EEprom mandatory (reserved 48 Bytes)
 *
 *      Address:  0 
 *      Device_id, Board_part_number, Bd_serial_number, location (J1 or J2)
 *          16   2   12            2      6           2    2
 */



/** 
 * @brief List of commands available for OneWire device DS2431.
 */
#define WRITEMEMORY         0x0F  /**< Command to write memory. */
#define READ_MEMORY         0xF0  /**< Command to read memory. */
#define READ_SCRATCHPAD     0xAA  /**< Command to read scratchpad. */
#define WRITE_SCRATCHPAD    0x0F  /**< Command to write scratchpad. */
#define COPY_SCRATCHPAD     0x55  /**< Command to copy scratchpad to memory. */
#define MATCHROM            0x55  /**< Command to match ROM. */
#define COPYLOCK            0x5A  /**< Command to copy lock. */
#define READSTATUSREG       0x66  /**< Command to read status register. */
#define WRITEAPPREG         0x99  /**< Command to write application register. */
#define VERIFYRESUME        0xA5  /**< Command to verify resume. */
#define READSTATUS          0xAA  /**< Command to read status. */
#define READMEMORYCRC       0xC3  /**< Command to read memory CRC. */
#define READROM             0x33  /**< Command to read ROM. */
#define SEARCH_ROM          0xF0  /**< Command to search ROM. */
#define READ_ROM            0x33  /**< Command to read ROM (duplicate). */
#define SKIP_ROM            0xCC  /**< Command to skip ROM. */
#define ALARM_SEARCH        0xEC  /**< Command to perform an alarm search. */


/** 
 * @brief Error codes for OneWire devices.
 */
#define CRC_MISMATCH        1   /**< Error code for CRC mismatch. */
#define INVALID_PAGE        2   /**< Error code for invalid page. */
#define PAGE_LOCKED         3   /**< Error code for locked page. */
#define BAD_INTEGRITY       4   /**< Error code for bad integrity check. */
#define COPY_FAILURE        5   /**< Error code for copy failure. */
#define APP_REG_LOCKED      6   /**< Error code for locked application register. */
#define NO_ONEWIRE          7   /**< Error code when no OneWire device is found. */
#define UNSUPPORTED_OPP     8   /**< Error code for unsupported operation. */
#define UNSUPPORTED_DEVICE  9   /**< Error code for unsupported device type. */
#define DEVICE_DISCONNECTED 10   /**< Error code for disconnected device. */

#define OW_STR_NOT_IDENTICAL    15 /**< Error code for OneWire string not identical. */
#define OW_READ_WRITE_FAIL      16 /**< Error code for read/write failure on OneWire. */
#define OW_WRITE_FAIL           17 /**< Error code for write failure on OneWire. */
#define OW_READ_FAIL            18 /**< Error code for read failure on OneWire. */
#define OW_NB_ONEWIRE           19 /**< Error code for number of OneWire devices found. */
#define OW_NO_ONEWIRE           20 /**< Error code for no OneWire devices found. */
#define OW_NO_VALIDID           21 /**< Error code for no valid ID found on OneWire. */


/**> List of function visible by other program*/

uint8_t onewire_write_info(const char* info,int start_address);
uint8_t onewire_read_info(char** rinfo, int start_address,int length,size_t nbid);
uint8_t onewire_check_devices(char** owdata, size_t nbid );


#endif


