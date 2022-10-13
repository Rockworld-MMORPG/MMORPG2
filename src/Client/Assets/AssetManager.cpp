#include "Assets/AssetManager.hpp"
#include <fstream>

namespace Client
{

	AssetManager::AssetManager(const std::filesystem::path& executableDirectory) :
	    m_assetDirectory(executableDirectory / "assets")
	{
		m_assets.emplace("error", std::vector<char>());
	}

	auto AssetManager::loadAsset(const std::filesystem::path& filepath, std::string identifier) -> void
	{
		auto assetLocation = m_assetDirectory / filepath;
		if (!std::filesystem::exists(assetLocation))
		{
			return;
		}

		if (m_assets.contains(identifier))
		{
			return;
		}

		std::ifstream reader(assetLocation, std::ios::in | std::ios::ate | std::ios::binary);
		auto fileSize = reader.tellg();
		reader.seekg(std::ios::beg);

		std::vector<char> buffer;
		buffer.resize(fileSize);
		reader.read(buffer.data(), fileSize);

		m_assets.emplace(identifier, std::move(buffer));
	}

	auto AssetManager::unloadAsset(const std::string& identifier) -> void
	{
		m_assets.erase(identifier);
	}

	auto AssetManager::getAsset(const std::string& identifier) const -> const std::vector<char>&
	{
		if (!m_assets.contains(identifier))
		{
			return m_assets.at("error");
		}
		return m_assets.at(identifier);
	}

} // namespace Client