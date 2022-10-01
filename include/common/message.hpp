#pragma once

#include <cstdint>

namespace Common
{

	enum Message : std::uint32_t
	{
		Movement,
		Position,
		Terminate
	};

} // namespace Common