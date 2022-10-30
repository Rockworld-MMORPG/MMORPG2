#pragma once

#include "Common/Network/SerialisedComponent.hpp"
#include <SFML/System/Vector2.hpp>

namespace Common::Game
{

	/**
	 * \struct WorldPosition WorldPosition.hpp <Common/Game/WorldPosition.hpp>
	 * \brief Data about the position of a world entity within the world space
	 */
	struct COMMON_API WorldPosition : public Network::SerialisedComponent<WorldPosition>
	{
		std::uint32_t instanceID = 0;
		sf::Vector2f position;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game