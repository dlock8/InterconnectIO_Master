
set (IO_MASTER_VERSION_MAJOR 1)
set (IO_MASTER_VERSION_MINOR 8)

add_definitions(-DSCPI_USER_CONFIG=1)  #DL  flag to add scpi_user_config.h

# Build the SCPI parsing library (a Git submodule) as an external project.
# We'll build it in its source directory using its own Makefile, then place the
# .a file and the library's headers in scpi-parser for including/linking it.
include(ExternalProject)
set(scpi_parser_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/scpi-parser/libscpi")
set(scpi_parser_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/scpi-parser")
ExternalProject_Add(scpi_parser_build
	SOURCE_DIR "${scpi_parser_SOURCE_DIR}"
	BUILD_IN_SOURCE ON
	CONFIGURE_COMMAND make clean
	# Only generate the static version of the library, not the shared one (doesn't make sense here).
	BUILD_COMMAND env  "CC=${CMAKE_C_COMPILER}" "CFLAGS=${CMAKE_C_FLAGS}" "PREFIX=${scpi_parser_PREFIX}" make static
	INSTALL_DIR "${scpi_parser_PREFIX}"
	INSTALL_COMMAND cp -r "${scpi_parser_SOURCE_DIR}/dist/libscpi.a" "${scpi_parser_SOURCE_DIR}/inc" "${scpi_parser_PREFIX}"
)

message(STATUS ">>>DIRECTORY USED")
message(STATUS "Source= ${PROJECT_SOURCE_DIR}")
message(STATUS "Binary=${PROJECT_BINARY_DIR}")

# Configure Major and minor version 
configure_file (
  "${PROJECT_SOURCE_DIR}/IO_master/include/userconfig.h.in"
 "${PROJECT_BINARY_DIR}/IO_master/include/userconfig.h"  ) 
include_directories("${PROJECT_BINARY_DIR}/IO_master/include") 



# After the SCPI library has been built (using the "scpi_parser_build"
# project), we define an "IMPORTED" library to be able to link to it and
# include it.
add_library(scpi_parser STATIC IMPORTED)
set_property(TARGET scpi_parser PROPERTY IMPORTED_LOCATION "${scpi_parser_PREFIX}/libscpi.a")
add_dependencies(scpi_parser scpi_parser_build)
target_link_libraries(scpi_parser INTERFACE "${scpi_parser_PREFIX}/libscpi.a")
target_include_directories(scpi_parser INTERFACE "${scpi_parser_SOURCE_DIR}/inc")


add_library(fts_scpi INTERFACE) #DL
target_include_directories(fts_scpi INTERFACE ./include)
target_sources(fts_scpi INTERFACE fts_scpi.c)

add_library(i2c_com INTERFACE) #DL
target_include_directories(i2c_com INTERFACE ./include)
target_sources(i2c_com INTERFACE i2c_com.c)


add_executable(master master.c)

pico_enable_stdio_usb(master 1)
pico_enable_stdio_uart(master 1)

pico_add_extra_outputs(master)

target_link_libraries(master
	pico_stdlib
	scpi_parser
	fts_scpi
	hardware_i2c
	i2c_com
)
