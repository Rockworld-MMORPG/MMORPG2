#pragma once

#include <Common/Network/SerialisedComponent.hpp>
#include <SFML/System/Vector2.hpp>

namespace Server::Game
{

	struct Player : public Common::Network::SerialisedComponent<Player>
	{
		sf::Vector2f position;

		auto serialise(Common::Network::MessageData& data) -> void;
		auto deserialise(Common::Network::MessageData& data) -> void;
	};

} // namespace Server::Game