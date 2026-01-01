# ============================================================================
# Backtest Library
# ============================================================================

add_library(backtest_lib
    backtest_engine.cpp
)

target_link_libraries(backtest_lib
    PUBLIC
        utils_lib
)

message(STATUS "Configured backtest_lib")
