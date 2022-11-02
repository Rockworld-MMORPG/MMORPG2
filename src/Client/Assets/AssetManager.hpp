#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Client
{

	/**
	 * \class AssetManager AssetManager.hpp "Assets/AssetManager.hpp"
	 * \brief Handles the loading, unloading, and storage of game files
	 */
	class AssetManager
	{
	public:
		/**
		 * \brief Construct a new Asset Manager object
		 *
		 * \param executableDirectory The path of the application directory (usually the parent_path of argv[0])
		 */
		AssetManager(const std::filesystem::path& executableDirectory);

		/**
		 * \brief Loads an asset into the asset manager
		 *
		 * \param filepath The filepath of the asset
		 * \param identifier The identifier to give to the asset
		 */
		auto loadAsset(const std::filesystem::path& filepath, std::string identifier) -> void;

		/**
		 * \brief Unloads an asset from the asset manager
		 *
		 * \param identifier The identifier of the asset to unload
		 */
		auto unloadAsset(const std::string& identifier) -> void;

		/**
		 * \brief Get an asset
		 *
		 * \param identifier The identifier of the asset to get
		 * \return const std::vector<char>& A byte vector containing the asset data
		 */
		auto getAsset(const std::string& identifier) const -> const std::vector<char>&;

	private:
		std::filesystem::path m_assetDirectory;
		std::unordered_map<std::string, const std::vector<char>> m_assets;
	};

} // namespace Client