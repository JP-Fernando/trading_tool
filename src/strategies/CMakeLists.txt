# ============================================================================
# Trading Tool - Strategies Library
# src/strategies/CMakeLists.txt
# ============================================================================

# Strategy sources
set(STRATEGY_SOURCES
    base_strategy.cpp
    mean_reversion.cpp
    trend_following.cpp
    regime_detector.cpp
)

# Create static library
add_library(strategies_lib STATIC 
    ${STRATEGY_SOURCES}
)

# Link dependencies
target_link_libraries(strategies_lib 
    PUBLIC 
        core_lib      # Strategies depend on core indicators
        utils_lib     # And utilities like logging
)

# Set library properties
set_target_properties(strategies_lib PROPERTIES
    OUTPUT_NAME "trading_strategies"
    POSITION_INDEPENDENT_CODE ON
)

# Installation
install(TARGETS strategies_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

message(STATUS "Configured strategies_lib with ${CMAKE_CURRENT_SOURCE_DIR}")