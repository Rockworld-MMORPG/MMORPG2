#include "Engine/Engine.hpp"
#include "Engine/State.hpp"
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <spdlog/spdlog.h>


namespace Client
{

	const auto WINDOW_WIDTH  = 1280U;
	const auto WINDOW_HEIGHT = 720U;

	Engine::Engine(const std::filesystem::path& executableDir) :
	    assetManager(executableDir)
	{
		window.create(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Window");
		window.setVerticalSyncEnabled(true);
		window.setKeyRepeatEnabled(false);
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

			networkManager.update();
			auto inboundMessages = networkManager.getMessages();
			getState().parseMessages(inboundMessages);

			auto event = sf::Event();
			while (window.pollEvent(event))
			{
				getState().handleEvents(event);
			}

			inputManager.update(deltaTime);
			getState().update(deltaTime);

			window.clear();
			getState().render(window);
			window.display();
		}
	}

	auto Engine::setShouldExit(bool shouldExit) -> void
	{
		m_shouldExit = shouldExit;
	}

} // namespace Client