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

	/**
	 * \struct ElementData UI.hpp "UI/UI.hpp"
	 * \brief Data needed to support updating and rendering all UI elements
	 */
	struct ElementData
	{
		bool mouseOver = false;
		Layer layer    = 0;
		entt::hashed_string identifier;
		std::vector<entt::entity> children;
		sf::Drawable* drawable = nullptr;
		sf::FloatRect collider;
	};

	/**
	 * \struct ElementCallbacks UI.hpp "UI/UI.hpp"
	 * \brief Callbacks attached to a UI element
	 */
	struct ElementCallbacks
	{
		std::function<void(sf::Mouse::Button)> onPress;
		std::function<void(sf::Mouse::Button)> onRelease;
		std::function<void()> onEnter;
		std::function<void()> onExit;
	};

	/**
	 * \struct ImageCreateInfo UI.hpp "UI/UI.hpp"
	 * \brief Data used to create an Image UI element
	 */
	struct ImageCreateInfo
	{
		sf::Vector2f position;
		sf::Vector2f size;
		sf::Texture& texture;
	};

	/**
	 * \struct TextCreateInfo UI.hpp "UI/UI.hpp"
	 * \brief Data used to create a Text UI element
	 */
	struct TextCreateInfo
	{
		sf::Vector2f position;
		sf::Font& font;
		std::string string;
		std::uint32_t fontSize;
	};

	/**
	 * \struct SliderCreateInfo UI.hpp "UI/UI.hpp"
	 * \brief Data used to create a Slider UI element
	 */
	struct SliderCreateInfo
	{
		static const auto DEFAULT_MIN = std::int32_t(0I);
		static const auto DEFAULT_MAX = std::int32_t(100I);

		sf::Vector2f position;
		sf::Vector2f size;
		std::int32_t minimumValue = DEFAULT_MIN;
		std::int32_t maximumValue = DEFAULT_MAX;
	};

	/**
	 * \struct TextInputCreateInfo UI.hpp "UI/UI.hpp"
	 * \brief Data used to create a Text Input UI element
	 */
	struct TextInputCreateInfo
	{
		static const auto DEFAULT_MAX_LENGTH     = std::size_t(32U);
		static const auto DEFAULT_CHARACTER_SIZE = std::uint32_t(12U);
		static const auto NO_MASKING             = char(0);

		sf::Vector2f position;
		size_t maxLength       = DEFAULT_MAX_LENGTH;
		char maskingCharacter  = NO_MASKING;
		std::uint32_t textSize = DEFAULT_CHARACTER_SIZE;
		sf::Font& font;
	};

	/**
	 * \struct ButtonCreateInfo UI.hpp "UI/UI.hpp"
	 * \brief Data used to create a Button UI element
	 */
	struct ButtonCreateInfo
	{
		sf::Vector2f position;
		sf::Vector2f size;
		std::string text;
		sf::Font& font;
		std::optional<std::function<void(sf::Mouse::Button)>> onPressCallback;
		std::optional<std::function<void(sf::Mouse::Button)>> onReleaseCallback;
	};

	/**
	 * \struct SliderData UI.hpp "UI/UI.hpp"
	 * \brief Data that a Slider UI element represents
	 */
	struct SliderData
	{
		std::int32_t value        = 0;
		std::int32_t minimumValue = SliderCreateInfo::DEFAULT_MIN;
		std::int32_t maximumValue = SliderCreateInfo::DEFAULT_MAX;
	};

	/**
	 * \struct TextInputData UI.hpp "UI/UI.hpp"
	 * \brief Data that a TextInput UI element represents
	 */
	struct TextInputData
	{
		bool active           = false;
		char maskingCharacter = TextInputCreateInfo::NO_MASKING;
		size_t maxLength      = TextInputCreateInfo::DEFAULT_MAX_LENGTH;
		std::string input;
	};

	/**
	 * \brief Create a raw UI Element
	 *
	 * \param registry The registry to create the UI element in
	 * \param identifier The identifier of the UI element
	 * \param layer The layer to create the UI element on
	 * \return entt::entity The entity representing the UI element
	 */
	auto createElement(entt::registry& registry, std::string identifier, Layer layer) -> entt::entity;

	/**
	 * \brief Create an Image UI Element
	 *
	 * \param registry The registry to create the UI element in
	 * \param identifier The identifier of the UI element
	 * \param layer The layer to create the UI element on
	 * \param createInfo The data used to create the Image UI element
	 * \return entt::entity The entity representing the UI element
	 */
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, ImageCreateInfo createInfo) -> entt::entity;

	/**
	 * \brief Create a Text UI Element
	 *
	 * \param registry The registry to create the UI element in
	 * \param identifier The identifier of the UI element
	 * \param layer The layer to create the UI element on
	 * \param createInfo The data used to create the Text UI element
	 * \return entt::entity The entity representing the UI element
	 */
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextCreateInfo createInfo) -> entt::entity;

	/**
	 * \brief Create a Slider UI Element
	 *
	 * \param registry The registry to create the UI element in
	 * \param identifier The identifier of the UI element
	 * \param layer The layer to create the UI element on
	 * \param createInfo The data used to create the Slider UI element
	 * \return entt::entity The entity representing the UI element
	 */
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, SliderCreateInfo createInfo) -> entt::entity;

	/**
	 * \brief Create a Button UI Element
	 *
	 * \param registry The registry to create the UI element in
	 * \param identifier The identifier of the UI element
	 * \param layer The layer to create the UI element on
	 * \param createInfo The data used to create the UI element
	 * \return entt::entity The entity representing the Button UI element
	 */
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, ButtonCreateInfo createInfo) -> entt::entity;

	/**
	 * \brief Create a Text Input UI Element
	 *
	 * \param registry The registry to create the UI element in
	 * \param identifier The identifier of the UI element
	 * \param layer The layer to create the UI element on
	 * \param createInfo The data used to create the Text Input UI element
	 * \return entt::entity The entity representing the UI element
	 */
	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextInputCreateInfo createInfo) -> entt::entity;

	/**
	 * \brief Destroys a UI element
	 *
	 * \param registry The registry to remove the UI element from
	 * \param identifier The identifier of the UI element
	 */
	auto destroyElement(entt::registry& registry, std::string identifier) -> void;

	/**
	 * \brief Update all the UI elements
	 *
	 * \param registry The registry to update the UI elements in
	 * \param deltaTime The time taken for the last frame to update and render
	 */
	auto update(entt::registry& registry, sf::Time deltaTime) -> void;

	/**
	 * \brief Handle any events polled by the OS
	 *
	 * \param registry The registry to update the UI elements in
	 * \param event The event polled by the OS
	 * \return true The event has been used by the UI
	 * \return false The event has not been used by the UI
	 */
	auto handleEvents(entt::registry& registry, sf::Event& event) -> bool;

	/**
	 * \class UIRenderer UI.hpp "UI/UI.hpp"
	 * \brief Renders UI elements to a render target with a screen-space UI camera
	 */
	class UIRenderer
	{
	public:
		/**
		 * \brief Resize the view of the UI camera
		 *
		 * \param newSize The new size of the view (likely the window size)
		 */
		auto resize(sf::Vector2f newSize) -> void;

		/**
		 * \brief Render all UI elements to the screen
		 *
		 * \param registry The registry to render the UI elements from
		 * \param renderTarget The target to render the UI elements to
		 */
		auto render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void;

	private:
		sf::View m_uiView;
	};

} // namespace Client::UI