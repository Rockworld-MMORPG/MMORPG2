#pragma once

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics/View.hpp>
#include <entt/entity/fwd.hpp>

namespace Client::UI
{

	class UIRenderer
	{
	public:
		UIRenderer(sf::RenderWindow& targetWindow);
		auto render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void;

	private:
		sf::View m_uiView;
	};

} // namespace Client::UI