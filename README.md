# First TestStation Interconnect IO SCPI firmware



## Project setup



## Building

This is a CMake-based project, so it’s the usual

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

However, there are two caveats:

1. As explained in the Pico SDK’s documentation, you need to set the `PICO_SDK_PATH` environment variable to point to the SDK’s location. If you forget this, the `cmake ..` command will fail.
2. The SCPI parser library this firmware depends on is a `Makefile`-based project. You will need to have `make` available to build it. Natively building on Windows, e.g. using Visual Studio, has not been tested. Using WSL or Cygwin instead is recommended, if you’re on Windows.

## Development

* [`master.c`](master.c) is the main source file for the firmware.
* [`CMakeLists.txt`](CMakeLists.txt) contains build instructions for CMake, including how to build the SCPI library.
* [`pico_sdk_import.cmake`](pico_sdk_import.cmake) was (as usual) copied verbatim from the Pico SDK and allows CMake to interact with the SDK’s functionality.


## SCPI command supported
## Required SCPI commands


| SCPI_COMMAND| PARAMETER | COMMENT
| :-----| :-----| :----- |
| *IDN? ||
| *RST  || Reset Command
|*CLS   || Clear Status
|*ESE   || Standard Event Status Enable Register
|*ESR   || Standard Event Status Register
|*OPC   || Operation Complete
|*SRE   || Service Request Enable
|*STB   || Read Status Byte
|*TST   || Self Test
|*WAI   || Wait-to-Continue (Do nothing on this parser)
|       ||
|SYSTem:ERRor[:NEXT]? ||
|SYSTem:ERRor:COUNt?||
|SYSTem:VERSion?||
| ||
|STATus:QUEStionable[:EVENt]?||
|STATus:QUEStionable:CONDition?||
|STATus:QUEStionable:ENABle||
|STATus:QUEStionable:ENABle?||
| ||
|STATus:OPERation[:EVENt]?||
|STATus:OPERation:CONDition?||
|STATus:OPERation:ENABle||
|STATus:OPERation:ENABle?||
| ||

## SCPI command associated to the Interconnect IO Board

| SCPI_COMMAND| PARAMETER | COMMENT
| :-----| :-----| :----- |
|ROUTe:CLOSe|(@<ch_list>)  |Close relay based on number
|ROUTe:CLOSe:EXCLusive |(@<ch_list>) |
|ROUTe:CLOSe:Rev |{BANK1-BANK4}| Close reverse relay to move the contact from HIGH side to LOW side of differential relay 
|ROUTe:OPEN| (@<ch_list>)|
|ROUTe:OPEN:Rev | {BANK1-BANK4}| Open reverse relay to move the contact to HIGH side of differential relay 
|ROUTe:OPEN:ALL |{BANK1-BANK4|ALL}|
|ROUTe:CHANnel:STATe?| (@<ch_list>)|
|ROUTe:BANK:STATe?| {BANK1-BANK4}|
|ROUTe:REV:STATe? |{BANK1-BANK4}| Read contact side of reverse relay, LOW Side = 0, HIGH side = 1
| | |
|ROUTe:CLOSe:PWR |{LPR1\|HPR1\|HPR2\|SSR1}|
|ROUTe:OPEN:PWR |{LPR1\|HPR1\|HPR2\|SSR1}|
|ROUTe:STATe:PWR? |{LPR1\|HPR1\|HPR2\|SSR1}|
| ||
|ROUTE:CLOSe:OC |{OC1\|OC2\|OC3}|
|ROUTE:OPEN:OC |{OC1\|OC2\|OC3}|
|ROUTE:STATe:OC?|{OC1\|OC2\|OC3}|
| ||
|DIGital:In:PORTn? |{0-1}|
|DIGital:In:PORTn:BITn? |{0-1}|
|DIGital:Out:PORTn |{0-1}{\<value\>}|   
|DIGital:Out:PORTn:BItn |{0-1} {0-7}|  
|DIGital:DIRection:PORTn |{0-1}{\|value\|}| 
|DIGital:DIRection:PORTn:BITn |{0-1} {0-7}| 
|DIGital:DIRection:PORTn? |{0-1} |
|DIGital:DIRection:PORTn:BITn? |{0-1} {0-7}|
| ||
|GPIO:DIRection:DEVice#:GP# |{0-3} {0-28} {0-1}|
|GPIO:Out:DEVice#:GP#  |{0-3} {0-28} {0-1}|
|GPIO:In:DEVice#:GP#?  |{0-3} {0-28} |
|GPIO:SETPad:DEVice#:GP# |{0-3} {0-28} {\<Value\>}|
|GPIO:GETPad:DEVice#:GP#? |{0-3} {0-28}|
| ||
|ANAlog:DAC:Volt | \<value\> |
|ANAlog:DAC:Save |\<Value\> |  Save a default value for startup
|ANAlog:ADC0:Volt?||
|ANAlog:ADC1:Volt?||
|ANAlog:ADC:Vsys?||
|ANAlog:ADC:Temp?||
|ANAlog:PWR:Volt?||
|ANAlog:PWR:Shunt?||
|ANAlog:PWR:Ima?||
|ANAlog:PWR:Pmw?||
|ANAlog:PWR:Cal| \<actualValue,expectedValue\> | calibrate full range
|ANAlog:MEASure:PWR? | {V\|S\|I\|P}|
|ANALOG:CALibrate:PWR| [\<value\>| ??????????
| ||
|SYSTEM:DEVice:VERsion?||
|SYSTem:BEEPer  ||
|SYSTEM:LED:ERRor |{0\|1\|OFF\|ON}|
|SYSTEM:LED:ERRor? ||
|SYSTEM:RESet  (*RST) || ??????????
|SYSTEM:MODule_option? | [TBD]|
| ||
|SYSTEM:OUTput  |  {0\|1\|OFF\|ON}  |    Turn ON/OFF internal 5V available to UUT.
|SYSTEM:SLAves  |  {0\|1\|OFF\|ON}  |    Turn ON/OFF Pico slaves
|SYSTEM:OUTput?||
|SYSTEM:SLAves?||
|SYSTEM:SLAves:STAtus? || read device status byte for slaves 1 to 3
| ||
|CFG:Write:Eeprom:STR |     varname svalues | varname = {partnumber,serialnumber,mod_option,def_cser}
|CFG:Read:Eeprom:STR?  |    varnames values|
|CFG:Write:Eeprom:Default  ||   Special command to write default value to eeprom
|CFG:Read:Eeprom:Full?  ||       Special command to read all data on eeprom
| ||

## SCPI command associated to the communication

| SCPI_COMMAND| PARAMETER | COMMENT
| :-----| :-----| :----- |
|COM:1WIRE:CHECK? |\<value\>| value = Number of expected Onewire devices on a link, normally 1 or 2,  Check if devices are detected|
|COM:1WIRE:WRITE |{String 64 chars maximum starting with 64bits lasered ROM}|
|COM:1WIRE:READ?  |[Nb of Onewire devices] | read string of all the 1-Wire devices on the link
| ||
|COM:INITialize:ENAble  |{SPI\|SERIAL\|I2C}|  Configure communication port
|COM:INITialize:DISable  |{SPI\|SERIAL\|I2C}|  Configure communication port to GPIO
|COM:INITialize:STATus? | {SPI\|SERIAL\|I2C}| Read if communication is enable or disable
| ||
|COM:SERIAL:WRITE   |\<svalues\>  |  send string, don't wait for answer
|COM:SERIAL:READ?   | \<svalues\> |  send string, wait for answer
|COM:SERIAL:Baud     |\<value\>    |  baudrate speed 
|COM:SERIAL:Baud?||
|COM:SERIAL:Protocol |\<svalues\> | exa: {N81  O72  E61 8N1 etc ..}
|COM:SERIAL:Protocol?||
|COM:SERIAL:Handshake | {0\|1\|OFF\|ON} |
|COM:SERIAL:Handshake?||
|COM:SERIAL:Timeout  |\<values\>  | Timeout in ms (32bits)
|COM:SERIAL:Timeout?||
| ||
|COM:SPI:WRIte      |\<data\> | Spi write data (byte if databit =8 or word if databits = 16) 
|COM:SPI:REAd:LENx? |\<opt:register\> | Spi read x bytes
|COM:SPI:Baudrate  |   \<value\> |   baudrate speed 
|COM:SPI:Baudrate? ||
|COM:SPI:CS     | \<gpio\>   |   Set Chipselect gpio,default is 5, valid num = {0,1,5,6,7,12,13,14,15,16,17}
|COM:SPI:CS?   ||  Get  Chipselect gpio number
|COM:SPI:Databits  | \<value\>   |  // number of data bits to read or write during SPI communication, normally 8 (bytes) or 16 (word) 
|COM:SPI:Databits? ||
|COM:SPI:Mode   | {0\|1\|2\|3\|4\|5\|6\|7\}  | Normal SPI mode + CS mode (0:End of transmit, 1: At each byte)
|                           ||  mode 0: Cs=0, Cpol=0, CPha=0  
|                           ||  mode 1: Cs=0, Cpol=0, Cpha=1  
|                           ||  mode 2: Cs=0, Cpol=1, Cpha=0
|                           ||  mode 3: Cs=0, Cpol=1, Cpha=1
|                           ||  mode 4: Cs=1, Cpol=0, CPha=0  
|                           ||  mode 5: Cs=1, Cpol=0, Cpha=1  
|                           ||  mode 6: Cs=1, Cpol=1, Cpha=0
|                           ||  mode 7: Cs=1, Cpol=1, Cpha=1
|COM:SPI:Mode? ||
| ||
|COM:I2C:WRIte      |\<data\> | Spi write data
|COM:I2C:REAd:LENx? |\<opt:register\> | Spi read x bytes
|COM:I2C:ADDRress |   \<Device address\> |   Address to use to talk to the device
|COM:I2C:ADDRress? ||
|COM:I2C:Baud? ||
|COM:I2C:Baud  |   \<value\> |   baudrate speed 
|COM:I2C:Baud? ||
|COM:I2C:Databits  | \<value\>   |  // Number of databits to write or read. Normally 8 (bytes) or 16 (word) 
|COM:I2C:DATAbits? ||








## SCPI Complete Command example:

| SCPI_COMMAND| COMMENT
| :-----| :-----|
|ROUTe:CLOSe(@101)  |         Close relay 100 and close relay Single ended (SE)
|ROUTe:CLOSe:EXC(@102)  |     open relay on bank1, including relay SE and close relay 102
|ROUTe:CLOSe (@100,108)   |    close relay 100 and 108
|ROUTe:CLOSe (@100:108) |      close relay 100,102,104,106, and 108
|ROUTe:OPEN:ALL BANK1,BANK2 |  open relay  on bank1 and bank2
|ROUTe:OPEN:ALL ALL    |        open relay  on all banks
|ROUTe:REV:STATe? BANK1   |    read which side of diff relay we are connected (0 = H or 1 = L)
|ROUTe:CHAN:STATe? (@100) |    read relay state ( 0 = Open  or 1 = close)
| |
|DIGital:DIRection:PORT0 #HFF |set the 8 bit of port0 to output  ( 0 = in, 1 = out)
|DIGital:Out:PORT1 #H55   |  Write hex value 0x55 to port 1
|DIGital:DIRection:PORT1? |  Read direction on port 1 for the 8 bits ( 0 = in, 1 = out)
|DIGital:In:PORT0?  | Read the 8 bits on port 0  
| |
|GPIO:DIRection:DEVice0:GP22 1  |Set gpio 22 on Device 0 (Master) to direction out (1 = out) 
|GPIO:Out:DEVice1:GP8 0  |  Set gpio 8 on Device 1 (Slave_1) to 0 (0 = low level) 
|GPIO:In:DEVice2:GP9?  |  Read value of gpio 9 on device 2 (Slave_2)
|GPIO:SETPad:DEVice3:GP22 | Set PAD value (8 bits) on gpio 22 on Device 3 (Slave_3) 
|GPIO:GETPad:DEVice1:GP8? | Read PAD value (8 bits) on gpio 8 on Device 1 (Slave_1) 
| |
|SYSTem:BEEPer   | Turn ON momentary the beeper
|SYSTEM:DEVice:VERsion? | Read version of each Pico devices [0,1,2,3], return string
|     |                               ex: 1.8, 1.3, 1.3 1.3
| |
|COM:1WIRE:WRITE "2D4CE282200000CC, 500-1010-020, 000001, J1"|Lasered ROM +  Board#+ Serial + Location



## Relay numbering scheme

## Differential relay will be located on lower address. 
## Single ended relay to be connected to high or low side following the address.
    Relay bank 1   Channel 100 @ 115      
    Relay bank 2   Channel 200 @ 215
    Relay bank 3   Channel 300 @ 315
    Relay bank 4   Channel 400 @ 415

    Relay bank 1   Differential: 10 @ 17    Single: 100 @ 107 (High) 108 @ 115 (Low)    
    Relay bank 2   Differential: 20 @ 27    Single: 200 @ 215 (High) 208 @ 215 (Low) 
    Relay bank 3   Differential: 30 @ 37    Single: 300 @ 315 (High) 308 @ 315 (Low) 
    Relay bank 4   Differential: 40 @ 47    Single: 400 @ 415 (High) 108 @ 415 (Low) 

## SE relay numbering for actuation without other relay
    Relay SE bank 1  116    ?? Not implented
    Relay SE bank 2  216    ??
    Relay SE bank 3  216    ??
    Relay SE bank 4  216    ??


