#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cstdint>
#include <unordered_map>

namespace Client::Graphics
{

	class TextureAtlas
	{
	public:
		TextureAtlas(sf::Vector2u textureSize, sf::Vector2u atlasSize);

		auto addTexture(std::uint32_t identifier, sf::Image& image) -> void;

		auto getTextureSize() const -> sf::Vector2u;
		auto getAtlasSize() const -> sf::Vector2u;
		auto getTextureCoordinates(std::uint32_t identifier) const -> sf::FloatRect;
		auto getTexture() -> sf::Texture&;

	private:
		std::size_t m_lastTextureIndex;
		std::size_t m_maxTextureIndex;
		sf::Vector2u m_textureSize;
		sf::Vector2u m_atlasSize;
		std::unordered_map<std::uint32_t, sf::FloatRect> m_textureHandles;
		sf::Texture m_texture;
	};

} // namespace Client::Graphics