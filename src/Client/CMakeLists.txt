add_executable(client)
add_executable(MMORPG::Client ALIAS client)

target_sources(
  client
  PRIVATE Main.cpp
          Version.cpp
          Discord/DiscordManager.cpp
          Network/NetworkManager.cpp
          Engine/Engine.cpp
          Game/Game.cpp
          Input/InputManager.cpp
          World/Level.cpp
          World/TerrainTile.cpp
          World/TerrainRenderer.cpp)

target_sources(
  client
  PRIVATE ${discord-gamesdk_SOURCE_DIR}/cpp/achievement_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/activity_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/application_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/core.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/image_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/lobby_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/network_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/overlay_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/relationship_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/storage_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/store_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/types.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/user_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/voice_manager.cpp)

target_include_directories(
  client
  PRIVATE ${discord-gamesdk_SOURCE_DIR}/cpp
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

add_library(discord_game_sdk SHARED IMPORTED)
set_target_properties(discord_game_sdk PROPERTIES IMPORTED_LOCATION
                                                  ${discord-gamesdk_SOURCE_DIR}/lib/x86_64/discord_game_sdk.so)

target_link_libraries(client PRIVATE common SFML::Graphics SFML::Audio)
target_link_libraries(client PRIVATE discord_game_sdk)
target_precompile_headers(client PRIVATE <cstdint>)
target_compile_features(client PRIVATE cxx_std_20)

set(CLIENT_VERSION_MAJOR 0)
set(CLIENT_VERSION_MINOR 1)
set(CLIENT_VERSION_PATCH 0)

set_target_properties(client PROPERTIES VERSION ${CLIENT_VERSION_MAJOR}.${CLIENT_VERSION_MINOR}.${CLIENT_VERSION_PATCH}
                                        SOVERSION ${CLIENT_VERSION_MAJOR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)

add_custom_command(
  TARGET client
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client $<TARGET_FILE_DIR:client>/assets)
