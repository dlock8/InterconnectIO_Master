
# Main pico_lib2 Cmakelist

    if (NOT PICO_LIB2_PATH)
        set(PICO_LIB2_PATH ${CMAKE_CURRENT_LIST_DIR})
    endif()

    set(PICO_LIB2_PATH "${PICO_LIB2_PATH}" CACHE PATH "Path to Pico Lib2")


    message(STATUS "Pico_lib2_path= ${PICO_LIB2_PATH}")
   
    add_subdirectory(src)

