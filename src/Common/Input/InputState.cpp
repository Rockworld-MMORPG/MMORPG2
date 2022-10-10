#include "Common/Input/InputState.hpp"

namespace Common::Input
{

	auto operator<<(Common::Network::MessageData& data, const InputState state) -> Network::MessageData&
	{
		data << state.forwards << state.backwards << state.left << state.right;
		return data;
	}

	auto operator>>(Common::Network::MessageData& data, InputState& state) -> Network::MessageData&
	{
		data >> state.forwards >> state.backwards >> state.left >> state.right;
		return data;
	}

} // namespace Common::Input