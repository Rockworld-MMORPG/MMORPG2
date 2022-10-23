#include <Common/World/Tile.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/System/Err.hpp>
#include <cstring>
#include <entt/core/hashed_string.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main(int argc, char** argv)
{
	sf::err().rdbuf(nullptr);

	auto assetDir = std::filesystem::path(argv[0]).parent_path() / "assets";

	auto tileImage   = sf::Image();
	auto texturePath = std::filesystem::path();
	auto tileName    = std::string();
	auto travelMode  = Common::World::Tile::TravelMode::None;

	for (auto i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "-assetdir") == 0)
		{
			if (argc == i)
			{
				std::cout << "Incorrect number of arguments (-assetdir not specified)";
				return 1;
			}

			auto path = std::filesystem::path(argv[i + 1]);
			if (!std::filesystem::exists(path))
			{
				std::cout << "Specified asset directory does not exist\n";
				return 1;
			}
			if (!std::filesystem::is_directory(path))
			{
				std::cout << "Specified asset directory is not a directory\n";
				return 1;
			}
			assetDir = path;
		}
		else if (std::strcmp(argv[i], "-texture") == 0)
		{
			if (argc == i)
			{
				std::cout << "Incorrect number of arguments (-texture not specified)";
				return 1;
			}

			auto path = assetDir / argv[i + 1];
			if (!std::filesystem::exists(path))
			{
				std::cout << "Specified texture does not exist\n";
				return 1;
			}
			if (!std::filesystem::is_regular_file(path))
			{
				std::cout << "Specified texture is not a file\n";
				return 1;
			}

			auto success = tileImage.loadFromFile(path);
			if (!success)
			{
				std::cout << "Specified texture is not a valid image (" << texturePath.string() << ")\n";
				return 1;
			}

			texturePath = argv[i + 1];
			++i;
		}
		else if (std::strcmp(argv[i], "-tilename") == 0)
		{
			if (argc == i)
			{
				std::cout << "Incorrect number of arguments (-tilename not specified)";
				return 1;
			}

			tileName = argv[i + 1];
			++i;
		}
		else if (std::strcmp(argv[i], "-travelmode") == 0)
		{
			if (argc == i)
			{
				std::cout << "Incorrect number of arguments (-texture not specified)";
				return 1;
			}

			if (std::strcmp(argv[i + 1], "walk") == 0)
			{
				travelMode = travelMode | Common::World::Tile::TravelMode::Walk;
				++i;
			}
			else if (std::strcmp(argv[i + 1], "fly") == 0)
			{
				travelMode = travelMode | Common::World::Tile::TravelMode::Fly;
				++i;
			}
			else
			{
				std::cout << "Travel mode not recognised (" << argv[i + 1] << ")\n";
			}
		}
	}

	if (texturePath.empty())
	{
		auto validPath = false;
		while (!validPath)
		{
			auto input = std::string();
			std::cout << "Enter the texture path: ";
			std::cin >> input;
			texturePath = assetDir / input;

			if (!std::filesystem::exists(texturePath))
			{
				std::cout << "File does not exist (" << texturePath.string() << ")\n";
				continue;
			}
			if (!std::filesystem::is_regular_file(texturePath))
			{
				std::cout << "Specified path is not a valid file (" << texturePath.string() << ")\n";
				continue;
			}

			auto success = tileImage.loadFromFile(texturePath);
			if (!success)
			{
				std::cout << "Specified file is not a valid image (" << texturePath.string() << ")\n";
				continue;
			}

			validPath = true;
		}
	}

	while (tileName.empty())
	{
		std::cout << "Enter the tile name: ";
		std::cin >> tileName;
	}

	auto outputName = tileName;
	outputName += ".json";

	auto nameHash = entt::hashed_string(tileName.c_str(), tileName.size());

	auto json = nlohmann::json{};
	json.emplace("identifier", nameHash.value());
	json.emplace("travel_type", static_cast<std::uint32_t>(travelMode));
	json.emplace("texture_path", texturePath.string());

	auto writer = std::ofstream(assetDir / outputName);
	writer << std::setw(4) << json << std::endl;
}
