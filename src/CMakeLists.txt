# ============================================================================
# Trading Tool - Python Bindings
# src/CMakeLists.txt
# ============================================================================

# Python module using pybind11
pybind11_add_module(trading_core 
    bindings.cpp
)

# Link against our C++ libraries
target_link_libraries(trading_core PRIVATE 
    core_lib
    backtest_lib
    utils_lib
    Threads::Threads
)

# Module-specific compiler flags
if(CMAKE_BUILD_TYPE MATCHES Release)
    # Strip symbols in release for smaller binary
    if(UNIX AND NOT APPLE)
        target_link_options(trading_core PRIVATE -s)
    endif()
    
    # Additional optimization for the Python module
    target_compile_options(trading_core PRIVATE
        -fvisibility=hidden  # Hide symbols by default
    )
endif()

# Set output name (Python will import as 'trading_core')
set_target_properties(trading_core PROPERTIES
    OUTPUT_NAME "trading_core"
    PREFIX ""  # No 'lib' prefix
    SUFFIX ".so"  # Force .so even on macOS (Python convention)
)

# Install the module to the trading_bot package directory
install(TARGETS trading_core
    LIBRARY DESTINATION trading_bot
    COMPONENT python
)

message(STATUS "Configured trading_core Python module")
message(STATUS "  Will be installed to: trading_bot/trading_core.so")
