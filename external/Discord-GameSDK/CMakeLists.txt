fetchcontent_declare(
  discord-gamesdk
  URL https://dl-game-sdk.discordapp.net/3.2.1/discord_game_sdk.zip
  URL_HASH MD5=73e5e1b3f8413a2c7184ef17476822f2)

fetchcontent_makeavailable(discord-gamesdk)

file(RELATIVE_PATH DISCORD_GAMESDK_SOURCE_DIR ${CMAKE_BINARY_DIR} ${discord-gamesdk_SOURCE_DIR})
set(DISCORD_GAMESDK_SOURCE_DIR
    ${DISCORD_GAMESDK_SOURCE_DIR}
    CACHE INTERNAL "" FORCE)
set(DISCORD_GAMESDK_SOURCE_DIR_ABSOLUTE
    ${discord-gamesdk_SOURCE_DIR}
    CACHE INTERNAL "" FORCE)

set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

add_library(discord-gamesdk)
add_library(Discord::GameSDK ALIAS discord-gamesdk)

add_library(discord-gamesdk-impl SHARED IMPORTED)

target_sources(
  discord-gamesdk
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

target_include_directories(discord-gamesdk PUBLIC ${discord-gamesdk_SOURCE_DIR}/cpp)
target_precompile_headers(discord-gamesdk PRIVATE <cstdint>)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  set(DISCORD_GAMESDK_LIBRARY
      ${DISCORD_GAMESDK_SOURCE_DIR}/lib/x86_64/discord_game_sdk.dll
      CACHE INTERNAL "" FORCE)
  set_target_properties(
    discord-gamesdk-impl PROPERTIES IMPORTED_LOCATION ${discord-gamesdk_SOURCE_DIR/lib/x86_64/discord_game_sdk.dll}
                                    IMPORTED_IMPLIB ${discord-gamesdk_SOURCE_DIR}/lib/x86_64/discord_game_sdk.dll.lib)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(DISCORD_GAMESDK_LIBRARY
      ${DISCORD_GAMESDK_SOURCE_DIR}/lib/x86_64/discord_game_sdk.so
      CACHE INTERNAL "" FORCE)
  set_target_properties(discord-gamesdk-impl PROPERTIES IMPORTED_LOCATION
                                                        ${discord-gamesdk_SOURCE_DIR}/lib/x86_64/discord_game_sdk.so)
else()
  set(DISCORD_GAMESDK_LIBRARY
      ${DISCORD_GAMESDK_SOURCE_DIR}/lib/x86_64/discord_game_sdk.dylib
      CACHE INTERNAL "" FORCE)
  set_target_properties(discord-gamesdk-impl PROPERTIES IMPORTED_LOCATION
                                                        ${discord-gamesdk_SOURCE_DIR}/lib/x86_64/discord_game_sdk.dylib)
endif()

target_link_libraries(discord-gamesdk INTERFACE discord-gamesdk-impl)
