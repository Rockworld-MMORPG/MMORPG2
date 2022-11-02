#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Server
{

	using CommandHandler = std::function<void(std::vector<std::string>)>;

	/**
	 * \class CommandShell CommandShell.hpp "Shell/CommandShell.hpp"
	 * \brief Parses and handles command-type messages
	 */
	class CommandShell
	{
	public:
		/**
		 * \brief Parse a string message and call any relevant handlers
		 *
		 * \param message The string message to parse
		 */
		auto parseMessage(const std::string& message) -> void;

		/**
		 * \brief Register a handler for a given command
		 *
		 * \param commandName The name of the command to registry the handler to
		 * \param handler The handler function to call when the command is received
		 */
		auto registerCommand(const std::string& commandName, CommandHandler handler) -> void;

	private:
		std::unordered_map<std::uint64_t, CommandHandler> m_commandHandlers;
	};

} // namespace Server