# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/pi/pico/InterconnectIO_Master/firmware/src/lib/scpi-parser/libscpi"
  "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/src/scpi_parser_build-build"
  "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi-parser"
  "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/tmp"
  "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/src/scpi_parser_build-stamp"
  "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/src"
  "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/src/scpi_parser_build-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/src/scpi_parser_build-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pi/pico/InterconnectIO_Master/build/firmware/src/scpi_parser_build-prefix/src/scpi_parser_build-stamp${cfgdir}") # cfgdir has leading slash
endif()
