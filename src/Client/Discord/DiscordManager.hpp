#pragma once

#include <memory>

namespace discord
{
	class Core;
}

namespace Client
{

	/**
	 * \class DiscordManager DiscordManager.hpp "Discord/DiscordManager.hpp"
	 * \brief Manages the state of the application's Discord integration
	 */
	class DiscordManager
	{
	public:
		/**
		 * \brief Get the Discord Manager singleton instance
		 */
		static auto get() -> DiscordManager&;

		/**
		 * \brief Update the Discord Manager
		 *
		 */
		auto update() -> void;

		/**
		 * \brief Set the Discord status
		 *
		 * \param playerActivity A string detailing what the player is currently doing
		 */
		auto setStatus(const std::string& playerActivity) -> void;

		/**
		 * \brief Set the Discord status
		 *
		 * \param playerActivity A string detailing what the player is currently doing
		 * \param partyActivity A string detailing what the party is currently doing
		 * \param partySize The size of the party the player is in
		 * \param maxPartySize The maximum size of the party the player is in
		 */
		auto setStatus(const std::string& playerActivity, const std::string& partyActivity, std::int32_t partySize, std::int32_t maxPartySize) -> void;

		/**
		 * \brief Set the Discord status asset
		 *
		 * \param largeImage A string containing the URL of the large image
		 * \param largeText A string detailing what the large image is
		 * \param smallImage A string containing the URL of the small image
		 * \param smallText A string detailing what the small image is
		 */
		auto setAssets(std::string largeImage, std::string largeText, std::string smallImage, std::string smallText) -> void;

		/**
		 * \struct Assets DiscordManager.hpp "Discord/DiscordManager.hpp"
		 * \brief Data representing the Discord Manager's status assets
		 */
		struct Assets
		{
			std::string largeImage;
			std::string largeText;
			std::string smallImage;
			std::string smallText;
		};

	private:
		/**
		 * \brief Construct a new Discord Manager object
		 *
		 */
		DiscordManager();

		std::unique_ptr<discord::Core> m_core;
		Assets m_assets;
	};

} // namespace Client