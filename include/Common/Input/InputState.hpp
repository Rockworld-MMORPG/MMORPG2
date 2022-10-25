#pragma once

#include "Common/Export.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Input
{

	struct InputState
	{
		bool forwards, backwards, left, right;
		bool changed;
	};

	COMMON_API auto operator<<(Common::Network::MessageData& data, InputState state) -> Network::MessageData&;
	COMMON_API auto operator>>(Common::Network::MessageData& data, InputState& state) -> Network::MessageData&;

} // namespace Common::Input