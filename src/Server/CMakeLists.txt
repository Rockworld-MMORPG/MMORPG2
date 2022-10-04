add_executable(server)
add_executable(MMORPG::Server ALIAS server)

target_sources(server PRIVATE Main.cpp EntityManager.cpp NetworkManager.cpp Version.cpp Message.cpp)
target_include_directories(
  server
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

set(SERVER_VERSION_MAJOR 0)
set(SERVER_VERSION_MINOR 1)
set(SERVER_VERSION_PATCH 0)

set_target_properties(server PROPERTIES VERSION ${SERVER_VERSION_MAJOR}.${SERVER_VERSION_MINOR}.${SERVER_VERSION_PATCH}
                                        SOVERSION ${SERVER_VERSION_MAJOR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)

target_link_libraries(server PRIVATE common)
target_compile_features(server PRIVATE cxx_std_20)
