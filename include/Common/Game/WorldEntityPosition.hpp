#pragma once

#include "Common/Network/SerialisedComponent.hpp"
#include <SFML/System/Vector2.hpp>

namespace Common::Game
{

	/**
	 * \struct WorldEntityPosition WorldEntityPosition.hpp <Common/Game/WorldEntityPosition.hpp>
	 * \brief Data about the position of a world entity within the world space
	 */
	struct COMMON_API WorldEntityPosition : public Network::SerialisedComponent<WorldEntityPosition>
	{
		std::uint32_t instanceID = 0;
		sf::Vector2f position;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game