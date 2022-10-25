#pragma once

#include "Common/Export.hpp"
#include "Common/Input/ActionType.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Input
{

	struct COMMON_API Action
	{
		ActionType type;
		using State_t = bool;
		enum class State : State_t
		{
			Begin,
			End
		} state;
	};

	auto operator<<(Common::Network::MessageData& data, Action action) -> Network::MessageData&;
	auto operator>>(Common::Network::MessageData& data, Action& action) -> Network::MessageData&;

} // namespace Common::Input