#include "Common/Input/Action.hpp"
#include "Common/Input/ActionType.hpp"

namespace Common::Input
{

	auto operator<<(Common::Network::MessageData& data, const Action action) -> Network::MessageData&
	{
		data << static_cast<ActionType_t>(action.type) << static_cast<Action::State_t>(action.state);
		return data;
	}

	auto operator>>(Common::Network::MessageData& data, Action& action) -> Network::MessageData&
	{
		auto type  = ActionType_t();
		auto state = Action::State_t();

		data >> type >> state;
		action.type  = static_cast<ActionType>(type);
		action.state = static_cast<Action::State>(state);
		return data;
	}

} // namespace Common::Input