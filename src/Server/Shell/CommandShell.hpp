#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Server
{

	using CommandHandler = std::function<void(std::vector<std::string>)>;

	class CommandShell
	{
	public:
		auto parseMessage(const std::string& message) -> void;
		auto registerCommand(const std::string& commandName, CommandHandler handler) -> void;

	private:
		std::unordered_map<std::uint64_t, CommandHandler> m_commandHandlers;
	};

} // namespace Server