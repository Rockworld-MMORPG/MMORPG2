#pragma once

#include "Common/Export.hpp"
#include "Common/Input/ActionType.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Input
{

	struct Action
	{
		ActionType type;
		using State_t = bool;
		enum class State : State_t
		{
			Begin,
			End
		} state;
	};

	COMMON_API auto operator<<(Common::Network::MessageData& data, Action action) -> Network::MessageData&;
	COMMON_API auto operator>>(Common::Network::MessageData& data, Action& action) -> Network::MessageData&;

} // namespace Common::Input