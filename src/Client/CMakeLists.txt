add_executable(mmorpg-client)
add_executable(MMORPG::Client ALIAS mmorpg-client)

target_sources(
  mmorpg-client
  PRIVATE Main.cpp
          Version.cpp
          Discord/DiscordManager.cpp
          Discord/Secrets.cpp
          Network/NetworkManager.cpp
          Assets/AssetManager.cpp
          Engine/Engine.cpp
          Graphics/TextureAtlas.cpp
          Input/InputManager.cpp
          Network/NetworkManager.cpp
          States/Game.cpp
          States/Login.cpp
          UI/UI.cpp
          World/TerrainTile.cpp
          World/TerrainRenderer.cpp)

target_sources(
  mmorpg-client
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
  mmorpg-client
  PRIVATE ${discord-gamesdk_SOURCE_DIR}/cpp
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

add_library(discord_game_sdk SHARED IMPORTED)
set_target_properties(discord_game_sdk PROPERTIES IMPORTED_LOCATION
                                                  ${discord-gamesdk_SOURCE_DIR}/lib/x86_64/discord_game_sdk.so)

target_link_libraries(mmorpg-client PRIVATE MMORPG::Common SFML::Graphics SFML::Audio nlohmann_json::nlohmann_json
                                            discord_game_sdk)
target_compile_features(mmorpg-client PRIVATE cxx_std_20)

target_precompile_headers(mmorpg-client PRIVATE <cstdint> PCH.hpp)

set(CLIENT_VERSION_MAJOR 0)
set(CLIENT_VERSION_MINOR 1)
set(CLIENT_VERSION_PATCH 0)
get_hash(CLIENT_VERSION_COMMIT)

set_target_properties(
  mmorpg-client PROPERTIES VERSION ${CLIENT_VERSION_MAJOR}.${CLIENT_VERSION_MINOR}.${CLIENT_VERSION_PATCH}
                           SOVERSION ${CLIENT_VERSION_MAJOR})

set(CLIENT_DISCORD_APPLICATION_ID $ENV{DISCORD_APPLICATION_ID})
set(CLIENT_DISCORD_CLIENT_ID $ENV{DISCORD_CLIENT_ID})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Discord/Secrets.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Discord/Secrets.cpp)

add_custom_command(
  TARGET mmorpg-client
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client $<TARGET_FILE_DIR:mmorpg-client>/assets)
