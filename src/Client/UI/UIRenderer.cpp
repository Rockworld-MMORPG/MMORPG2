#include "UI/UIRenderer.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/System/Vector2.hpp"
#include <UI/Layer.hpp>
#include <entt/entity/registry.hpp>
#include <map>
#include <spdlog/spdlog.h>

namespace Client::UI
{

	UIRenderer::UIRenderer(sf::RenderWindow& targetWindow)
	{
		auto windowSize = static_cast<sf::Vector2f>(targetWindow.getSize());
		m_uiView.setCenter(windowSize * 0.5F);
		m_uiView.setSize(windowSize);
	}

	auto UIRenderer::render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void
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

} // namespace Client::UI