#include "Shell/CommandShell.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <sstream>

namespace Server
{

	constexpr auto hashString(const char* str, size_t length) -> uint64_t
	{
		// Hash a string using FNV-1a
		// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1a_hash

		constexpr auto FNV_OFFSET_BIAS = uint64_t(0xcbf29ce484222325);
		constexpr auto FNV_PRIME       = uint64_t(0x100000001b3);

		auto hash = std::uint64_t(FNV_OFFSET_BIAS);
		for (size_t i = 0; i < length; i++)
		{
			hash ^= str[i];
			hash *= FNV_PRIME;
		}
		return hash;
	}

	auto CommandShell::parseMessage(const std::string& message) -> void
	{
		auto tokens = std::vector<std::string>();
		std::stringstream sstream(message);
		std::copy(std::istream_iterator<std::string>(sstream), std::istream_iterator<std::string>(), std::back_inserter(tokens));

		auto command   = tokens.front();
		auto hsCommand = hashString(command.c_str(), command.size());
		if (m_commandHandlers.contains(hsCommand))
		{
			m_commandHandlers.at(hsCommand)(tokens);
		}
	}

	auto CommandShell::registerCommand(const std::string& commandName, CommandHandler handler) -> void
	{
		auto hsCommandName = hashString(commandName.c_str(), commandName.size());
		m_commandHandlers.emplace(hsCommandName, handler);
	}

} // namespace Server