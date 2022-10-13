#pragma once

#include "Engine/State.hpp"
#include "World/Level.hpp"
#include "World/TerrainRenderer.hpp"
#include <Common/Network/ClientID.hpp>
#include <Common/Network/Message.hpp>
#include <Common/Network/NetworkEntity.hpp>
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

		World::Level m_level;
		World::TerrainRenderer m_terrainRenderer;

		sf::View m_camera;

		entt::basic_registry<Common::Network::NetworkEntity> m_registry;
		sf::Texture m_playerTexture;
	};

} // namespace Client::Game