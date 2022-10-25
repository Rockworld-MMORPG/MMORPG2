#pragma once

#include "Common/Export.hpp"
#include <cstdint>

namespace Common::Input
{

	using ActionType_t = std::uint32_t;
	enum class ActionType : ActionType_t
	{
		None = 0x00,

		MoveForward  = 0x01,
		MoveBackward = 0x02,
		StrafeLeft   = 0x03,
		StrafeRight  = 0x04,
		RotateLeft   = 0x05,
		RotateRight  = 0x06,

		Attack = 0x10,
		Use    = 0x11
	};

} // namespace Common::Input