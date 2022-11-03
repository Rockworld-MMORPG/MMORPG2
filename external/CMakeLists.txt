include(FetchContent)

message(STATUS "Fetching dependencies")

add_subdirectory(SFML)
add_subdirectory(EnTT)
add_subdirectory(spdlog)
add_subdirectory(Discord-GameSDK)
add_subdirectory(Argon2-Hpp)
add_subdirectory(ImGui)
add_subdirectory(ImGui-SFML)
add_subdirectory(json)
add_subdirectory(OpenSSL)
add_subdirectory(SQLiteCpp)
