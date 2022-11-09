#pragma once

#include "Common/Export.hpp"
#include "Common/Network/SerialisedComponent.hpp"
#include <SFML/System/Time.hpp>

namespace Common::Game
{

	struct COMMON_API WorldEntityGCD : Network::SerialisedComponent<WorldEntityGCD>
	{
		sf::Time resetTime   = sf::seconds(1.0F);
		sf::Time currentTime = sf::Time::Zero;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game