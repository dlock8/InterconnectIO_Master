#ifndef dev_ds2431
#define dev_ds2431


#define ONEWIRE_PIN 10      // GPIO pin connected to the One-Wire bus
#define ADDR_INFO   0       // Start address of the information string on eeprom
#define NB_INFO     48      // Maximum character of filed INFO 
#define ADDR_TEST   0x60    // Test Address available to check read write during check
#define NB_TEST     32       // Size of the selftest field
#define MAX_ONEWIRE 2        // Maximum number of devices on 1-wire in same time


/**
 * @brief Structure who contains information about the Onewire devices
 * 
 *      Not used for the moment but could be useful in case of different model is used
 */
typedef struct {
    uint8_t id;         /**<   2 byte identifier on the lasered ROM */
    const char* name;   /**<   Device partnumber */
    int pages;          /**<   Number of memory pages*/
    int addrSize;       /**<   Number of bytes required in the adsress*/
    bool isEPROM;       /**<   True if device is Eprom Add-Only Memory*/
} model_type;

/**
 * @brief Structure used to contains information about each Onewire found on the link
 * 
 */
struct rom{
    uint8_t id[MAX_ONEWIRE][8];         /**> Contains the device id in bytes format */
    char idstr[MAX_ONEWIRE][16+1];      /**> Contains string of the device_id + null character*/
    int ecode[MAX_ONEWIRE];             /**> Contains error code for each device on the link*/
    size_t nbid;                        /**> Counter to number of device found on OneWire link */
};


/**
 * @brief  Format EEprom mandatory (reserved 48 Bytes)
 *
 *      Address:  0 
 *      Device_id, Board_part_number, Bd_serial_number, location (J1 or J2)
 *          16   2   12            2      6           2    2
 */


/**> List of commands available for OneWire device DS2431 */
#define WRITEMEMORY         0x0F  
#define READ_MEMORY         0xF0
#define READ_SCRATCHPAD     0xAA
#define WRITE_SCRATCHPAD    0x0F
#define COPY_SCRATCHPAD     0x55
#define MATCHROM            0x55  
#define COPYLOCK            0x5A  
#define READSTATUSREG       0x66  
#define WRITEAPPREG         0x99  
#define VERIFYRESUME        0xA5  
#define READSTATUS          0xAA  
#define READMEMORYCRC       0xC3 
#define READROM			    0x33   
#define SEARCH_ROM          0xF0  
#define READ_ROM            0x33
#define SKIP_ROM            0xCC
#define ALARM_SEARCH        0xEC


/**> Error code for Onewire devices */
#define CRC_MISMATCH        1 
#define INVALID_PAGE        2 
#define PAGE_LOCKED         3 
#define BAD_INTEGRITY       4 
#define COPY_FAILURE        5 
#define APP_REG_LOCKED      6
#define NO_ONEWIRE          7
#define UNSUPPORTED_OPP     8 
#define UNSUPPORTED_DEVICE  9 
#define DEVICE_DISCONNECTED 10 

#define OW_STR_NOT_IDENTICAL    15
#define OW_READ_WRITE_FAIL      16
#define OW_WRITE_FAIL           17
#define OW_READ_FAIL            18
#define OW_NB_ONEWIRE           19
#define OW_NO_ONEWIRE           20
#define OW_NO_VALIDID           21



/**> List of function visible by other program*/

uint8_t onewire_write_info(const char* info,int start_address);
uint8_t onewire_read_info(char** rinfo, int start_address,int length,size_t nbid);
uint8_t onewire_check_devices(char** owdata, size_t nbid );


#endif


