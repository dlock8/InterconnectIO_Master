# pico_lib2 source development for the 24lc32
if (NOT TARGET dev_24lc32)
    add_library(dev_24lc32 INTERFACE)
    MESSAGE(STATUS "sources = 24lc32 ${CMAKE_CURRENT_LIST_DIR}")
    target_include_directories(dev_24lc32 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_sources(dev_24lc32 INTERFACE ${CMAKE_CURRENT_LIST_DIR}/dev_24lc32.c)
    target_link_libraries(dev_24lc32 INTERFACE lib2_sys)
endif()





