#include "Engine/Engine.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "spdlog/spdlog.h"


namespace Client
{

	Engine::Engine(std::filesystem::path assetDir) :
	    assetDirectory(std::move(assetDir))
	{
		m_window.create(sf::VideoMode(sf::Vector2u(1280U, 720U), 32U), "Window");
		m_window.setVerticalSyncEnabled(true);
		m_clock.restart();
	}

	Engine::~Engine()
	{
		m_window.close();
	}

	auto Engine::pushState(std::unique_ptr<State>&& state) -> void
	{
		m_stateStack.emplace(std::forward<std::unique_ptr<State>>(state));
	}

	auto Engine::popState() -> void
	{
		if (!m_stateStack.empty())
		{
			m_stateStack.pop();
		}
	}

	auto Engine::getState() -> State&
	{
		return *m_stateStack.top();
	}

	auto Engine::run() -> void
	{
		if (m_stateStack.empty())
		{
			spdlog::error("Tried to run the engine but it has no state");
		}

		while (!m_shouldExit)
		{
			auto deltaTime = m_clock.restart().asSeconds();

			networkManager.update();
			auto inboundMessages = networkManager.getMessages();
			getState().parseMessages(inboundMessages);

			auto event = sf::Event();
			while (m_window.pollEvent(event))
			{
				getState().handleEvents(event);
			}

			getState().update(deltaTime);

			m_window.clear();
			getState().render(m_window);
			m_window.display();
		}
	}

	auto Engine::setShouldExit(bool shouldExit) -> void
	{
		m_shouldExit = shouldExit;
	}

} // namespace Client