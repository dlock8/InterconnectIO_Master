# First TestStation InterconnectIO SCPI firmware

The complete FTS project is documented on a github website:  https://dlock8.github.io/FTS_Website/

On this deposit, we have the firmware used to create the Master Pico load to control the interconnectIO board.

The hardware support for the firmware is on github location: https://github.com/dlock8/InterconnectIO_Board


The Master Pico software is the earth of interconnectIO board. The software receive SCPI command from the serial port, 
execute the action requested by the command and return answer to the user.


## License
This project is licensed under the BSD 3-Clause License. See the [LICENSE](./LICENSE) file for more details.


## Project setup

This project has been developped on raspberry pi 4 following installation instruction from this pdf [getting-started-with-pico_C.pdf](./documentation/getting-started-with-pico_C.pdf),  a copy of the pdf is located on the main folder.
Visual studio has been used for developpment and rasberry pi 4 for Pico debug.  

Initially based on https://github.com/cronologic-de/webusb, the project has elvoved to be a complete SCPI instrument 
without frontend interface (next phase).

The compilation is performed using Visual Studio and the extension installed are:

* Cmake v0.0.17
* Cmake Tools v1.20.10
* Cortex-Debug v1.12.1
* debug-tracker-vscode v0.0.15
* Doxygen Documentation Generator v1.4.0
* Doxygen runner v1.8.0
* Github Pull Request v0.96.0
* Hex Editor v1.10.0
* MemoryView v0.0.25
* Periheral Viewer v1.4.6
* RTOS view v0.0.7
* C/C++ v1.21.6

I not sure if all extensions are required but is the one installed presently.


## Building

Build of this cmake project is performed with Visual Studio

## Development

* [`master.c`](master.c) is the main source file for the firmware.
* [`CMakeLists.txt`](CMakeLists.txt) contains build instructions for CMake, including how to build the SCPI library.
* [`pico_sdk_import.cmake`](pico_sdk_import.cmake) was (as usual) copied verbatim from the Pico SDK and allows CMake to interact with the SDK’s functionality.

## Installation

* The Files master.uf2 contains the firmware to be loaded on the Pico RP2040 board using USB cable and boot button.
* When software loaded, the Pico board should be installed on the location marked MASTER on interconnectIO Board.
* On board Pico Led will flash slowly (heartbeat) on power ON.
* Loading master.uf2 will trigger the watchdog, resulting in a burst of beeps during the boot sequence to signal the watchdog timeout. Only cycling the power ON/OFF will reset the watchdog trigger flag.

## Beep code

The boot sequence performs basic diagnostics. A failure in the tests will result in a burst of beeps. The error codes are as follows:

*   1 Beep Burst:  Master Pico's temperature is too high 
*   2 Beep Burst:   Master Pico's system voltage (VSYS) is out of limits
*   3 Beep Burst:    Error in the internal I2C communication
*   4 Beep Burst:    Error reading parameters from I2C EEPROM; default values will be used
*   5 Beep Burst:    A watchdog timeout has occurred on the Master Pico. 



## SCPI command supported by the InterconnectIO Board

The main communication for the InterconnectIO Board is based on the SCPI standard, developed by Jan Breuer [https://www.jaybee.cz/scpi-parser/]. According to the SCPI standard, some commands are mandatory, while others can be developed by the designer. In this section, we provide a list of the required SCPI commands as well as those developed specifically to control the InterconnectIO Board.


## Required SCPI commands


| SCPI_COMMAND| COMMENT
| :-----| :----- |
|*IDN?  | Identification string
|*RST   | Reset Command
|*CLS   | Clear Status
|*ESE   | Standard Event Status Enable Register
|*ESR   | Standard Event Status Register
|*OPC   | Operation Complete
|*SRE   | Service Request Enable
|*STB   | Read Status Byte
|*TST   |Internal SelfTest 
|*WAI   | Wait-to-Continue (Do nothing on this parser)
|SYSTem:ERRor[:NEXT]? |  Read actual error on the error FIFO
|SYSTem:ERRor:COUNt?|  read number of errors in the FIFO
|SYSTem:VERSion?|      read SCPI version used
|STATus:QUEStionable[:EVENt]?|  queries the event register (QUES) for the Questionable Data register
|STATus:QUEStionable:CONDition?| queries the condition register (QUESC) for the Questionable Data register
|STATus:QUEStionable:ENABle| set the status enable register (QUESE) for the Questionable Data register
|STATus:QUEStionable:ENABle?| queries the status enable register (QUESE) for the Questionable Data register
|STATus:OPERation[:EVENt]?| queries the event register (OPER) for the operation Data register
|STATus:OPERation:CONDition?| queries the condition register (OPERC) for the operation Data register
|STATus:OPERation:ENABle| set the status enable register (OPERE) for the operation Data register
|STATus:OPERation:ENABle?| queries the status enable register (OPERE) for the operation Data register

## SCPI command associated to the InterconnectIO Board

| SCPI_COMMAND| PARAMETER | COMMENT
| :-----| :-----| :----- |
|ROUTe:CLOSe|(@<ch_list>)  |Close relay based on number
|ROUTe:CLOSe:EXCLusive |(@<ch_list>) |   open relay already closed on the bank and close relay listed
|ROUTe:CLOSe:Rev |{BANK1-BANK4}| Close reverse relay to move the contact from HIGH side to LOW side of differential relay 
|ROUTe:OPEN| (@<ch_list>)|  Open relay from the channel list
|ROUTe:OPEN:Rev | {BANK1-BANK4}| Open reverse relay to move the contact to HIGH side of differential relay 
|ROUTe:OPEN:ALL |{BANK1-BANK4\|ALL}|  Open all relays from a particular Bank or all relays from all banks
|ROUTe:CHANnel:STATe?| (@<ch_list>)|  Return state of the relay in the channel list,  0: open relay, 1: Close relay
|ROUTe:BANK:STATe?| {BANK1-BANK4}|  Read decimal value of relays state on the particular bank. Each bank is a byte long
|ROUTe:REV:STATe? |{BANK1-BANK4}| Read contact side of reverse relay, LOW Side = 0, HIGH side = 1
|ROUTe:CLOSe:PWR |{LPR1\|HPR1\|HPR2\|SSR1}|  Close the designated power relay
|ROUTe:OPEN:PWR |{LPR1\|HPR1\|HPR2\|SSR1}|  Open the designated power relay
|ROUTe:STATe:PWR? |{LPR1\|HPR1\|HPR2\|SSR1}| Read state of the power relay, 0: Open, 1:Closed
|ROUTE:CLOSe:OC |{OC1\|OC2\|OC3}|   Close or activate the designated open collector transistor
|ROUTE:OPEN:OC |{OC1\|OC2\|OC3}|    Open or desactivate the designated open collector transistor
|ROUTE:STATe:OC?|{OC1\|OC2\|OC3}|   Read state of the the designated open collector transistor, 0: Open, 1:Closed
|DIGital:In:PORTn? |{0-1}|      Read Decimal value of the designated digital port (port0: 8 bits, port1: 8 bits)
|DIGital:In:PORTn:BITn? |{0-1}| Read value of the bit position at the designated port 
|DIGital:Out:PORTn |{0-1}{\<value\>}|   At the designated digital port, set the output to the value (byte)
|DIGital:Out:PORTn:BItn |{0-1} {0-7} {\<value\>}|   At the designated digital port and the designated bit, set the output to the value
|DIGital:DIRection:PORTn |{0-1}{\|value\|}| Set the direction of the designated port to the value (byte), 0: input, 1: output
|DIGital:DIRection:PORTn:BITn |{0-1} {0-7} {\<value\>}| At the designated digital port and the designated bit, set the direction to the value
|DIGital:DIRection:PORTn? |{0-1} | Read the direction value for the designated port
|DIGital:DIRection:PORTn:BITn? |{0-1} {0-7}| Read the direction value for the designated port and the designated bit position
|GPIO:DIRection:DEVice#:GP# |{0-3} {0-28} {0-1}|    At the designated device and defined gpio number, set the direction to input (0) or output (1). <br />DEVice0: Master_Pico (SCPI interpreter) <br /> DEVice1: Slave1_Pico (Digital Port 0 & 1) <br /> DEVice2: Slave2_Pico (relay bank 1 & 3) <br /> DEVice3: Slave3_Pico (relay bank 2 & 4)
|GPIO:Out:DEVice#:GP#  |{0-3} {0-28} {0-1}|  At the designated device and defined gpio number, set the output to the value.
|GPIO:In:DEVice#:GP#?  |{0-3} {0-28} |      At the designated device and defined gpio number, read the value of the GPIO.
|GPIO:SETPad:DEVice#:GP# |{0-3} {0-28} {\<Value\>}| At the designated device and defined gpio number, set the pad value.
|GPIO:GETPad:DEVice#:GP#? |{0-3} {0-28}|    At the designated device and defined gpio number,read the pad value. <br /> **PAD REGISTER DEFINITION** <br /> Bit 7: &ensp; OD Output disable <br /> Bit 6: &ensp; IE Input  enable  <br /> Bit 5:4 &ensp;DRIVE Strength 0x0: 2mA, 0x1: 4mA, 0x2: 8mA, 0x3: 12mA<br /> Bit 3:&ensp; PUE Pull up enable <br />Bit 2:&ensp; PDE Pull down enable<br />Bit 1:&ensp;   SCHT  Enable schnitt trigger<br />Bit 0:&ensp; SLF Slew rate control 1=fast 0 = slow <br />
|ANAlog:DAC:Volt | \<value\> | Set DAC output to the value
|ANAlog:DAC:Save |\<Value\> |  Save a default value for startup
|ANAlog:ADC0:Volt?||  Read Voltage at ADC input 0
|ANAlog:ADC1:Volt?||  Read Voltage at ADC input 1
|ANAlog:ADC:Vsys?||   Read system voltage from Pico Master
|ANAlog:ADC:Temp?||   Read Pico Master internal temperature in Celcius
|ANAlog:PWR:Volt?||   Read voltage at load using power monitoring device 
|ANAlog:PWR:Shunt?||  Read voltage at the shunt resistor (0.1 ohm) using power monitoring device 
|ANAlog:PWR:Ima?||    Read current(mA) passing in the shunt resistor (calculation I = E/R)
|ANAlog:PWR:Pmw?||    Read power(mW) at the load (calculation P = VI)
|ANAlog:PWR:Cal| \<actualValue,expectedValue\> | calibrate current(mA) on full range to get more precision
|SYSTem:DEVice:VERSion?||  Return firmware version of Pico for Master, Slave1, Slave2 and Slave3
|SYSTem:BEEPer  ||  Generate beep pulse
|SYSTem:LED:ERRor |{0\|1\|OFF\|ON}|     Manual control of the read error led
|SYSTem:LED:ERRor? || Status of the error led
|SYSTem:OUTput  |  {0\|1\|OFF\|ON}  |    Turn ON/OFF internal 5V available to UUT.
|SYSTem:OUTput?||  Read output status 0: OFF, 1: ON
|SYSTem:SLAves  |  {0\|1\|OFF\|ON}  |    Turn ON/OFF Pico slaves controller (run signals)
|SYSTem:SLAves?||  0: All Pico Slaves disabled, 1: All Pico Slaves enabled
|SYSTem:SLAves:STAtus? || read Pico device 'status byte' for slaves 1 to 3
|SYSTem:TESTboard | {0-5}| Selftest execute from menu below <br /> **0** Input test number to execute (0 to exit) <br>  **1** Selftest using only selftest board, no check of onewire <br> **2** Selftest run only if selftest board is installed, onewire validation <br>  **3** Selftest using selftest board and loopback connector <br>  **4** Selftest of instruments in manual mode using selftest board <br>  **5** Test of SCPI command,selftest board is required
|CFG:Write:Eeprom:STR | 'varname string,value string' | valid varname = <br> **'partnumber'**: partnumber of the InterconnectIO board, default: '500-1000-010' <br> **'serialnumber'** :  serial number of the InterconnectIO board, default: '00001' <br> **'mod_option'** :  optionnal module installed on the InterconnectIO board, default: 'DAC,PWR'<br> **'com_ser_speed'** :  baudrate used by the SCPI command serial port, default: '115200'<br> **'pico_slaves_run'** :  flag to control the slaves RUN pin actuation. 0: Pico Slaves reset at each boot(disable USB), 1: Do not reset slaves at boot, default: '0'<br> **'testboard_num'** :  partnumber of the selftest board written on the onewire device , default: '500-1010-020'
|CFG:Write:Eeprom:Default  ||   Special command to write all default value to eeprom
|CFG:Read:Eeprom:Full?  ||       Special command to read all data on eeprom
|CFG:Read:Eeprom:STR?  |    'varnames string'|  Reads string value from the designated parameter

## SCPI command associated to the communication

| SCPI_COMMAND| PARAMETER | COMMENT
| :-----| :-----| :----- |
|COM:1WIRE:CHECK? |\<value\>| value = Number of expected Onewire devices on a link, normally 1 or 2,  Check if devices are detected|
|COM:1WIRE:WRITE |{String 64 chars maximum starting with 64bits lasered ROM}| example: 2D4CE282200000CC, 500-1010-020, 000001, J1
|COM:1WIRE:READ?  |[Nb of Onewire devices] | read string of all the 1-Wire devices on the link, specify number of onewire to be read
|COM:INITialize:ENAble  |{SPI\|SERIAL\|I2C}|  Configure designated communication port
|COM:INITialize:DISable  |{SPI\|SERIAL\|I2C}|  Configure designated communication port to GPIO
|COM:INITialize:STATus? | {SPI\|SERIAL\|I2C}| Read if the designated communication is enable or disable
|COM:SERIAL:WRITE   |\<svalues\>  |  send string, don't wait for answer
|COM:SERIAL:READ?   | \<svalues\> |  send string, wait for answer
|COM:SERIAL:Baud     |\<value\>    | set baudrate speed 
|COM:SERIAL:Baud?|| read baudrate speed
|COM:SERIAL:Protocol |\<svalues\> | exa: {N81  O72  E61 8N1 etc ..}. Three character to define prtocol to be use
|COM:SERIAL:Protocol?|| read the used protocol
|COM:SERIAL:Handshake | {0\|1\|OFF\|ON} |  set the RTS-CTS handshake according to the value
|COM:SERIAL:Handshake?|| read value corresponding to the handshake state, 0: disabled, 1: enabled 
|COM:SERIAL:Timeout  |\<values\>  | Timeout in ms (32bits), default is 1000  
|COM:SERIAL:Timeout?|| read value of the timeout
|COM:SPI:WRIte      |\<data\> | Spi write data (byte if databit =8 or word if databits = 16) 
|COM:SPI:REAd:LENx? |\<opt:register\> | Spi read x bytes
|COM:SPI:Baudrate  |   \<value\> |   Set baudrate speed in Hz
|COM:SPI:Baudrate? || read SPI speed in Hz
|COM:SPI:CS     | \<gpio\>   |   Set Chipselect gpio,default is 5, valid num = {0,1,5,6,7,12,13,14,15,16,17}
|COM:SPI:CS?   ||  Get  Chipselect gpio number
|COM:SPI:Databits  | \<value\>   |  // number of data bits to read or write during SPI communication, normally 8 (byte long) or 16 (word long) 
|COM:SPI:Databits? || read databit value
|COM:SPI:Mode   | {0\|1\|2\|3\|4\|5\|6\|7}  | Set SPI mode + ChipSelect toggle mode (0:End of transmit, 1: At each byte) <br>mode 0: Cs=0, Cpol=0, CPha=0  <br> mode 1: Cs=0, Cpol=0, Cpha=1  <br> mode 2: Cs=0, Cpol=1, Cpha=0  <br> mode 3: Cs=0, Cpol=1, Cpha=1  <br> mode 4: Cs=1, Cpol=0, CPha=0  <br> mode 5: Cs=1, Cpol=0, Cpha=1  <br> mode 6: Cs=1, Cpol=1, Cpha=0  <br> mode 7: Cs=1, Cpol=1, Cpha=1  <br> 
|COM:SPI:Mode? || read mode number used 
|COM:I2C:WRIte      |\<data\> | Spi write data
|COM:I2C:REAd:LENx? |\<opt:register\> | Spi read x bytes from optional register
|COM:I2C:ADDRress |   \<Device address\> |   I2C Address to use to talk to the device
|COM:I2C:ADDRress? ||  read address value
|COM:I2C:Baudrate? ||  read I2C speed in Hz
|COM:I2C:Baudrate  |   \<value\> |   Set speed in Hz
|COM:I2C:Databits  | \<value\>   |   Number of databits to write or read. Normally 8 (bytes) or 16 (word) 
|COM:I2C:Databits? ||  read number of databits used on I2C communication




## SCPI Complete Command example:

| SCPI_COMMAND| COMMENT
| :-----| :-----|
|ROUTe:CLOSe(@101)  |         Close relay 101, Signal on High side
|ROUTe:CLOSe:EXC(@102)  |     open relay on bank1 (101) and close relay 102
|ROUTe:CLOSe (@108,115)   |    close relay 108 and 115, signal on low side
|ROUTe:CLOSe (@100:104) |      close relay 100,101,102,103, and 104
|ROUTe:OPEN:ALL BANK1,BANK2 |  open relay  on bank1 and bank2
|ROUTe:OPEN:ALL ALL    |        open relay  on all banks
|ROUTe:REV:STATe? BANK1   |    read which side of diff relay we are connected (0 = H or 1 = L)
|ROUTe:CHAN:STATe? (@100) |    read relay state ( 0 = Open  or 1 = close)
|DIGital:DIRection:PORT0 \#HFF |set the 8 bit of port0 to output  ( 0 = in, 1 = out)
|DIGital:Out:PORT1 \#H55   |  Write hex value 0x55 to port 1
|DIGital:DIRection:PORT1? |  Read direction on port 1 for the 8 bits ( 0 = in, 1 = out)
|DIGital:In:PORT0?  | Read the 8 bits on port 0  
|GPIO:DIRection:DEVice0:GP22 1  |Set gpio 22 on Device 0 (Master) to direction out (1 = out) 
|GPIO:Out:DEVice1:GP8 0  |  Set gpio 8 on Device 1 (Slave_1) to 0 (0 = low level) 
|GPIO:In:DEVice2:GP9?  |  Read value of gpio 9 on device 2 (Slave_2)
|GPIO:SETPad:DEVice3:GP22 | Set PAD value (8 bits) on gpio 22 on Device 3 (Slave_3) 
|GPIO:GETPad:DEVice1:GP8? | Read PAD value (8 bits) on gpio 8 on Device 1 (Slave_1) 
|SYSTem:BEEPer   | Turn ON momentary the beeper
|SYSTEM:DEVice:VERsion? | Read version of each Pico devices [0,1,2,3], return string <br>  ex: 1.0, 1.0, 1.0 1.0
|COM:1WIRE:WRITE "2D4CE282200000CC, 500-1010-020, 000001, J1"|Lasered ROM +  Board#+ Serial + Location



## Relay numbering scheme

    Relay bank 1   Differential: 10 @ 17    Single: 100 @ 107 (High) 108 @ 115 (Low)    
    Relay bank 2   Differential: 20 @ 27    Single: 200 @ 215 (High) 208 @ 215 (Low) 
    Relay bank 3   Differential: 30 @ 37    Single: 300 @ 315 (High) 308 @ 315 (Low) 
    Relay bank 4   Differential: 40 @ 47    Single: 400 @ 415 (High) 108 @ 415 (Low) 

* Relay type are DPDT (Double Pole Double Throw). Relay could be connected in differential mode or configured in single ended mode
* Relay address used reflect the difference between the differential and single mode.
* Relay @10, @100 and @108 are the same physical relay.  
* If relay @10 is closed, the high side of the relay (BK1_CH0_H) will be connected on high side of the common point (BK1_COM_H). The low side of the relay (BK1_CH0_L) will be connected on the low side of the common point (BK1_COM_L)
* If relay @100 is closed, the high side of the relay (BK1_CH0_H) will be connected on high side of the common point (BK1_COM_H). The low side of the relay (BK1_CH0_L) will be connected on the low side of the common point (BK1_COM_L)
*   If relay @108 is closed, the high side of the relay (BK1_CH0_H) will be connected on low side of the common point (BK1_COM_L) because the reverse relay will be actuated on the same time of the relay @108. The low side of the relay (BK1_CH0_L) will be connected on the high side of the common point (BK1_COM_H)
* The 4 relay banks are identical and follow the same rules 




