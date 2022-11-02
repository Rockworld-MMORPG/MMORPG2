project(
  mmorpg-leveleditor
  VERSION 0.1.0
  LANGUAGES CXX)

add_executable(mmorpg-leveleditor Main.cpp TerrainRenderer.cpp TerrainTile.cpp)
add_executable(MMORPG::mmorpg-leveleditor ALIAS mmorpg-leveleditor)

target_compile_features(mmorpg-leveleditor PRIVATE cxx_std_20)
target_link_libraries(mmorpg-leveleditor PRIVATE MMORPG::Common SFML::Graphics ImGui-SFML::ImGui-SFML
                                                 nlohmann_json::nlohmann_json)

add_custom_command(
  TARGET mmorpg-leveleditor
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client
          $<TARGET_FILE_DIR:mmorpg-leveleditor>/assets)
