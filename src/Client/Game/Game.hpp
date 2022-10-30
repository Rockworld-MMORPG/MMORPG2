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

	/**
	 * \class Game Game.hpp "Game/Game.hpp"
	 * \brief State implementing the main gameplay cycle
	 */
	class Game final : public State
	{
	public:
		/**
		 * \brief Construct a new Game state object
		 *
		 * \param engine A reference to the engine the state is contained in
		 */
		Game(Engine& engine);

		/**
		 * \brief Destroy the Game state object
		 *
		 */
		~Game() override;

		/**
		 * \brief Called once per frame to handle any messages the client has received from the server
		 *
		 * \param messages A vector containing all the messages the client has received
		 */
		auto parseMessages(std::vector<Common::Network::Message>& messages) -> void override;

		/**
		 * \brief Called whenever there are events polled from the OS
		 *
		 * \param event The event that has been polled
		 */
		auto handleEvents(sf::Event& event) -> void override;

		/**
		 * \brief Called once per frame to update any game logic
		 *
		 * \param deltaTime How long the previous frame took to update and render
		 */
		auto update(sf::Time deltaTime) -> void override;

		/**
		 * \brief Called once per frame to render any drawables to the render target
		 *
		 * \param renderTarget The target to render to
		 */
		auto render(sf::RenderTarget& renderTarget) -> void override;

	private:
		/**
		 * \brief Parses a message received over TCP
		 *
		 * \param message The message to parse
		 */
		auto parseTCP(Common::Network::Message& message) -> void;

		/**
		 * \brief Parses a message received over UDP
		 *
		 * \param message The message to parse
		 */
		auto parseUDP(Common::Network::Message& message) -> void;

		/**
		 * \brief Loads a tile into the texture atlas
		 *
		 * \param data A vector of bytes representing the tile data
		 */
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