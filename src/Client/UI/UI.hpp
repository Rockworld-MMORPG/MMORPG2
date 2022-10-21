#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <entt/entity/fwd.hpp>

namespace Client::UI
{

	using Layer = std::uint8_t;

	struct ImageCreateInfo
	{
		sf::Vector2f position;
		sf::Vector2f size;
		sf::Texture& texture;
	};

	struct TextCreateInfo
	{
		sf::Vector2f position;
		sf::Font& font;
		std::string string;
		std::uint32_t fontSize;
	};

	auto createElement(entt::registry& registry, std::string identifier, Layer layer) -> entt::entity;
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, ImageCreateInfo createInfo) -> entt::entity;
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextCreateInfo createInfo) -> entt::entity;

	auto destroyElement(entt::registry& registry, std::string identifier) -> void;

	auto update(entt::registry& registry, sf::Time deltaTime) -> void;
	auto handleEvents(entt::registry& registry, sf::Event& event) -> void;

	class UIRenderer
	{
	public:
		auto render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void;

	private:
		sf::View m_uiView;
	};

} // namespace Client::UI