#include "Engine/Engine.hpp"
#include "Game/Game.hpp"
#include "Version.hpp"
#include <spdlog/spdlog.h>


using namespace Client;

auto main(int /* argc */, char** argv) -> int
{
#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::trace);
#endif

	spdlog::info("Client version {}.{}.{}", Version::getMajor(), Version::getMinor(), Version::getPatch());

	auto executablePath      = std::filesystem::path(*argv);
	const auto executableDir = executablePath.parent_path();
	const auto assetDir      = executableDir / "assets";


	Engine engine(assetDir);
	engine.pushState(std::make_unique<Game::Game>(engine));
	engine.run();
	return 0;
}