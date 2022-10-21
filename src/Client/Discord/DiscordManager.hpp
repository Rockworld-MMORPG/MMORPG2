#pragma once

#include <memory>

namespace discord
{
	class Core;
}

namespace Client
{

	class DiscordManager
	{
	public:
		static auto get() -> DiscordManager&;

		auto update() -> void;
		auto setStatus(const std::string& playerActivity) -> void;
		auto setStatus(const std::string& playerActivity, const std::string& partyActivity, std::int32_t partySize, std::int32_t maxPartySize) -> void;

		auto setAssets(std::string largeImage, std::string largeText, std::string smallImage, std::string smallText) -> void;

		struct Assets
		{
			std::string largeImage;
			std::string largeText;
			std::string smallImage;
			std::string smallText;
		};

	private:
		DiscordManager();

		std::unique_ptr<discord::Core> m_core;
		Assets m_assets;
	};

} // namespace Client