#include "Discord/DiscordManager.hpp"
#include "Discord/Secrets.hpp"
#include <cstdint>
#include <discord.h>
#include <spdlog/spdlog.h>

namespace Client
{

	auto DiscordManager::get() -> DiscordManager&
	{
		static DiscordManager manager;
		return manager;
	}

	auto DiscordManager::update() -> void
	{
		if (m_core)
		{
			m_core->RunCallbacks();
		}
	}

	auto getDefaultActivity(DiscordManager::Assets& assets) -> discord::Activity
	{
		auto activity = discord::Activity();
		activity.SetType(discord::ActivityType::Playing);
		activity.SetSupportedPlatforms(static_cast<std::uint32_t>(discord::ActivitySupportedPlatformFlags::Desktop));
		activity.SetInstance(false);

		activity.GetAssets().SetLargeImage(assets.largeImage.c_str());
		activity.GetAssets().SetLargeText(assets.largeText.c_str());
		activity.GetAssets().SetSmallImage(assets.smallImage.c_str());
		activity.GetAssets().SetSmallText(assets.smallText.c_str());

		return activity;
	}

	auto DiscordManager::setStatus(const std::string& playerActivity) -> void
	{
		if (!m_core)
		{
			spdlog::warn("Tried to set Discord status but we have no core");
			return;
		}

		auto activity = getDefaultActivity(m_assets);
		activity.SetDetails(playerActivity.c_str());

		m_core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
			if (result != discord::Result::Ok)
			{
				spdlog::warn("Failed to update Discord status (error code {})", static_cast<std::uint32_t>(result));
			}
		});
	}

	auto DiscordManager::setStatus(const std::string& playerActivity, const std::string& partyActivity, std::int32_t partySize, std::int32_t maxPartySize) -> void
	{
		if (!m_core)
		{
			spdlog::warn("Tried to set Discord status but we have no core");
			return;
		}

		auto activity = getDefaultActivity(m_assets);
		activity.SetDetails(playerActivity.c_str());
		activity.SetState(partyActivity.c_str());
		activity.GetParty().GetSize().SetCurrentSize(partySize);
		activity.GetParty().GetSize().SetMaxSize(maxPartySize);

		m_core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
			if (result != discord::Result::Ok)
			{
				spdlog::warn("Failed to update Discord status (error code {})", static_cast<std::uint32_t>(result));
			}
		});
	}

	auto DiscordManager::setAssets(std::string largeImage, std::string largeText, std::string smallImage, std::string smallText) -> void
	{
		m_assets.largeImage = std::move(largeImage);
		m_assets.largeText  = std::move(largeText);
		m_assets.smallImage = std::move(smallImage);
		m_assets.smallText  = std::move(smallText);
	}

	DiscordManager::DiscordManager()
	{
		discord::Core* core = nullptr;
		auto errorCode      = discord::Core::Create(Discord::getClientID(), static_cast<std::uint64_t>(discord::CreateFlags::NoRequireDiscord), &core);
		if (errorCode != discord::Result::Ok)
		{
			spdlog::warn("Failed to create a Discord core (error code {})", static_cast<std::uint32_t>(errorCode));
			return;
		}

		spdlog::debug("Successfully created Discord core");

		m_core.reset(core);
		m_core->SetLogHook(discord::LogLevel::Info, [](discord::LogLevel logLevel, const char* str) {
			spdlog::info("Discord - {}", str);
		});
		m_core->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel logLevel, const char* str) {
			spdlog::debug("Discord - {}", str);
		});
		m_core->SetLogHook(discord::LogLevel::Warn, [](discord::LogLevel logLevel, const char* str) {
			spdlog::warn("Discord - {}", str);
		});
		m_core->SetLogHook(discord::LogLevel::Error, [](discord::LogLevel logLevel, const char* str) {
			spdlog::error("Discord - {}", str);
		});
	}

} // namespace Client