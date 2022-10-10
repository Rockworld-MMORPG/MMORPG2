#include "Game/Game.hpp"
#include "Common/Input/InputState.hpp"
#include "Engine/Engine.hpp"
#include "Network/NetworkManager.hpp"
#include <Common/Input/Action.hpp>
#include <Common/Input/ActionType.hpp>
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
#include <spdlog/spdlog.h>

namespace Client::Game
{

	Game::Game(Engine& engine) :
	    State(engine)
	{
		m_playerTexture.loadFromFile(engine.assetDirectory / "player.png");
		engine.networkManager.connect();
	}

	Game::~Game() = default;

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
					if (m_registry.all_of<Common::Input::InputState>(ent))
					{
						auto& input = m_registry.get<Common::Input::InputState>(ent) = inputState;
					}
				}
			}
			break;
			case Common::Network::MessageType::CreateEntity:
			{
				Common::Network::ClientID_t entity = -1;
				message.data >> entity;
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
			case sf::Event::KeyPressed:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Key::W:
						sendAction(Common::Input::ActionType::MoveForward, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					case sf::Keyboard::Key::A:
						sendAction(Common::Input::ActionType::StrafeLeft, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					case sf::Keyboard::Key::S:
						sendAction(Common::Input::ActionType::MoveBackward, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					case sf::Keyboard::Key::D:
						sendAction(Common::Input::ActionType::StrafeRight, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					default:
						break;
				}
			}
			break;
			case sf::Event::KeyReleased:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Key::W:
						sendAction(Common::Input::ActionType::MoveForward, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::A:
						sendAction(Common::Input::ActionType::StrafeLeft, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::S:
						sendAction(Common::Input::ActionType::MoveBackward, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::D:
						sendAction(Common::Input::ActionType::StrafeRight, Common::Input::Action::State::End, engine.networkManager);
						break;
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
		auto fDt = deltaTime.asSeconds();

		for (const auto entity : m_registry.view<sf::Sprite, Common::Input::InputState>())
		{
			auto& input  = m_registry.get<Common::Input::InputState>(entity);
			auto& sprite = m_registry.get<sf::Sprite>(entity);

			auto delta = sf::Vector2f(0.0F, 0.0F);
			if (input.forwards)
			{
				delta.y -= 200.0F;
			}
			if (input.backwards)
			{
				delta.y += 200.0F;
			}
			if (input.left)
			{
				delta.x -= 200.0F;
			}
			if (input.right)
			{
				delta.x += 200.0F;
			}

			delta *= fDt;
			sprite.move(delta);
		}
	}

	auto Game::render(sf::RenderTarget& renderTarget) -> void
	{
		for (const auto entity : m_registry.view<sf::Sprite>())
		{
			renderTarget.draw(m_registry.get<sf::Sprite>(entity));
		}
	}

} // namespace Client::Game