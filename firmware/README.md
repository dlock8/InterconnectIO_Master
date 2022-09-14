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

ROUTe:CLOSe (@<ch_list>)  
ROUTe:CLOSe:EXCLusive (@<ch_list>) 
ROUTe:CLOSe:Se [{BANK1-BANK4}]
ROUTe:OPEN (@<ch_list>)
ROUTe:OPEN:Se [{BANK1-BANK4}]
ROUTE:OPEN:ALL[{BANK1-BANK4|ALL}]
ROUTE:CHANnel:STATe? (@<ch_list>)
ROUTE:BANK:STATe? [{BANK1-BANK4}]
ROUTE:SE:STATe? [{BANK1-BANK4}]  

SYSTem:BEEPer

## SCPI example:
ROUTe:CLOSe (@101)            --> Close relay 100 and close relay Single ended (SE)
ROUTe:CLOSe:EXC (@102)        --> open relay on bank1, including relay SE and close relay 102
ROUTe:CLOSe (@100,108)        --> close relay 100 and 108
ROUTe:CLOSe (@100:108)        --> close relay 100,102,104,106, and 108
ROUTe:OPEN:ALL BANK1,BANK2    --> open relay  on bank1 and bank2
ROUTe:OPEN:ALL ALL            --> open relay  on all banks
ROUTe:SE:STATe? BANK1         --> read which side of diff relay we are connected (H or L)
ROUTe:CHAN:STATe? (@100)

## Relay numbering. Differential relay will be located on lower address. Single ended relay to be connected to high or low side following the address.
    Relay bank 1   100 @ 115      
    Relay bank 2   200 @ 215
    Relay bank 3   300 @ 315
    Relay bank 4   400 @ 415

    Relay bank 1   Differential: 10 @ 17    Single: 100 @ 107 (High) 108 @ 115 (Low)      
    Relay bank 2   Differential: 20 @ 27    Single: 200 @ 215 (High) 208 @ 215 (Low) 
    Relay bank 3   Differential: 30 @ 37    Single: 300 @ 315 (High) 308 @ 315 (Low) 
    Relay bank 4   Differential: 40 @ 47    Single: 400 @ 415 (High) 108 @ 415 (Low) 

## SE relay numbering for actuation without other relay
    Relay SE bank 1  116    
    Relay SE bank 2  216
    Relay SE bank 3  216
    Relay SE bank 4  216


