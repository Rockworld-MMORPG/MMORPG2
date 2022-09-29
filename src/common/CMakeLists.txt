add_library(common)
target_sources(common PRIVATE common.cpp)
target_link_libraries(common PUBLIC SFML::Graphics)
set_target_properties(common PROPERTIES 
    VERSION 0.1.0
    SOVERSION 0
)
target_include_directories(common PRIVATE . PUBLIC ${mmorpg_SOURCE_DIR}/include)