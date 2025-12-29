# ============================================================================
# Trading Tool - Utils Library
# src/utils/CMakeLists.txt
# ============================================================================

# Utils library sources
set(UTILS_SOURCES
    logger.cpp
)

# Create static library
add_library(utils_lib STATIC 
    ${UTILS_SOURCES}
)

# Link dependencies
target_link_libraries(utils_lib 
    PUBLIC 
        pybind11::module
)

# Set library properties
set_target_properties(utils_lib PROPERTIES
    OUTPUT_NAME "trading_utils"
    POSITION_INDEPENDENT_CODE ON
)

# C++20
set_target_properties(utils_lib PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS utils_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

message(STATUS "Configured utils_lib with ${CMAKE_CURRENT_SOURCE_DIR}")