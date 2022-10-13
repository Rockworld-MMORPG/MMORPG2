#pragma once

#include "Assets/AssetManager.hpp"
#include "Engine/State.hpp"
#include "Input/InputManager.hpp"
#include "Network/NetworkManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <stack>

namespace Client
{

	class Engine
	{
	public:
		Engine(const std::filesystem::path& executableDir);

		auto pushState(std::unique_ptr<State>&& state) -> void;
		auto popState() -> void;
		auto getState() -> State&;

		auto run() -> void;
		auto setShouldExit(bool shouldExit) -> void;

		AssetManager assetManager;
		NetworkManager networkManager;
		InputManager inputManager;

	private:
		std::stack<std::unique_ptr<State>> m_stateStack;
		bool m_shouldExit = false;

		sf::RenderWindow m_window;
		sf::Clock m_clock;
	};

} // namespace Client