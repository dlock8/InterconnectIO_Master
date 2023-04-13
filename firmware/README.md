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

* [`fts_scpi.c`](fts_scpi.c) is the main source file for the firmware.
* [`CMakeLists.txt`](CMakeLists.txt) contains build instructions for CMake, including how to build the SCPI library.
* [`pico_sdk_import.cmake`](pico_sdk_import.cmake) was (as usual) copied verbatim from the Pico SDK and allows CMake to interact with the SDK’s functionality.


## SCPI command supported

*IDN?
*RST
*CLS


ROUTe:CLOSe (@<ch_list>)  
ROUTe:CLOSe:EXCLusive (@<ch_list>) 
ROUTe:CLOSe:Se {BANK1-BANK4}
ROUTe:OPEN (@<ch_list>)
ROUTe:OPEN:Se  {BANK1-BANK4}
ROUTE:OPEN:ALL {BANK1-BANK4|ALL}
ROUTE:CHANnel:STATe? (@<ch_list>)
ROUTE:BANK:STATe? {BANK1-BANK4}
ROUTE:SE:STATe? {BANK1-BANK4}

ROUTE:CLOSe:PWR {LPR1|HPR1|HPR2|SSR1}
ROUTE:OPEN:PWR {LPR1|HPR1|HPR2|SSR1}
ROUTE:STATe:PWR? {LPR1|HPR1|HPR2|SSR1}

DIGital:In:PORTn? {0-1}
DIGital:In:PORTn:BITn? {0-1}
DIGital:Out:PORTn {0-1} {Value}   
DIGital:Out:PORTn:BItn {0-1} {0-7}  
DIGital:DIRection:PORTn {0-1}{Value} 
DIGital:DIRection:PORTn:BITn {0-1} {0-7} 
DIGital:DIRection:PORTn? {0-1} 
DIGital:DIRection:PORTn:BITn? {0-1} {0-7}

GPIO:DIRection:DEVice#:GP# {0-3} {0-28} {0-1}
GPIO:Out:DEVice#:GP#  {0-3} {0-28} {0-1}
GPIO:In:DEVice#:GP#?  {0-3} {0-28} 
GPIO:SETPad:DEVice#:GP# {0-3} {0-28} {Value}
GPIO:GETPad:DEVice#:GP#? {0-3} {0-28}

ANAlog:MEASure:ADC#?
ANAlog:SET_VREF [Value] 
ANAlog:GET_VREF? 
ANAlog:TEMPerature? 

SYSTEM:DEVice:VERsion?
SYSTem:BEEPer  
SYSTEM:LED:ERRor {0|1|ON|OFF}
SYSTEM:RESet  
SYSTEM:MODule_option?  [TBD]
SYSTEM:INITialize {SPI|UART2|UART4|I2C|ADC}



COM:1WIRE:WRITE
COM:1WIRE:READ? [{SERIAL|MEMORY|ALL}]
COM:I2C:WRITE
COM:SPI:WRITE
COM:SERIAL:WRITE





## SCPI example:
ROUTe:CLOSe (@101)              --> Close relay 100 and close relay Single ended (SE)
ROUTe:CLOSe:EXC (@102)          --> open relay on bank1, including relay SE and close relay 102
ROUTe:CLOSe (@100,108)          --> close relay 100 and 108
ROUTe:CLOSe (@100:108)          --> close relay 100,102,104,106, and 108
ROUTe:OPEN:ALL BANK1,BANK2      --> open relay  on bank1 and bank2
ROUTe:OPEN:ALL ALL              --> open relay  on all banks
ROUTe:SE:STATe? BANK1           --> read which side of diff relay we are connected (0 = H or 1 = L)
ROUTe:CHAN:STATe? (@100)        --> read relay state ( 0 = Open  or 1 = close)

DIGital:DIRection:PORT0 #HFF    --> set the 8 bit of port0 to output  ( 0 = in, 1 = out)
DIGital:DIRection:PORT0:BIT1?   --> Read direction on port 0 bit 1
DIGital:Out:PORT1 #H55          --> Write hex value 0x55 to port 1
DIGital:DIRection:PORT1?        --> Read direction on port 1 for the 8 bits ( 0 = in, 1 = out)
DIGital:In:PORT0?               --> Read the 8 bits on port 0  


GPIO:DIRection:DEVice0:GP22 1   --> Set gpio 22 on Device 0 (Master) to direction out (1 = out) 
GPIO:Out:DEVice1:GP8 0          --> Set gpio 8 on Device 1 (Slave_1) to 0 (0 = low level) 
GPIO:In:DEVice2:GP9?            --> Read value of gpio 9 on device 2 (Slave_2)
GPIO:SETPad:DEVice3:GP22        --> Set PAD value (8 bits) on gpio 22 on Device 3 (Slave_3) 
GPIO:GETPad:DEVice1:GP8?        --> Read PAD value (8 bits) on gpio 8 on Device 1 (Slave_1) 

SYSTem:BEEPer                   --> Turn ON momentary the beeper
SYSTEM:DEVice:VERsion?          --> Read version of each Pico devices [0,1,2,3], return string
                                    ex: 1.8, 1.3, 1.3 1.3


## Relay numbering. Differential relay will be located on lower address. Single ended relay to be connected to high or low side following the address.
    Relay bank 1   Channel 100 @ 115      
    Relay bank 2   Channel 200 @ 215
    Relay bank 3   Channel 300 @ 315
    Relay bank 4   Channel 400 @ 415

    Relay bank 1   Differential: 10 @ 17    Single: 100 @ 107 (High) 108 @ 115 (Low)    
    Relay bank 2   Differential: 20 @ 27    Single: 200 @ 215 (High) 208 @ 215 (Low) 
    Relay bank 3   Differential: 30 @ 37    Single: 300 @ 315 (High) 308 @ 315 (Low) 
    Relay bank 4   Differential: 40 @ 47    Single: 400 @ 415 (High) 108 @ 415 (Low) 

## SE relay numbering for actuation without other relay
    Relay SE bank 1  116    
    Relay SE bank 2  216
    Relay SE bank 3  216
    Relay SE bank 4  216


