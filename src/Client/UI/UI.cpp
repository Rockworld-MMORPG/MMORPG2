#include "UI/UI.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/registry.hpp>
#include <map>
#include <spdlog/spdlog.h>

using entt::operator""_hs;

namespace Client::UI
{

	using UI_ELEMENT_TAG = entt::tag<"ui_element"_hs>;

	auto createElement(entt::registry& registry, std::string identifier, Layer layer) -> entt::entity
	{
		auto entity = registry.create();
		registry.emplace<entt::hashed_string>(entity, identifier.c_str(), identifier.size());
		registry.emplace<UI_ELEMENT_TAG>(entity);
		registry.emplace<Layer>(entity);

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, Layer layer, ImageCreateInfo createInfo) -> entt::entity
	{
		auto entity = createElement(registry, identifier, layer);

		auto& sprite = registry.emplace<sf::Sprite>(entity);
		sprite.setTexture(createInfo.texture);
		sprite.setPosition(createInfo.position);

		auto scaleX = sprite.getLocalBounds().width / createInfo.size.x;
		auto scaleY = sprite.getLocalBounds().height / createInfo.size.y;
		sprite.setScale(sf::Vector2f(scaleX, scaleY));

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextCreateInfo createInfo) -> entt::entity
	{
		auto entity = createElement(registry, identifier, layer);

		auto& text = registry.emplace<sf::Text>(entity);
		text.setFont(createInfo.font);
		text.setCharacterSize(createInfo.fontSize);
		text.setPosition(createInfo.position);
		text.setString(createInfo.string);

		return entity;
	}

	auto destroyElement(entt::registry& registry, std::string identifier) -> void
	{
		auto hsIdentifier = entt::hashed_string(identifier.c_str(), identifier.size());
		for (const auto entity : registry.view<UI_ELEMENT_TAG>())
		{
			if (registry.get<entt::hashed_string>(entity) == hsIdentifier)
			{
				registry.destroy(entity);
				return;
			}
		}
	}

	auto update(entt::registry& registry, const sf::Time deltaTime) -> void
	{
		for (const auto entity : registry.view<UI_ELEMENT_TAG>())
		{
		}
	}

	auto handleEvents(entt::registry& registry, sf::Event& event) -> void
	{
		switch (event.type)
		{
			case sf::Event::MouseMoved:
			{
			}
			break;
			case sf::Event::MouseButtonPressed:
			{
			}
			break;
			case sf::Event::MouseButtonReleased:
			{
			}
			break;
			case sf::Event::TextEntered:
			{
			}
			break;
			default:
				break;
		}
	}

	auto UIRenderer::render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void
	{
		auto targetSize = static_cast<sf::Vector2f>(renderTarget.getSize());
		m_uiView.setSize(targetSize);
		m_uiView.setCenter(targetSize * 0.5F);
		renderTarget.setView(m_uiView);

		std::multimap<std::uint8_t, entt::entity> elementsToDraw;
		for (const auto entity : registry.view<UI_ELEMENT_TAG>())
		{
			auto layer = registry.get<Layer>(entity);
			elementsToDraw.emplace(layer, entity);
		}

		for (const auto [layer, entity] : elementsToDraw)
		{
			if (registry.all_of<sf::Sprite>(entity))
			{
				auto& drawable = registry.get<sf::Sprite>(entity);
				renderTarget.draw(drawable);
			}
			else if (registry.all_of<sf::Text>(entity))
			{
				auto& text = registry.get<sf::Text>(entity);
				renderTarget.draw(text);
			}
		}
	}

} // namespace Client::UI