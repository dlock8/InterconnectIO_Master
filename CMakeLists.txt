# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Main Cmake


# Display a message with the required CMake version
message(STATUS "CMake minimum required version: 3.18")

# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.18)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(pico_sdk_import.cmake)
pico_sdk_init()



project(INTERCONNECTIO_MASTER C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


set(PICOTOOL_FETCH_FROM_GIT_PATH "~/pico")  # the directory where you want picotool to be fetched

set (IO_MASTER_VERSION_MAJOR 1)
set (IO_MASTER_VERSION_MINOR 1)

add_definitions(-DSCPI_USER_CONFIG=1)  #DL  flag to add scpi_user_config.h


# Configure Major and minor version 
configure_file (
  "${PROJECT_SOURCE_DIR}/firmware/src/include/userconfig.h.in"
 "${PROJECT_BINARY_DIR}/firmware/src/include/userconfig.h"  ) 
include_directories("${PROJECT_BINARY_DIR}/firmware/src/include") 

add_subdirectory(firmware/src)


