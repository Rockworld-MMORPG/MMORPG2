add_library(common)
add_library(MMORPG::Common ALIAS common)

target_sources(common PRIVATE Common.cpp Network/MessageType.cpp Network/ClientID.cpp)
target_include_directories(
  common
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_link_libraries(common PUBLIC SFML::Graphics SFML::Network SFML::Audio EnTT::EnTT spdlog::spdlog)

set_target_properties(common PROPERTIES VERSION 0.1.0 SOVERSION 0)
