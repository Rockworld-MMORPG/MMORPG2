#pragma once

#include <cstdint>

namespace Common::Version
{

	/// \brief Get the major version of the common library
	auto getMajor() -> std::uint32_t;

	/// \brief Get the minor version of the common library
	auto getMinor() -> std::uint32_t;

	/// \brief Get the patch version of the common library
	auto getPatch() -> std::uint32_t;

	/// \brief Get the git short hash of the commit the common library was built with
	auto getCommit() -> std::uint32_t;

} // namespace Common::Version