#include "Graphics/TextureAtlas.hpp"

namespace Client::Graphics
{
	TextureAtlas::TextureAtlas(const sf::Vector2u textureSize, const sf::Vector2u atlasSize) :
	    m_lastTextureIndex(0),
	    m_maxTextureIndex(static_cast<std::size_t>(atlasSize.x * atlasSize.y)),
	    m_textureSize(textureSize),
	    m_atlasSize(atlasSize)
	{
		auto atlasDimensions = textureSize.cwiseMul(atlasSize);
		auto result          = m_texture.create(atlasDimensions);
		if (!result)
		{
			spdlog::warn("Failed to create texture atlas of size {}x{}", atlasDimensions.x, atlasDimensions.y);
		}
	}

	auto TextureAtlas::addTexture(std::uint32_t identifier, sf::Image& image) -> void
	{
		if (m_textureHandles.contains(identifier))
		{
			spdlog::debug("Texture {} already exists in atlas, skipping", identifier);
			return;
		}

		auto index = m_lastTextureIndex++;
		if (index >= m_maxTextureIndex)
		{
			spdlog::warn("Tried to insert texture {} into an atlas that is full", identifier);
			return;
		}

		if (image.getSize() != m_textureSize)
		{
			spdlog::warn("Tried to insert a texture but it is not the right size");
			return;
		}

		auto indexX = index % m_atlasSize.x;
		auto indexY = index / m_atlasSize.y;

		auto destination = sf::Vector2u(indexX * m_textureSize.x, indexY * m_textureSize.y);
		m_texture.update(image, destination);
		m_textureHandles.emplace(identifier, sf::FloatRect(static_cast<sf::Vector2f>(destination), static_cast<sf::Vector2f>(m_textureSize)));
	}

	auto TextureAtlas::getTextureSize() const -> sf::Vector2u
	{
		return m_textureSize;
	}

	auto TextureAtlas::getAtlasSize() const -> sf::Vector2u
	{
		return m_atlasSize;
	}

	auto TextureAtlas::getTextureCoordinates(std::uint32_t identifier) const -> sf::FloatRect
	{
		if (!m_textureHandles.contains(identifier))
		{
			spdlog::debug("Tried to get texture {} but it does not exist in this atlas", identifier);
			return {};
		}

		return m_textureHandles.at(identifier);
	}

	auto TextureAtlas::getTexture() -> sf::Texture&
	{
		return m_texture;
	}


} // namespace Client::Graphics