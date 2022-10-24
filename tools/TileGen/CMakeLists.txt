project(
  tilegen
  VERSION 0.1.0
  LANGUAGES CXX)

add_executable(tilegen Main.cpp)
add_executable(MMORPG::TileGen ALIAS tilegen)

target_compile_features(tilegen PRIVATE cxx_std_20)
target_link_libraries(tilegen PRIVATE MMORPG::Common SFML::Graphics nlohmann_json::nlohmann_json)

add_custom_command(
  TARGET tilegen
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client $<TARGET_FILE_DIR:tilegen>/assets)
