#include "Common/Game/WorldEntityGCD.hpp"

namespace Common::Game
{

	auto WorldEntityGCD::serialise(Network::MessageData& data) -> void
	{
		data << resetTime.asMicroseconds() << currentTime.asMicroseconds();
	}

	auto WorldEntityGCD::deserialise(Network::MessageData& data) -> void
	{
		auto rTime = std::int64_t(0);
		auto cTime = std::int64_t(0);

		data >> rTime >> cTime;
		resetTime   = sf::microseconds(rTime);
		currentTime = sf::microseconds(cTime);
	}

} // namespace Common::Game