add_executable(client)
add_executable(MMORPG::Client ALIAS client)

target_sources(client PRIVATE Main.cpp Version.cpp)
target_link_libraries(client PRIVATE common)
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
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/client $<TARGET_FILE_DIR:client>/assets)
