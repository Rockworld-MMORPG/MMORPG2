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

	spdlog::info("Client version {}.{}.{}+{:08x}", Version::getMajor(), Version::getMinor(), Version::getPatch(), Version::getCommit());

	Engine engine(std::filesystem::path(*argv).parent_path());
	engine.pushState(std::make_unique<Game::Game>(engine));
	engine.run();
	return 0;
}