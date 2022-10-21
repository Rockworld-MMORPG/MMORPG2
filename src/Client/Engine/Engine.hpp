#pragma once

#include "Discord/DiscordManager.hpp"
#include "Engine/State.hpp"
#include "Input/InputManager.hpp"
#include "Network/NetworkManager.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include <filesystem>
#include <memory>
#include <stack>

namespace Client
{

	class Engine
	{
	public:
		Engine(std::filesystem::path assetDir);

		auto pushState(std::unique_ptr<State>&& state) -> void;
		auto popState() -> void;
		auto getState() -> State&;

		auto run() -> void;
		auto setShouldExit(bool shouldExit) -> void;

		NetworkManager networkManager;
		InputManager inputManager;
		std::filesystem::path assetDirectory;

	private:
		std::stack<std::unique_ptr<State>> m_stateStack;
		bool m_shouldExit = false;

		sf::RenderWindow m_window;
		sf::Clock m_clock;
	};

} // namespace Client