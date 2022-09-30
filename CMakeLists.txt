cmake_minimum_required(VERSION 3.24)
project(mmorpg
	VERSION 0.1.0.0
	LANGUAGES C CXX
)
enable_testing()

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (isMultiConfig)
	if (NOT "DebugStrict" IN_LIST CMAKE_CONFIGURATION_TYPES)
		list(APPEND CMAKE_CONFIGURATION_TYPES DebugStrict)
	endif()
else()
	set(allowedBuildTypes Debug Release DebugStrict)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
	if (NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
	elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
		message(FATAL_ERROR "Build type not recognised: ${CMAKE_BUILD_TYPE}")
	endif()
endif()

set(CMAKE_C_FLAGS_DEBUGSTRICT 
	"${CMAKE_C_FLAGS_DEBUG}"
	CACHE STRING ""
)

set(CMAKE_CXX_FLAGS_DEBUGSTRICT 
	"${CMAKE_CXX_FLAGS_DEBUG}"
	CACHE STRING ""
)

set(CMAKE_EXE_LINKER_FLAGS_DEBUGSTRICT 
	"${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
	CACHE STRING ""
)

set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSTRICT 
	"${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
	CACHE STRING ""
)

set(CMAKE_STATIC_LINKER_FLAGS_DEBUGSTRICT 
	"${CMAKE_STATIC_LINKER_FLAGS_DEBUG}"
	CACHE STRING ""
)

set(CMAKE_MODULE_LINKER_FLAGS_DEBUGSTRICT 
	"${CMAKE_MODULE_LINKER_FLAGS_DEBUG}"
	CACHE STRING ""
)

# External dependencies
add_subdirectory(external)

# Project targets
add_subdirectory(src)

# Tests, if we're the main project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_subdirectory(tests)
endif()