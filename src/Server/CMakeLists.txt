add_executable(mmorpg-server)
add_executable(MMORPG::Server ALIAS mmorpg-server)

target_sources(
  mmorpg-server
  PRIVATE Main.cpp
          Version.cpp
          Database/DatabaseManager.cpp
          Database/Secrets.cpp
          Login/LoginManager.cpp
          Network/NetworkManager.cpp
          Server/Server.cpp
          Shell/CommandShell.cpp)
target_include_directories(
  mmorpg-server
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${mmorpg_SOURCE_DIR}/include)

target_precompile_headers(mmorpg-server PRIVATE PCH.hpp)

set(SERVER_VERSION_MAJOR 0)
set(SERVER_VERSION_MINOR 1)
set(SERVER_VERSION_PATCH 0)
get_hash(SERVER_VERSION_COMMIT)

set_target_properties(
  mmorpg-server PROPERTIES VERSION ${SERVER_VERSION_MAJOR}.${SERVER_VERSION_MINOR}.${SERVER_VERSION_PATCH}
                           SOVERSION ${SERVER_VERSION_MAJOR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.cpp)

set(SERVER_DATABASE_URI $ENV{MONGODB_URI})
set(SERVER_DATABASE_CONNECTION_STRING $ENV{MONGODB_CONNECTION_STRING})
set(SERVER_DATABASE_USERNAME $ENV{MONGODB_USERNAME})
set(SERVER_DATABASE_PASSWORD $ENV{MONGODB_PASSWORD})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Database/Secrets.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/Database/Secrets.cpp)

target_link_libraries(mmorpg-server PRIVATE MMORPG::Common Argon2::Argon2-Hpp Mongo::MongoCXX)

target_compile_features(mmorpg-server PRIVATE cxx_std_20)
