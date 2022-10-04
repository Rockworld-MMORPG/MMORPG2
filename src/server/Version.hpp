#pragma once

#include <cstdint>

namespace Server::Version
{

	auto getMajor() -> std::uint32_t;
	auto getMinor() -> std::uint32_t;
	auto getPatch() -> std::uint32_t;

} // namespace Server::Version