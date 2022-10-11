fetchcontent_declare(
  discord-gamesdk
  URL https://dl-game-sdk.discordapp.net/3.2.1/discord_game_sdk.zip
  URL_HASH MD5=73e5e1b3f8413a2c7184ef17476822f2)

fetchcontent_makeavailable(discord-gamesdk)

add_library(gamesdk)
add_library(Discord::GameSDK ALIAS gamesdk)

target_sources(
  gamesdk
  PRIVATE ${discord-gamesdk_SOURCE_DIR}/cpp/lobby_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/relationship_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/store_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/core.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/network_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/image_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/voice_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/achievement_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/storage_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/overlay_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/types.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/activity_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/application_manager.cpp
          ${discord-gamesdk_SOURCE_DIR}/cpp/user_manager.cpp)

set_target_properties(gamesdk PROPERTIES VERSION 3.2.1 SOVERSION 3)

target_include_directories(gamesdk PUBLIC ${discord-gamesdk_SOURCE_DIR}/cpp)
target_precompile_headers(gamesdk PRIVATE <cstdint>)
