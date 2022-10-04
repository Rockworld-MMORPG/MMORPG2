#pragma once

#include <cstdint>

namespace Client::Version
{

	auto getMajor() -> std::uint32_t;
	auto getMinor() -> std::uint32_t;
	auto getPatch() -> std::uint32_t;

} // namespace Client::Version