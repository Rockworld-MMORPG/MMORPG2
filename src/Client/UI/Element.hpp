#pragma once

#include "SFML/Graphics/Texture.hpp"
#include "UI/Layer.hpp"
#include <SFML/System/Vector2.hpp>
#include <entt/fwd.hpp>

namespace Client::UI
{

	auto createElement(entt::registry& registry, std::string identifier, Layer layer) -> entt::entity;

}