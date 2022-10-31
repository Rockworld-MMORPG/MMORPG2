#include "UI/UI.hpp"
#include <map>

using entt::operator""_hs;

namespace Client::UI
{

	auto createElement(entt::registry& registry, std::string identifier, const Layer layer) -> entt::entity
	{
		auto entity     = registry.create();
		auto& data      = registry.emplace<ElementData>(entity);
		data.layer      = layer;
		data.identifier = entt::hashed_string(identifier.c_str(), identifier.size());

		auto& callbacks   = registry.emplace<ElementCallbacks>(entity);
		callbacks.onPress = [](const sf::Mouse::Button) {
		};
		callbacks.onRelease = [](const sf::Mouse::Button) {
		};
		callbacks.onEnter = []() {
		};
		callbacks.onExit = []() {
		};

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, const Layer layer, const ImageCreateInfo createInfo) -> entt::entity
	{
		auto entity = createElement(registry, identifier, layer);

		auto& sprite = registry.emplace<sf::Sprite>(entity);
		sprite.setTexture(createInfo.texture);

		auto scaleX = sprite.getLocalBounds().width / createInfo.size.x;
		auto scaleY = sprite.getLocalBounds().height / createInfo.size.y;
		sprite.setScale(sf::Vector2f(scaleX, scaleY));
		sprite.setPosition(createInfo.position);

		auto& data    = registry.get<ElementData>(entity);
		data.drawable = &sprite;
		data.collider = sprite.getGlobalBounds();

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, const Layer layer, const TextCreateInfo createInfo) -> entt::entity
	{
		auto entity = createElement(registry, identifier, layer);

		auto& text = registry.emplace<sf::Text>(entity);
		text.setFont(createInfo.font);
		text.setCharacterSize(createInfo.fontSize);
		text.setString(createInfo.string);
		text.setPosition(createInfo.position);

		auto& data    = registry.get<ElementData>(entity);
		data.drawable = &text;

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, const Layer layer, const SliderCreateInfo createInfo) -> entt::entity
	{
		auto entity = createElement(registry, identifier, layer);

		auto& background = registry.emplace<sf::RectangleShape>(entity);
		background.setSize(createInfo.size);
		background.setFillColor(sf::Color::White);
		background.setOutlineColor(sf::Color::Black);
		background.setPosition(createInfo.position);

		auto& bgData    = registry.get<ElementData>(entity);
		bgData.drawable = &background;
		bgData.collider = background.getGlobalBounds();

		bgData.children.emplace_back(createElement(registry, identifier + "_handle", layer + 1));

		auto& handle = registry.emplace<sf::CircleShape>(bgData.children.front());
		handle.setRadius(createInfo.size.y * 0.5F);
		handle.setFillColor(sf::Color::White);
		handle.setOutlineColor(sf::Color::Black);
		handle.setPosition(createInfo.position + sf::Vector2f(-handle.getRadius(), 0.0F));

		auto& handleData    = registry.get<ElementData>(bgData.children.front());
		handleData.drawable = &handle;
		handleData.collider = handle.getGlobalBounds();

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, const Layer layer, const ButtonCreateInfo createInfo) -> entt::entity
	{
		auto entity = createElement(registry, identifier, layer);

		auto& rect = registry.emplace<sf::RectangleShape>(entity);
		rect.setSize(createInfo.size);
		rect.setFillColor(sf::Color::White);
		rect.setOutlineColor(sf::Color::Black);
		rect.setPosition(createInfo.position);

		auto& data    = registry.get<ElementData>(entity);
		data.drawable = &rect;
		data.collider = rect.getGlobalBounds();

		auto& callbacks = registry.get<ElementCallbacks>(entity);

		if (createInfo.onPressCallback.has_value())
		{
			callbacks.onPress = *createInfo.onPressCallback;
		}
		if (createInfo.onReleaseCallback.has_value())
		{
			callbacks.onRelease = *createInfo.onReleaseCallback;
		}

		data.children.emplace_back(createElement(registry, identifier + "_text", layer + 1, TextCreateInfo{createInfo.position, createInfo.font, createInfo.text, static_cast<uint32_t>(createInfo.size.y)}));
		auto& text = registry.get<sf::Text>(data.children.front());
		text.setFillColor(sf::Color::Black);
		while (text.getGlobalBounds().width > rect.getGlobalBounds().width * 0.9F || text.getGlobalBounds().height > rect.getGlobalBounds().height * 0.9F)
		{
			text.setCharacterSize(text.getCharacterSize() - 1);
		}

		text.setPosition(rect.getPosition() + sf::Vector2f((rect.getGlobalBounds().width - text.getGlobalBounds().width) / 2, (rect.getGlobalBounds().height - text.getGlobalBounds().height) / 2));

		return entity;
	}

	auto createElement(entt::registry& registry, std::string identifier, Layer layer, TextInputCreateInfo createInfo) -> entt::entity
	{
		auto entity     = createElement(registry, identifier, layer);
		auto& data      = registry.get<ElementData>(entity);
		auto& callbacks = registry.get<ElementCallbacks>(entity);

		// Unicode is compatible with ASCII
		const auto WIDEST_CHARACTER = 'M';
		auto widestCharacterWidth   = createInfo.font.getGlyph(WIDEST_CHARACTER, createInfo.textSize, false).bounds.width;

		auto& background = registry.emplace<sf::RectangleShape>(entity);
		background.setSize(sf::Vector2f(static_cast<float>(createInfo.maxLength) * widestCharacterWidth, static_cast<float>(createInfo.textSize)));
		background.setFillColor(sf::Color::White);
		background.setOutlineThickness(2.0F);
		background.setOutlineColor(sf::Color::Black);
		background.setPosition(createInfo.position);

		auto& inputData            = registry.emplace<TextInputData>(entity);
		inputData.maskingCharacter = createInfo.maskingCharacter;
		inputData.maxLength        = createInfo.maxLength;

		data.drawable = &background;
		data.collider = background.getGlobalBounds();

		data.children.emplace_back(createElement(
		    registry,
		    identifier + "_text",
		    layer + 1,
		    TextCreateInfo{
		        background.getPosition(),
		        createInfo.font,
		        "",
		        createInfo.textSize}));
		registry.get<sf::Text>(data.children.front()).setFillColor(sf::Color::Black);

		callbacks.onPress = [&](const sf::Mouse::Button button) {
			inputData.active = true;
			background.setOutlineColor(sf::Color::Red);
		};
		callbacks.onExit = [&]() {
			inputData.active = false;
			background.setOutlineColor(sf::Color::Black);
		};

		return entity;
	}

	auto destroyElement(entt::registry& registry, std::string identifier) -> void
	{
		auto hsIdentifier = entt::hashed_string(identifier.c_str(), identifier.size());
		for (const auto entity : registry.view<ElementData>())
		{
			auto& data = registry.get<ElementData>(entity);
			if (data.identifier == hsIdentifier)
			{
				for (const auto child : data.children)
				{
					registry.destroy(child);
				}
				registry.destroy(entity);
				return;
			}
		}
	}

	auto update(entt::registry& registry, const sf::Time deltaTime) -> void
	{
	}

	auto handleEvents(entt::registry& registry, sf::Event& event) -> bool
	{
		static auto mousePosition = sf::Vector2f();

		switch (event.type)
		{
			case sf::Event::MouseMoved:
			{
				mousePosition = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);
				for (const auto entity : registry.view<ElementData>())
				{
					auto& elementData      = registry.get<ElementData>(entity);
					auto& elementCallbacks = registry.get<ElementCallbacks>(entity);
					auto mouseOver         = elementData.collider.contains(mousePosition);

					if (mouseOver && !elementData.mouseOver)
					{
						elementData.mouseOver = true;
						elementCallbacks.onEnter();
					}
					else if (!mouseOver && elementData.mouseOver)
					{
						elementData.mouseOver = false;
						elementCallbacks.onExit();
					}
				}
			}
			break;
			case sf::Event::MouseButtonPressed:
			{
				for (const auto entity : registry.view<ElementData>())
				{
					auto& elementData      = registry.get<ElementData>(entity);
					auto& elementCallbacks = registry.get<ElementCallbacks>(entity);

					if (elementData.mouseOver)
					{
						elementCallbacks.onPress(event.mouseButton.button);
					}
				}
			}
			break;
			case sf::Event::MouseButtonReleased:
			{
				for (const auto entity : registry.view<ElementData>())
				{
					auto& elementData      = registry.get<ElementData>(entity);
					auto& elementCallbacks = registry.get<ElementCallbacks>(entity);

					if (elementData.mouseOver)
					{
						elementCallbacks.onRelease(event.mouseButton.button);
					}
				}
			}
			break;
			// Consume events that should be going to text input
			case sf::Event::KeyPressed:
			case sf::Event::KeyReleased:
			{
				for (const auto entity : registry.view<TextInputData>())
				{
					auto& textData = registry.get<TextInputData>(entity);
					if (textData.active)
					{
						return true;
					}
				}
			}
			break;
			case sf::Event::TextEntered:
			{
				for (const auto entity : registry.view<TextInputData>())
				{
					auto& elementData = registry.get<ElementData>(entity);
					auto& textData    = registry.get<TextInputData>(entity);
					if (!textData.active)
					{
						continue;
					}


					auto& data = registry.get<ElementData>(entity);
					auto& text = registry.get<sf::Text>(data.children.front());

					switch (event.text.unicode)
					{
						case 8: // Backspace
							if (!textData.input.empty())
							{
								textData.input.pop_back();
							}
							break;
						default:
							if (event.text.unicode < 32)
							{
								break;
							}
							if (event.text.unicode > 126)
							{
								break;
							}
							if (textData.input.size() == textData.maxLength)
							{
								break;
							}

							textData.input.push_back(static_cast<char>(event.text.unicode));
					}

					if (textData.maskingCharacter == TextInputCreateInfo::NO_MASKING)
					{
						text.setString(textData.input);
					}
					else
					{
						auto string = std::string(textData.input.size(), textData.maskingCharacter);
						text.setString(string);
					}
					return true;
				}
			}
			break;
			default:
				break;
		}

		return false;
	}

	auto UIRenderer::resize(sf::Vector2f newSize) -> void
	{
		m_uiView.setSize(newSize);
		m_uiView.setCenter(newSize * 0.5F);
	}

	auto UIRenderer::render(entt::registry& registry, sf::RenderTarget& renderTarget) -> void
	{
		renderTarget.setView(m_uiView);

		std::multimap<std::uint8_t, sf::Drawable*> elementsToDraw;
		for (const auto entity : registry.view<ElementData>())
		{
			auto& data = registry.get<ElementData>(entity);
			if (data.drawable != nullptr)
			{
				elementsToDraw.emplace(data.layer, data.drawable);
			}
		}

		for (const auto [layer, drawable] : elementsToDraw)
		{
			renderTarget.draw(*drawable);
		}
	}

} // namespace Client::UI