project(
  mmorpg-tilegen
  VERSION 0.1.0
  LANGUAGES CXX)

add_executable(mmorpg-tilegen Main.cpp)
add_executable(MMORPG::mmorpg-tilegen ALIAS mmorpg-tilegen)

target_compile_features(mmorpg-tilegen PRIVATE cxx_std_20)
target_link_libraries(mmorpg-tilegen PRIVATE MMORPG::Common SFML::Graphics nlohmann_json::nlohmann_json)

add_custom_command(
  TARGET mmorpg-tilegen
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/Client
          $<TARGET_FILE_DIR:mmorpg-tilegen>/assets)
