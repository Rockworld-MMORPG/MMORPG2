#pragma once

#include <cstdint>

namespace common
{

	enum Message : std::uint32_t
	{
		Movement,
		Position,
		Terminate
	};

} // namespace common