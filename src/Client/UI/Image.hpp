#pragma once

#include "SFML/Graphics/Texture.hpp"
#include <SFML/System/Vector2.hpp>
#include <entt/fwd.hpp>

namespace Client::UI
{

	auto createImage(entt::registry& registry, sf::Vector2f position, sf::Vector2f size, sf::Texture& texture) -> void;

} // namespace Client::UI