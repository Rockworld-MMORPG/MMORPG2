#pragma once

#if defined(_WIN32)
// Windows
#	if defined(_EXPORT_COMMON)
// MinGW
#		if defined(_MSC_VER)
#			define COMMON_API __declspec(dllexport)
// MSVC
#		else
#			define COMMON_API __attribute__((dllexport))
#		endif
#	else
// MinGW
#		if defined(_MSC_VER)
#			define COMMON_API __declspec(dllimport)
// MSVC
#		else
#			define COMMON_API __attribute__((dllimport))
#		endif
#	endif
// Not Windows
#else
#	define COMMON_API __attribute__((visibility("default")))
#endif