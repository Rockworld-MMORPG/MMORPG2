#pragma once

#include "Common/Export.hpp"
#include <cstdint>

namespace Common::Network
{

	enum class Protocol : std::uint8_t
	{
		TCP,
		UDP
	};

} // namespace Common::Network