cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(INTERCONNECTIO C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

project(INTERCONNECTIO C CXX ASM)

add_subdirectory(IO_master)
#add_subdirectory(IO_slave)


