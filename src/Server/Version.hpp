#pragma once

#include <cstdint>

namespace Server::Version
{

	/// \brief Get the major version of the server application
	auto getMajor() -> std::uint32_t;

	/// \brief Get the minor version of the server application
	auto getMinor() -> std::uint32_t;

	/// \brief Get the patch version of the server application
	auto getPatch() -> std::uint32_t;

	/// \brief Get the git short hash of the commit the server application was built with
	auto getCommit() -> std::uint32_t;

} // namespace Server::Version