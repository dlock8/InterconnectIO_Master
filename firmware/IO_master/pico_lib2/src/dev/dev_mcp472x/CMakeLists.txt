MESSAGE(STATUS "sources = mcp472x")
if (NOT TARGET mcp472x)
    MESSAGE(STATUS "sources = mcp4725 ${CMAKE_CURRENT_LIST_DIR}")
    add_library(mcp472x INTERFACE)
    target_include_directories(mcp472x INTERFACE ${CMAKE_CURRENT_LIST_DIR})
    target_sources(mcp472x INTERFACE ${CMAKE_CURRENT_LIST_DIR}/dev_mcp4725.c)
    target_link_libraries(mcp472x INTERFACE lib2_sys)
endif()

