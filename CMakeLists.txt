cmake_minimum_required(VERSION 3.24)
project(
  mmorpg
  VERSION 0.1.0.0
  LANGUAGES C CXX)

enable_testing()

option(MMORPG_USE_MOLD_LINKER "Force use the mold linker instead of the system default" OFF)

if(MMORPG_USE_MOLD_LINKER)
  # Use mold to link because it's far faster
  add_link_options(-fuse-ld=mold)
endif()

# Add CMake functions
include(cmake/GetHash.cmake)

# External dependencies
add_subdirectory(external)

# Project targets
add_subdirectory(src)

# Project tools
add_subdirectory(tools)

# Tests, if we're the main project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_subdirectory(tests)
endif()
