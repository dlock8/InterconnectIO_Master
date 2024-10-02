# pico_lib2 source development for the ds2431
if (NOT TARGET dev_ds2431)
    add_library(dev_ds2431 INTERFACE)
    MESSAGE(STATUS "sources = ds2431 ${CMAKE_CURRENT_LIST_DIR}")
    target_include_directories(dev_ds2431 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_sources(dev_ds2431 INTERFACE ${CMAKE_CURRENT_LIST_DIR}/dev_ds2431.c)
    target_link_libraries(dev_ds2431 INTERFACE lib2_sys)
endif()