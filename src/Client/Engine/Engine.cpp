#include "Engine/Engine.hpp"
#include "Discord/DiscordManager.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "spdlog/spdlog.h"
#include "types.h"

namespace Client
{
	const auto WINDOW_WIDTH  = 1280U;
	const auto WINDOW_HEIGHT = 720U;

	Engine::Engine(std::filesystem::path assetDir) :
	    assetDirectory(std::move(assetDir))
	{
		DiscordManager::get().setStatus("Exploring the valley of many rocks");

		m_window.create(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Window");
		m_window.setVerticalSyncEnabled(true);
		m_window.setKeyRepeatEnabled(false);
		m_clock.restart();
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
			auto deltaTime = m_clock.restart();

			DiscordManager::get().update();

			networkManager.update();
			auto inboundMessages = networkManager.getMessages();
			getState().parseMessages(inboundMessages);

			auto event = sf::Event();
			while (m_window.pollEvent(event))
			{
				inputManager.parseEvents(event);
				getState().handleEvents(event);
			}

			inputManager.update(deltaTime);
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