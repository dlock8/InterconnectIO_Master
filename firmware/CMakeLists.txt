# Main Cmake

cmake_minimum_required(VERSION 3.18)

# Set CMP0076 policy
cmake_policy(SET CMP0076 NEW)

include(pico_sdk_import.cmake)

project(INTERCONNECTIO_MASTER C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

project(INTERCONNECTIO_MASTER C CXX ASM)

message(STATUS "Cmake module directory: ${CMAKE_MODULE_PATH}")
message(STATUS "Cmake source directory: ${CMAKE_SOURCE_DIR}")

add_subdirectory(src)



