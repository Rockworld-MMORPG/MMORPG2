add_executable(server)
target_sources(server PRIVATE main.cpp)
target_link_libraries(server PRIVATE common)
target_compile_features(server PRIVATE cxx_std_20)