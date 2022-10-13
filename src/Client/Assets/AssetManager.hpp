#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace Client
{

	class AssetManager
	{
	public:
		AssetManager(const std::filesystem::path& executableDirectory);

		auto loadAsset(const std::filesystem::path& filepath, std::string identifier) -> void;
		auto unloadAsset(const std::string& identifier) -> void;

		auto getAsset(const std::string& identifier) const -> const std::vector<char>&;

	private:
		std::filesystem::path m_assetDirectory;
		std::unordered_map<std::string, const std::vector<char>> m_assets;
	};

} // namespace Client