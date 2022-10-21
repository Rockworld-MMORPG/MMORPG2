#pragma once

#include "SFML/Graphics/Texture.hpp"
#include "UI/Layer.hpp"
#include <SFML/System/Vector2.hpp>
#include <entt/fwd.hpp>

namespace Client::UI
{

	auto createImage(entt::registry& registry, entt::entity element, sf::Vector2f position, sf::Vector2f size, sf::Texture& texture) -> void;
	auto createImage(entt::registry& registry, std::string identifier, Layer layer, sf::Vector2f position, sf::Vector2f size, sf::Texture& texture) -> void;

} // namespace Client::UI