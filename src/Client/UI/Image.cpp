#include "UI/Image.hpp"
#include "UI/Element.hpp"
#include "UI/Layer.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <entt/entity/registry.hpp>

namespace Client::UI
{

	auto createImage(entt::registry& registry, entt::entity element, sf::Vector2f position, sf::Vector2f size, sf::Texture& texture) -> void
	{
		auto& sprite = registry.emplace<sf::Sprite>(element);
		sprite.setTexture(texture);
		sprite.setPosition(position);

		auto scaleX = sprite.getGlobalBounds().width / size.x;
		auto scaleY = sprite.getGlobalBounds().height / size.y;
		sprite.setScale(sf::Vector2f(scaleX, scaleY));
	}

	auto createImage(entt::registry& registry, std::string identifier, Layer layer, sf::Vector2f position, sf::Vector2f size, sf::Texture& texture) -> void
	{
		auto entity = createElement(registry, identifier, layer);

		auto& sprite = registry.emplace<sf::Sprite>(entity);
		sprite.setTexture(texture);
		sprite.setPosition(position);

		auto scaleX = sprite.getGlobalBounds().width / size.x;
		auto scaleY = sprite.getGlobalBounds().height / size.y;
		sprite.setScale(sf::Vector2f(scaleX, scaleY));
	}

} // namespace Client::UI