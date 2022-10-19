#pragma once

#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include <SFML/Graphics/View.hpp>
#include <entt/entity/fwd.hpp>

namespace Client::UI
{

	class UIManager
	{
	public:
		UIManager(sf::RenderWindow& targetWindow);

		auto update(sf::Time deltaTime, sf::RenderTarget& renderTarget) -> void;
		auto render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void;
		auto removeUIElement(entt::registry& registry, const std::string& identifier) -> void;


	private:
		sf::View m_uiView;
	};

} // namespace Client::UI