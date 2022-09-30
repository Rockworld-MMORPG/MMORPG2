add_executable(client)
add_executable(MMORPG::Client ALIAS client)
target_sources(client PRIVATE main.cpp)
target_link_libraries(client PRIVATE common)
target_compile_features(client PRIVATE cxx_std_20)

add_custom_command(
    TARGET client
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${mmorpg_SOURCE_DIR}/assets/client $<TARGET_FILE_DIR:client>/assets
)