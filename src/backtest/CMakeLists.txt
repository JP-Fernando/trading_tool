set(BACKTEST_SOURCES
    backtest_engine.cpp 
    # order_manager.cpp
    # position_manager.cpp
    # risk_engine.cpp
    # etc
)

add_library(backtest_lib STATIC ${BACKTEST_SOURCES})

target_link_libraries(backtest_lib 
    PUBLIC 
        core_lib 
        utils_lib
)

set_target_properties(backtest_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD 20
)