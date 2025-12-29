# ============================================================================
# Trading Tool - Core Library
# src/core/CMakeLists.txt
# ============================================================================

# Core library sources
set(CORE_SOURCES
    indicators.cpp
    market_manager.cpp
    thread_pool.cpp
    trading_core.cpp
)

# Future sources (uncomment when implemented)
# set(BACKTEST_SOURCES
#     backtest_engine.cpp
#     order_manager.cpp
#     position_manager.cpp
#     risk_engine.cpp
# )

# Create static library
add_library(core_lib STATIC 
    ${CORE_SOURCES}
    # ${BACKTEST_SOURCES}  # Uncomment when ready
)

# Link dependencies
target_link_libraries(core_lib 
    PUBLIC 
        Threads::Threads
        pybind11::module
)

# Set library properties
set_target_properties(core_lib PROPERTIES
    OUTPUT_NAME "trading_core"
    POSITION_INDEPENDENT_CODE ON  # Required for shared library
)

# Installation
install(TARGETS core_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# Export compile commands for IDE integration
set_target_properties(core_lib PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

message(STATUS "Configured core_lib with ${CMAKE_CURRENT_SOURCE_DIR}")