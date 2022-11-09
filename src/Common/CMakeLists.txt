add_library(mmorpg-common)
add_library(MMORPG::Common ALIAS mmorpg-common)

target_sources(
  mmorpg-common
  PRIVATE Game/WorldEntityGCD.cpp
          Game/WorldEntityPosition.cpp
          Game/WorldEntityType.cpp
          Game/WorldEntityName.cpp
          Game/WorldEntityStats.cpp
          Game/WorldEntity.cpp
          Input/Action.cpp
          Input/InputState.cpp
          Network/Crypto.cpp
          Network/Message.cpp
          Network/MessageData.cpp
          Network/MessageType.cpp
          World/Level.cpp
          World/Tile.cpp)

target_include_directories(
  mmorpg-common
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_link_libraries(mmorpg-common PUBLIC SFML::Network EnTT::EnTT spdlog::spdlog crypto nlohmann_json::nlohmann_json)
target_compile_features(mmorpg-common PRIVATE cxx_std_20)
target_compile_definitions(
  mmorpg-common PRIVATE _EXPORT_COMMON=TRUE
                        _BUILD_SHARED=$<IF:$<STREQUAL:$<TARGET_PROPERTY:mmorpg-common,TYPE>,SHARED_LIBRARY>,TRUE,FALSE>)

target_precompile_headers(mmorpg-common PRIVATE PCH.hpp)

set(COMMON_VERSION_MAJOR 0)
set(COMMON_VERSION_MINOR 1)
set(COMMON_VERSION_PATCH 0)
get_hash(COMMON_VERSION_COMMIT)

set_target_properties(
  mmorpg-common PROPERTIES VERSION ${COMMON_VERSION_MAJOR}.${COMMON_VERSION_MINOR}.${COMMON_VERSION_PATCH}
                           SOVERSION ${COMMON_VERSION_MAJOR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)
