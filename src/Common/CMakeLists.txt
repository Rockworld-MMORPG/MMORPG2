add_library(common)
add_library(MMORPG::Common ALIAS common)

target_sources(
  common
  PRIVATE Common.cpp
          Network/Message.cpp
          Network/MessageData.cpp
          Network/MessageType.cpp
          Network/ClientID.cpp
          Input/Action.cpp
          Input/InputState.cpp)
target_include_directories(
  common
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_link_libraries(common PUBLIC SFML::Network EnTT::EnTT spdlog::spdlog)

set_target_properties(common PROPERTIES VERSION 0.1.0 SOVERSION 0)
