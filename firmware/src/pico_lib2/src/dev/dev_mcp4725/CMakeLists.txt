MESSAGE(STATUS "sources = mcp4725")
if (NOT TARGET dev_mcp4725)
    MESSAGE(STATUS "sources = mcp4725 ${CMAKE_CURRENT_LIST_DIR}")
    add_library(dev_mcp4725 INTERFACE)
    target_include_directories(dev_mcp4725 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_sources(dev_mcp4725 INTERFACE ${CMAKE_CURRENT_LIST_DIR}/dev_mcp4725.c)
    target_link_libraries(dev_mcp4725 INTERFACE lib2_sys)
endif()

