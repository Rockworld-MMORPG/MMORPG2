project(
  leveleditor
  VERSION 0.1.0
  LANGUAGES CXX)

add_executable(leveleditor Main.cpp TerrainRenderer.cpp TerrainTile.cpp)
add_executable(MMORPG::LevelEditor ALIAS leveleditor)

target_compile_features(leveleditor PRIVATE cxx_std_20)
target_link_libraries(leveleditor PRIVATE MMORPG::Common SFML::Graphics)

add_custom_command(
  TARGET leveleditor
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${leveleditor_SOURCE_DIR}/OpenSans-Regular.ttf
          $<TARGET_FILE_DIR:leveleditor>/OpenSans-Regular.ttf)
