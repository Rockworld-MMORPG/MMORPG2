project(
  mmorpg-shellclient
  VERSION 0.1.0
  LANGUAGES CXX)

add_executable(mmorpg-shellclient Main.cpp)
add_executable(MMORPG::mmorpg-shellclient ALIAS mmorpg-shellclient)

target_compile_features(mmorpg-shellclient PRIVATE cxx_std_20)
target_link_libraries(mmorpg-shellclient PRIVATE MMORPG::Common)
