#include "Engine/Engine.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "spdlog/spdlog.h"
#include "types.h"

namespace Client
{

	const auto CLIENT_ID      = discord::ClientId(1029180832537116672);
	const auto APPLICATION_ID = std::int64_t(1029180832537116672);

	const auto WINDOW_WIDTH  = 1280U;
	const auto WINDOW_HEIGHT = 720U;

	Engine::Engine(std::filesystem::path assetDir) :
	    assetDirectory(std::move(assetDir))
	{
		spdlog::debug("Creating core");
		discord::Core* core = nullptr;
		auto result         = discord::Core::Create(CLIENT_ID, static_cast<std::uint64_t>(discord::CreateFlags::NoRequireDiscord), &core);

		if (result != discord::Result::Ok)
		{
			spdlog::debug("Failed to create Discord core (error code {})", static_cast<std::uint64_t>(result));
		}
		else
		{
			m_discord.reset(core);

			m_discord->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel logLevel, const char* message) {
				spdlog::debug("Discord - {}", message);
			});

			spdlog::debug("Creating activity");
			auto mmorpgActivity = discord::Activity();
			mmorpgActivity.SetApplicationId(APPLICATION_ID);
			mmorpgActivity.SetName("Wow of Wowiewowow");
			mmorpgActivity.SetState("Alone :(");
			mmorpgActivity.SetDetails("Probably something really cool");
			mmorpgActivity.SetInstance(false);
			mmorpgActivity.SetType(discord::ActivityType::Playing);
			spdlog::debug("Updating activity");
			m_discord->ActivityManager().UpdateActivity(mmorpgActivity, [](discord::Result result) {
				spdlog::debug("Callback with result {}", static_cast<std::uint32_t>(result));
			});
		}

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

			if (m_discord)
			{
				m_discord->RunCallbacks();
			}

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