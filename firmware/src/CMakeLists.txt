# Project Cmakelist


# External Project Setup for SCPI parser
include(ExternalProject)
set(scpi_parser_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/scpi-parser/libscpi")
set(scpi_parser_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/scpi-parser")

ExternalProject_Add(scpi_parser_build
    SOURCE_DIR "${scpi_parser_SOURCE_DIR}"
    BUILD_IN_SOURCE ON
    CONFIGURE_COMMAND make clean
    BUILD_COMMAND env "CC=${CMAKE_C_COMPILER}" "CFLAGS=${CMAKE_C_FLAGS}" "PREFIX=${scpi_parser_PREFIX}" make static
    INSTALL_DIR "${scpi_parser_PREFIX}"
    INSTALL_COMMAND cp -r "${scpi_parser_SOURCE_DIR}/dist/libscpi.a" "${scpi_parser_SOURCE_DIR}/inc" "${scpi_parser_PREFIX}"
    BUILD_BYPRODUCTS "${scpi_parser_PREFIX}/libscpi.a"
)

# Define imported library for scpi_parser
add_library(scpi_parser STATIC IMPORTED)
set_property(TARGET scpi_parser PROPERTY IMPORTED_LOCATION "${scpi_parser_PREFIX}/libscpi.a")
add_dependencies(scpi_parser scpi_parser_build)
target_include_directories(scpi_parser INTERFACE "${scpi_parser_SOURCE_DIR}/inc")

# Main target setup
set(SOURCES_FILES master.c test.c i2c_com.c functadv.c fts_scpi.c scpi_spi.c scpi_i2c.c scpi_uart.c)
add_executable(${PROJECT_NAME} ${SOURCES_FILES})

# Add the dependencies for your executable
add_dependencies(${PROJECT_NAME} scpi_parser)



pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0) # temporary enable (1) to debug



add_library(fts_scpi INTERFACE) #DL
target_include_directories(fts_scpi INTERFACE ./include)
target_sources(fts_scpi INTERFACE fts_scpi.c)

add_library(i2c_com INTERFACE) #DL
target_include_directories(i2c_com INTERFACE ./include)
target_sources(i2c_com INTERFACE i2c_com.c)

add_library(functadv INTERFACE) #DL
target_include_directories(functadv INTERFACE ./include)
target_sources(functadv INTERFACE functadv.c)

add_library(test INTERFACE) #DL
target_include_directories(test INTERFACE ./include)
target_sources(test INTERFACE test.c)

add_library(scpi_spi INTERFACE) #DL
target_include_directories(scpi_spi INTERFACE ./include)
target_sources(test INTERFACE scpi_spi.c)

add_library(scpi_i2c INTERFACE) #DL
target_include_directories(scpi_i2c INTERFACE ./include)
target_sources(test INTERFACE scpi_i2c.c)

add_library(scpi_uart INTERFACE) #DL
target_include_directories(scpi_uart INTERFACE ./include)
target_sources(test INTERFACE scpi_uart.c)

add_subdirectory(pico_lib2)   # add Pico_lib2 to project

target_link_libraries(${PROJECT_NAME}
	pico_stdlib               # Core Pico library
	hardware_adc              # Hardware ADC support
	hardware_i2c              # Hardware I2C support
	hardware_spi              # Hardware spi support
	hardware_uart			  # Hardware UART support
	scpi_parser               # SCPI parser (external library)
	fts_scpi
	i2c_com                   # Custom I2C communication
	functadv                  # Your project's advanced functionality
	test                      # Test functions
	scpi_uart                 # UART-specific SCPI functions
	scpi_spi                  # SPI-specific SCPI functions
	scpi_i2c                  # I2C-specific SCPI functions
	lib2_sys                  # External system library
)


# Ensure output files are copied after build
set(UF2_ELF_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/firmware/build)
file(MAKE_DIRECTORY ${UF2_ELF_OUTPUT_DIR})
set(UF2_FILE ${CMAKE_BINARY_DIR}/firmware/src/${PROJECT_NAME}.uf2)
set(ELF_FILE ${CMAKE_BINARY_DIR}/firmware/src/${PROJECT_NAME}.elf)


# Copy the .uf2 file
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${UF2_FILE} ${UF2_ELF_OUTPUT_DIR}
    COMMENT "Copying .uf2 file to ${UF2_OUTPUT_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy ${UF2_FILE} ${CMAKE_BINARY_DIR}
	COMMENT "Copying .uf2 file to ${CMAKE_BINARY_DIR}"
)

# Copy the .elf file
add_custom_command(
    TARGET ${PROJECT_NAME}  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${ELF_FILE} ${UF2_ELF_OUTPUT_DIR}
    COMMENT "Copying .elf file to ${ELF_OUTPUT_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy ${ELF_FILE} ${CMAKE_BINARY_DIR}
	COMMENT "Copying .elf file to ${CMAKE_BINARY_DIR}"
)


