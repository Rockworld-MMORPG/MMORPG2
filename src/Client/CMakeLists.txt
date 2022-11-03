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

target_include_directories(
  mmorpg-client
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_link_libraries(mmorpg-client PRIVATE MMORPG::Common SFML::Graphics SFML::Audio nlohmann_json::nlohmann_json
                                            Discord::GameSDK)

target_compile_features(mmorpg-client PRIVATE cxx_std_20)

target_precompile_headers(mmorpg-client PRIVATE PCH.hpp)

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

# TODO - figure out how to make this not just copy the entire relative path
add_custom_command(
  TARGET mmorpg-client
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/${DISCORD_GAMESDK_LIBRARY}
          $<TARGET_FILE_DIR:mmorpg-client>/${DISCORD_GAMESDK_LIBRARY})

add_custom_command(
  TARGET mmorpg-client
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client $<TARGET_FILE_DIR:mmorpg-client>/assets)
