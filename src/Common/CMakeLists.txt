add_library(common)
add_library(MMORPG::Common ALIAS common)

target_sources(
  common
  PRIVATE Common.cpp
          Network/Message.cpp
          Network/MessageData.cpp
          Network/MessageType.cpp
          Input/Action.cpp
          Input/InputState.cpp
          World/Level.cpp
          World/Tile.cpp)

target_include_directories(
  common
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_link_libraries(common PUBLIC SFML::Network EnTT::EnTT spdlog::spdlog)
target_compile_features(common PRIVATE cxx_std_20)
target_compile_definitions(
  common PRIVATE _EXPORT_COMMON=TRUE
                 _BUILD_SHARED=$<IF:$<STREQUAL:$<TARGET_PROPERTY:common,TYPE>,SHARED_LIBRARY>,TRUE,FALSE>)

set(COMMON_VERSION_MAJOR 0)
set(COMMON_VERSION_MINOR 1)
set(COMMON_VERSION_PATCH 0)
get_hash(COMMON_VERSION_COMMIT)

set_target_properties(common PROPERTIES VERSION ${COMMON_VERSION_MAJOR}.${COMMON_VERSION_MINOR}.${COMMON_VERSION_PATCH}
                                        SOVERSION ${COMMON_VERSION_MAJOR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)
