#pragma once

#include "TerrainTile.hpp"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cstdint>
#include <unordered_map>

class TextureManager
{
public:
	TextureManager()
	{
		m_textureAtlas.create(sf::Vector2u(TILE_SCALE * ATLAS_SIZE_X, TILE_SCALE * ATLAS_SIZE_Y));
	}

	auto addTexture(const std::uint32_t identifier, const sf::Image& image) -> void
	{
		auto index  = m_lastIndex++;
		auto indexX = index % ATLAS_SIZE_X;
		auto indexY = index / ATLAS_SIZE_X;

		auto iCoordX = indexX * static_cast<std::int32_t>(TILE_SCALE);
		auto iCoordY = indexY * static_cast<std::int32_t>(TILE_SCALE);
		auto iWidth  = static_cast<std::int32_t>(TILE_SCALE);
		auto iHeight = static_cast<std::int32_t>(TILE_SCALE);

		auto fCoordX = static_cast<float>(indexX) / static_cast<float>(ATLAS_SIZE_X);
		auto fCoordY = static_cast<float>(indexY) / static_cast<float>(ATLAS_SIZE_Y);
		auto fWidth  = TILE_SCALE / static_cast<float>(m_textureAtlas.getSize().x);
		auto fHeight = TILE_SCALE / static_cast<float>(m_textureAtlas.getSize().y);

		m_textureAtlas.update(image.getPixelsPtr(), image.getSize(), sf::Vector2u(indexX * TILE_SCALE, indexY * TILE_SCALE));
		m_textureCoords.emplace(identifier, sf::FloatRect(sf::Vector2f(fCoordX, fCoordY), sf::Vector2f(fWidth, fHeight)));
		m_textureRects.emplace(identifier, sf::IntRect(sf::Vector2i(iCoordX, iCoordY), sf::Vector2i(iWidth, iHeight)));
	}

	auto bind() -> void
	{
	}

	auto getAtlas() -> sf::Texture&
	{
		return m_textureAtlas;
	}

	auto getTextureCoords(std::uint32_t identifier) const -> sf::FloatRect
	{
		if (!m_textureCoords.contains(identifier))
		{
			return {};
		}

		return m_textureCoords.at(identifier);
	}

	auto getTextureRect(std::uint32_t identifier) const -> sf::IntRect
	{
		if (!m_textureRects.contains(identifier))
		{
			return {};
		}

		return m_textureRects.at(identifier);
	}

private:
	const std::size_t ATLAS_SIZE_X = 32;
	const std::size_t ATLAS_SIZE_Y = 32;

	std::size_t m_lastIndex = 0;
	std::unordered_map<std::uint32_t, sf::FloatRect> m_textureCoords;
	std::unordered_map<std::uint32_t, sf::IntRect> m_textureRects;
	sf::Texture m_textureAtlas;
};
