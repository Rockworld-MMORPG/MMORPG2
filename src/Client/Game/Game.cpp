#include "Game/Game.hpp"
#include "Engine/Engine.hpp"
#include "Network/NetworkManager.hpp"
#include <Common/Input/Action.hpp>
#include <Common/Input/ActionType.hpp>
#include <Common/Input/InputState.hpp>
#include <Common/Network/MessageData.hpp>
#include <Common/Network/MessageType.hpp>
#include <Common/Network/NetworkEntity.hpp>
#include <Common/Network/Protocol.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
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
				m_level.setTile(xPos, yPos, testLevel.at(xPos + yPos * Common::World::LEVEL_WIDTH));
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
			case Common::Network::MessageType::InputState:
			{
				auto size = std::uint32_t(0);
				message.data >> size;

				for (auto i = 0; i < size; ++i)
				{
					auto entity     = Common::Network::ClientID_t(-1);
					auto inputState = Common::Input::InputState();
					message.data >> entity >> inputState;

					auto ent = static_cast<Common::Network::NetworkEntity>(entity);
					if (!m_registry.valid(ent))
					{
						auto data = Common::Network::MessageData();
						data << entity;
						engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::GetEntity, data);
					}
					else
					{
						auto& input = m_registry.emplace_or_replace<Common::Input::InputState>(ent, inputState);
					}
				}
			}
			break;
			case Common::Network::MessageType::CreateEntity:
			{
				auto entity = Common::Network::ClientID_t(-1);
				sf::Vector2f position(0.0F, 0.0F);
				message.data >> entity >> position.x >> position.y;
				if (entity == -1)
				{
					// This probably wasn't supposed to happen
					break;
				}

				auto ent = static_cast<Common::Network::NetworkEntity>(entity);
				if (!m_registry.valid(ent))
				{
					auto newEntity = m_registry.create(ent);
					if (newEntity != ent)
					{
						m_registry.destroy(newEntity);
						spdlog::warn("Failed to create entity {}", entity);
						break;
					}

					auto& sprite = m_registry.emplace<sf::Sprite>(newEntity);
					sprite.setTexture(m_playerTexture);
					sprite.setPosition(position);
					m_registry.emplace<Common::Input::InputState>(newEntity);
				}
			}
			break;
			case Common::Network::MessageType::DestroyEntity:
			{
				Common::Network::ClientID_t entity = -1;
				message.data >> entity;
				auto ent = static_cast<Common::Network::NetworkEntity>(entity);
				if (m_registry.valid(ent))
				{
					m_registry.destroy(ent);
				}
			}
			break;
			default:
				// Do nothing
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
			auto& input  = m_registry.get<Common::Input::InputState>(entity);
			auto& sprite = m_registry.get<sf::Sprite>(entity);

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

			if (static_cast<Common::Network::ClientID_t>(entity) == engine.networkManager.getClientID().get())
			{
				m_camera.setCenter(sprite.getPosition());
				engine.window.setView(m_camera);
			}
		}
	}

	auto Game::render(sf::RenderTarget& renderTarget) -> void
	{
		m_terrainRenderer.render(renderTarget);

		for (const auto entity : m_registry.view<sf::Sprite>())
		{
			renderTarget.draw(m_registry.get<sf::Sprite>(entity));
		}
	}

} // namespace Client::Game