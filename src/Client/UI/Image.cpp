#include "UI/Image.hpp"
#include "UI/Layer.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <entt/entity/registry.hpp>

namespace Client::UI
{

	auto createImage(entt::registry& registry, sf::Vector2f position, sf::Vector2f size, sf::Texture& texture) -> void
	{
		auto entity = registry.create();

		auto& sprite = registry.emplace<sf::Sprite>(entity);
		sprite.setTexture(texture);
		sprite.setPosition(position);

		auto scaleX = sprite.getGlobalBounds().width / size.x;
		auto scaleY = sprite.getGlobalBounds().height / size.y;
		sprite.setScale(sf::Vector2f(scaleX, scaleY));

		auto& layer = registry.emplace<Layer>(entity);
		layer.index = 0;
	}

} // namespace Client::UI