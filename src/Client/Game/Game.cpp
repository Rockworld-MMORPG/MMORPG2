#include "Game/Game.hpp"
#include "Engine/Engine.hpp"
#include "Network/NetworkManager.hpp"
#include <Common/Input/Action.hpp>
#include <Common/Input/ActionType.hpp>
#include <Common/Input/InputState.hpp>
#include <Common/Network/MessageData.hpp>
#include <Common/Network/MessageType.hpp>
#include <Common/Network/Protocol.hpp>
#include <Common/World/Level.hpp>
#include <Common/World/Tile.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdint>
#include <fstream>
#include <spdlog/spdlog.h>

namespace Client::Game
{

	Game::Game(Engine& engine) :
	    State(engine)
	{
		engine.assetManager.loadAsset("test_level.dat", "test_level");
		engine.assetManager.loadAsset("player.png", "player");

		m_camera.setCenter(sf::Vector2f(0.0F, 0.0F));
		m_camera.setSize(static_cast<sf::Vector2f>(engine.window.getSize()));
		engine.window.setView(m_camera);

		const auto& player = engine.assetManager.getAsset("player");
		auto success       = m_playerTexture.loadFromMemory(player.data(), player.size());
		engine.networkManager.connect();

		engine.inputManager.bindAction(sf::Keyboard::W, Common::Input::ActionType::MoveForward);
		engine.inputManager.bindAction(sf::Keyboard::A, Common::Input::ActionType::StrafeLeft);
		engine.inputManager.bindAction(sf::Keyboard::S, Common::Input::ActionType::MoveBackward);
		engine.inputManager.bindAction(sf::Keyboard::D, Common::Input::ActionType::StrafeRight);

		const auto& testLevel = engine.assetManager.getAsset("test_level");

		for (auto yPos = 0; yPos < Common::World::LEVEL_HEIGHT; ++yPos)
		{
			for (auto xPos = 0; xPos < Common::World::LEVEL_WIDTH; ++xPos)
			{
				auto tile    = Common::World::Tile();
				tile.type[0] = static_cast<std::uint8_t>(testLevel.at((xPos + yPos * Common::World::LEVEL_WIDTH) * 4 + 0));
				tile.type[1] = static_cast<std::uint8_t>(testLevel.at((xPos + yPos * Common::World::LEVEL_WIDTH) * 4 + 1));
				tile.type[2] = static_cast<std::uint8_t>(testLevel.at((xPos + yPos * Common::World::LEVEL_WIDTH) * 4 + 2));
				tile.travelMode
				    = static_cast<Common::World::Tile::TravelMode>(testLevel.at((xPos + yPos * Common::World::LEVEL_WIDTH) * 4 + 3));

				m_level.setTile(xPos, yPos, tile);
			}
		}

		m_terrainRenderer.addLevel(m_level);
	}

	Game::~Game()
	{
		engine.networkManager.disconnect();
		engine.window.setView(engine.window.getDefaultView());
	}

	auto Game::parseTCP(Common::Network::Message& message) -> void
	{
		switch (message.header.type)
		{
			case Common::Network::MessageType::Disconnect:
				engine.networkManager.disconnect();
				break;
			default:
				break;
		}
	}

	auto Game::parseUDP(Common::Network::Message& message) -> void
	{
		switch (message.header.type)
		{
			case Common::Network::MessageType::CreateEntity:
			{
				auto entity  = m_registry.create();
				auto& sprite = m_registry.emplace<sf::Sprite>(entity);
				sprite.setTexture(m_playerTexture);

				auto& serverID = m_registry.emplace<entt::entity>(entity);
				message.data >> serverID;

				auto& inputState = m_registry.emplace<Common::Input::InputState>(entity);
			}
			break;
			case Common::Network::MessageType::InputState:
			{
				auto serverEntity = entt::entity();
				message.data >> serverEntity;
				for (const auto entity : m_registry.view<Common::Input::InputState, entt::entity>())
				{
					if (m_registry.get<entt::entity>(entity) != serverEntity)
					{
						continue;
					}
					auto& inputState = m_registry.get<Common::Input::InputState>(entity);
					message.data >> inputState.forwards >> inputState.backwards >> inputState.left >> inputState.right;
				}
			}
			break;
			case Common::Network::MessageType::DestroyEntity:
			{
				auto serverEntity = entt::entity();
				auto localEntity  = entt::entity(entt::null);
				message.data >> serverEntity;
				for (const auto entity : m_registry.view<entt::entity>())
				{
					if (m_registry.get<entt::entity>(entity) == serverEntity)
					{
						localEntity = entity;
						break;
					}
				}
				if (localEntity != entt::null)
				{
					m_registry.destroy(localEntity);
				}
			}
			break;
		}
	}

	auto Game::parseMessages(std::vector<Common::Network::Message>& messages) -> void
	{
		for (auto& message : messages)
		{
			switch (message.header.protocol)
			{
				case Common::Network::Protocol::TCP:
					parseTCP(message);
					break;
				case Common::Network::Protocol::UDP:
					parseUDP(message);
					break;
			}
		}
	}

	auto sendAction(const Common::Input::ActionType actionType, const Common::Input::Action::State actionState, NetworkManager& networkManager) -> void
	{
		auto action  = Common::Input::Action();
		action.type  = actionType;
		action.state = actionState;

		auto data = Common::Network::MessageData();
		data << action;

		networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Action, data);
	}

	auto Game::handleEvents(sf::Event& event) -> void
	{
		switch (event.type)
		{
			case sf::Event::Closed:
				engine.setShouldExit(true);
				break;
			case sf::Event::KeyReleased:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Key::P:
					{
						auto data = Common::Network::MessageData();
						engine.networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Terminate, data);
					}
					break;
					case sf::Keyboard::Key::C:
					{
						auto data = Common::Network::MessageData();
						engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
					}
					break;
					default:
						break;
				}
			}
			break;
			default:
				break;
		}
	}

	auto Game::update(const sf::Time deltaTime) -> void
	{
		auto changedStates = engine.inputManager.getChangedStates();
		for (const auto actionType : changedStates)
		{
			auto pressed = engine.inputManager.getState(actionType).isPressed;
			auto action  = Common::Input::Action();
			action.state = pressed ? Common::Input::Action::State::Begin : Common::Input::Action::State::End;
			action.type  = actionType;

			auto data = Common::Network::MessageData();
			data << action;
			engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Action, data);
		}

		for (const auto entity : m_registry.view<sf::Sprite, Common::Input::InputState>())
		{
			auto& input    = m_registry.get<Common::Input::InputState>(entity);
			auto& sprite   = m_registry.get<sf::Sprite>(entity);
			auto& serverID = m_registry.get<entt::entity>(entity);

			if (serverID == engine.networkManager.getClientID())
			{
				m_camera.setCenter(sprite.getPosition() + sprite.getGlobalBounds().getSize() * 0.5F);
			}

			auto deltaPosition = sf::Vector2f(0.0F, 0.0F);
			if (input.forwards)
			{
				deltaPosition.y -= 200.0F;
			}
			if (input.backwards)
			{
				deltaPosition.y += 200.0F;
			}
			if (input.left)
			{
				deltaPosition.x -= 200.0F;
			}
			if (input.right)
			{
				deltaPosition.x += 200.0F;
			}

			deltaPosition *= deltaTime.asSeconds();
			sprite.move(deltaPosition);
		}
	}

	auto Game::render(sf::RenderTarget& renderTarget) -> void
	{
		renderTarget.setView(m_camera);
		m_terrainRenderer.render(renderTarget);

		for (const auto entity : m_registry.view<sf::Sprite>())
		{
			renderTarget.draw(m_registry.get<sf::Sprite>(entity));
		}
	}

} // namespace Client::Game