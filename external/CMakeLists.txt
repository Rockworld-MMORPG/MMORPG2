include(FetchContent)

message(STATUS "Fetching dependencies")

message(STATUS "Fetching SFML")
add_subdirectory(SFML)

message(STATUS "Fetching EnTT")
add_subdirectory(EnTT)

message(STATUS "Fetching spdlog")
add_subdirectory(spdlog)

message(STATUS "Fetching Discord GameSDK")
add_subdirectory(Discord-GameSDK)

message(STATUS "Fetching Argon2-Hpp")
add_subdirectory(Argon2-Hpp)

message(STATUS "Fetching ImGui")
add_subdirectory(ImGui)

message(STATUS "Fetching ImGui-SFML")
add_subdirectory(ImGui-SFML)

message(STATUS "Fetching json")
add_subdirectory(json)

message(STATUS "Fetching OpenSSL")
add_subdirectory(OpenSSL)

message(STATUS "Fetching mongocxx")
add_subdirectory(mongocxx)
