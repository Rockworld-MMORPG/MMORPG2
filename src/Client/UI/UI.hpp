#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Mouse.hpp>
#include <entt/core/hashed_string.hpp>
#include <entt/entity/fwd.hpp>
#include <functional>

namespace Client::UI
{

	using Layer = std::int16_t;

	struct ElementData
	{
		bool mouseOver = false;
		Layer layer    = 0;
		entt::hashed_string identifier;
		std::vector<entt::entity> children;
		sf::Drawable* drawable = nullptr;
		sf::FloatRect collider;
	};

	struct ElementCallbacks
	{
		std::function<void(sf::Mouse::Button)> onPress;
		std::function<void(sf::Mouse::Button)> onRelease;
		std::function<void()> onEnter;
		std::function<void()> onExit;
	};

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

	struct SliderCreateInfo
	{
		sf::Vector2f position;
		sf::Vector2f size;
		std::int32_t minimumValue = 0;
		std::int32_t maximumValue = 100;
	};

	struct TextInputCreateInfo
	{
		sf::Vector2f position;
		sf::Vector2f size;
		std::uint32_t textSize = 12;
		sf::Font& font;
	};

	struct ButtonCreateInfo
	{
		sf::Vector2f position;
		sf::Vector2f size;
		std::string text;
		sf::Font& font;
		std::optional<std::function<void(sf::Mouse::Button)>> onPressCallback;
		std::optional<std::function<void(sf::Mouse::Button)>> onReleaseCallback;
	};

	struct SliderData
	{
		float value               = 0.0F;
		std::int32_t minimumValue = 0;
		std::int32_t maximumValue = 100;
	};

	struct TextInputData
	{
		bool active = false;
		std::string input;
	};

	auto createElement(entt::registry& registry, std::string identifier, Layer layer) -> entt::entity;

	auto createElement(entt::registry& registry, std::string identifier, Layer layer, ImageCreateInfo createInfo) -> entt::entity;
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextCreateInfo createInfo) -> entt::entity;
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, SliderCreateInfo createInfo) -> entt::entity;
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, ButtonCreateInfo createInfo) -> entt::entity;
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextInputCreateInfo createInfo) -> entt::entity;

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