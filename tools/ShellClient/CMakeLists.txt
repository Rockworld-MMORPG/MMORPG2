project(
  shellclient
  VERSION 0.1.0
  LANGUAGES CXX)

add_executable(shellclient Main.cpp)
add_executable(MMORPG::ShellClient ALIAS shellclient)

target_compile_features(shellclient PRIVATE cxx_std_20)
target_link_libraries(shellclient PRIVATE MMORPG::Common)
