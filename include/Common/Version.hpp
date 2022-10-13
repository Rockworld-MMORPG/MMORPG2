#pragma once

#include <cstdint>

namespace Common::Version
{

	auto getMajor() -> std::uint32_t;
	auto getMinor() -> std::uint32_t;
	auto getPatch() -> std::uint32_t;
	auto getCommit() -> std::uint32_t;

} // namespace Common::Version