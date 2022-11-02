#include "Server/Server.hpp"
#include "Version.hpp"
#include <spdlog/spdlog.h>

auto main(int argc, char** argv) -> int
{
	spdlog::set_level(spdlog::level::debug);

	spdlog::info("Server version {}.{}.{}", Server::Version::getMajor(), Server::Version::getMinor(), Server::Version::getPatch());
	auto server = Server::Server(std::filesystem::path(argv[0]).parent_path());
	server.run();

	return 0;
}