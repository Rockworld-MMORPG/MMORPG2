add_executable(server)
add_executable(MMORPG::Server ALIAS server)

target_sources(server PRIVATE main.cpp NetworkManager.cpp)
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(server PRIVATE common)
target_compile_features(server PRIVATE cxx_std_20)
