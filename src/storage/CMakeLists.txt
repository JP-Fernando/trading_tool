# ============================================================================
# Trading Tool - Storage Library
# src/storage/CMakeLists.txt
# ============================================================================

# Storage sources
set(STORAGE_SOURCES
    tick_writer.cpp
    event_log.cpp
    metadata_store.cpp
)

# Create static library
add_library(storage_lib STATIC 
    ${STORAGE_SOURCES}
)

# Link dependencies
target_link_libraries(storage_lib 
    PUBLIC 
        utils_lib     # Logging and time utilities
)

# Optional: Link against PostgreSQL client library
# find_package(PostgreSQL)
# if(PostgreSQL_FOUND)
#     target_include_directories(storage_lib PRIVATE ${PostgreSQL_INCLUDE_DIRS})
#     target_link_libraries(storage_lib PRIVATE ${PostgreSQL_LIBRARIES})
# endif()

# Set library properties
set_target_properties(storage_lib PROPERTIES
    OUTPUT_NAME "trading_storage"
    POSITION_INDEPENDENT_CODE ON
)

# Installation
install(TARGETS storage_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

message(STATUS "Configured storage_lib with ${CMAKE_CURRENT_SOURCE_DIR}")