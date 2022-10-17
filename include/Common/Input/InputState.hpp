#pragma once

#include "Common/Network/MessageData.hpp"

namespace Common::Input
{

	struct InputState
	{
		bool forwards, backwards, left, right;
		bool changed;
	};

	auto operator<<(Common::Network::MessageData& data, InputState state) -> Network::MessageData&;
	auto operator>>(Common::Network::MessageData& data, InputState& state) -> Network::MessageData&;

} // namespace Common::Input