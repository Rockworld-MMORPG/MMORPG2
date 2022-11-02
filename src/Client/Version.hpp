#pragma once

#include <cstdint>

namespace Client::Version
{

	/// \brief Get the major version of the client application
	auto getMajor() -> std::uint32_t;

	/// \brief Get the minor version of the client application
	auto getMinor() -> std::uint32_t;

	/// \brief Get the patch version of the client application
	auto getPatch() -> std::uint32_t;

	/// \brief Get the git short hash of the commit the client application was built with
	auto getCommit() -> std::uint32_t;


} // namespace Client::Version