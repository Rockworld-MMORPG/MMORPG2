add_executable(client)
target_sources(client PRIVATE main.cpp)
target_link_libraries(client PRIVATE common)
target_compile_features(client PRIVATE cxx_std_20)