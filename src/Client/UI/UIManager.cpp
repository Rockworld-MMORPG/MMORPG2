#include "UI/UIManager.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/System/Vector2.hpp"
#include "entt/core/hashed_string.hpp"
#include <UI/Layer.hpp>
#include <entt/entity/registry.hpp>
#include <map>
#include <spdlog/spdlog.h>

namespace Client::UI
{

	UIManager::UIManager(sf::RenderWindow& targetWindow)
	{
		auto windowSize = static_cast<sf::Vector2f>(targetWindow.getSize());
		m_uiView.setCenter(windowSize * 0.5F);
		m_uiView.setSize(windowSize);
	}

	auto UIManager::update(const sf::Time deltaTime, sf::RenderTarget& renderTarget) -> void
	{
		m_uiView.setSize(static_cast<sf::Vector2f>(renderTarget.getSize()));
	}

	auto UIManager::render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void
	{
		renderTarget.setView(m_uiView);

		std::multimap<std::uint8_t, entt::entity> elementsToDraw;
		for (const auto entity : registry.view<Layer>())
		{
			auto layer = registry.get<Layer>(entity).index;
			elementsToDraw.emplace(layer, entity);
		}

		for (const auto [layer, entity] : elementsToDraw)
		{
			auto& drawable = registry.get<sf::Sprite>(entity);
			renderTarget.draw(drawable);
		}
	}

	auto UIManager::removeUIElement(entt::registry& registry, const std::string& identifier) -> void
	{
		auto targetEntity = entt::entity(entt::null);
		for (const auto entity : registry.view<Layer, entt::hashed_string>())
		{
			if (registry.get<entt::hashed_string>(entity) == entt::hashed_string(identifier.c_str()))
			{
				registry.destroy(entity);
				return;
			}
		}
	}

} // namespace Client::UI