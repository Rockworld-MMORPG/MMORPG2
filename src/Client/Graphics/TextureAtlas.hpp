#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cstdint>
#include <unordered_map>

namespace Client::Graphics
{

	/**
	 * \class TextureAtlas TextureAtlas.hpp "Graphics/TextureAtlas.hpp"
	 * \brief Stores a number of textures, of the same dimensions, in a single larger texture
	 */
	class TextureAtlas
	{
	public:
		/**
		 * \brief Construct a new Texture Atlas object
		 *
		 * \param textureSize The size of the textures the atlas should store
		 * \param atlasSize The number of textures the atlas should store in the x and y directions
		 */
		TextureAtlas(sf::Vector2u textureSize, sf::Vector2u atlasSize);

		/**
		 * \brief Adds a texture to the texture atlas
		 *
		 * \param identifier The identifier to assign to the texture
		 * \param image The image data to store in the atlas
		 */
		auto addTexture(std::uint32_t identifier, sf::Image& image) -> void;

		/**
		 * \brief Get the size of a texture in the atlas
		 */
		auto getTextureSize() const -> sf::Vector2u;

		/**
		 * \brief Get the number of textures in the atlas in the x and y directions
		 */
		auto getAtlasSize() const -> sf::Vector2u;

		/**
		 * \brief Get the coordinates of a texture in the atlas
		 *
		 * \param identifier The identifier of the texture in the atlas
		 * \return sf::FloatRect A FloatRect representing the coordinates of the texture in the atlas, or one containing all zeroes if it could not be found
		 */
		auto getTextureCoordinates(std::uint32_t identifier) const -> sf::FloatRect;

		/**
		 * \brief Get the atlas' texture
		 */
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