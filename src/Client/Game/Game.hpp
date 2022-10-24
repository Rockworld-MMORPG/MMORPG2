#pragma once

#include "Engine/State.hpp"
#include "UI/UI.hpp"
#include "World/TerrainRenderer.hpp"
#include <Common/Network/Message.hpp>
#include <Common/World/Level.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <entt/entity/registry.hpp>

namespace Client::Game
{

	class Game final : public State
	{
	public:
		Game(Engine& engine);
		~Game() override;

		auto parseMessages(std::vector<Common::Network::Message>& messages) -> void override;
		auto handleEvents(sf::Event& event) -> void override;
		auto update(sf::Time deltaTime) -> void override;
		auto render(sf::RenderTarget& renderTarget) -> void override;

	private:
		auto parseTCP(Common::Network::Message& message) -> void;
		auto parseUDP(Common::Network::Message& message) -> void;

		auto loadTile(const std::vector<char>& data) -> void;

		Common::World::Level m_level;
		World::TerrainRenderer m_terrainRenderer;
		Graphics::TextureAtlas m_textureAtlas;

		UI::UIRenderer m_uiRenderer;

		sf::View m_camera;

		entt::registry m_registry;
		sf::Texture m_playerTexture;
		sf::Font m_font;
	};

} // namespace Client::Game