#include "Engine/Engine.hpp"
#include "States/Login.hpp"
#include "Version.hpp"

using namespace Client;

auto main(int /* argc */, char** argv) -> int
{
#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::trace);
#endif

	spdlog::info("Client version {}.{}.{}+{:08x}", Version::getMajor(), Version::getMinor(), Version::getPatch(), Version::getCommit());

	Engine engine(std::filesystem::path(*argv).parent_path());
	engine.pushState(std::make_unique<States::Login>(engine));
	engine.run();
	return 0;
}