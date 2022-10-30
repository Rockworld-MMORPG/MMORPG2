add_executable(client)
add_executable(MMORPG::Client ALIAS client)

target_sources(
  client
  PRIVATE Main.cpp
          Version.cpp
          Assets/AssetManager.cpp
          Engine/Engine.cpp
          Game/Game.cpp
          Graphics/TextureAtlas.cpp
          Input/InputManager.cpp
          Network/NetworkManager.cpp
          UI/UI.cpp
          World/TerrainTile.cpp
          World/TerrainRenderer.cpp)

target_include_directories(
  client
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_link_libraries(client PRIVATE common SFML::Graphics SFML::Audio nlohmann_json::nlohmann_json)
target_compile_features(client PRIVATE cxx_std_20)

target_precompile_headers(client PRIVATE PCH.hpp)

set(CLIENT_VERSION_MAJOR 0)
set(CLIENT_VERSION_MINOR 1)
set(CLIENT_VERSION_PATCH 0)
get_hash(CLIENT_VERSION_COMMIT)

set_target_properties(client PROPERTIES VERSION ${CLIENT_VERSION_MAJOR}.${CLIENT_VERSION_MINOR}.${CLIENT_VERSION_PATCH}
                                        SOVERSION ${CLIENT_VERSION_MAJOR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)

add_custom_command(
  TARGET client
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client $<TARGET_FILE_DIR:client>/assets)
