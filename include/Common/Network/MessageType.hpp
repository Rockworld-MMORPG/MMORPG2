#pragma once

#include "Common/Export.hpp"
#include <cstdint>

namespace Common::Network
{

	class COMMON_API MessageData;

	enum class MessageType : std::uint8_t
	{
		// 0x0 - Control messages
		None       = 0x00,
		Connect    = 0x01,
		Disconnect = 0x02,
		Command    = 0x03,

		// 0x1 - Gameplay messages
		CreateEntity  = 0x10,
		DestroyEntity = 0x11,
		Movement      = 0x12,
		Position      = 0x13,
		Action        = 0x14,
		InputState    = 0x15,
		GetEntity     = 0x16,
	};

	auto operator<<(MessageData& messageData, MessageType messageType) -> MessageData&;
	auto operator>>(MessageData& messageData, MessageType& messageType) -> MessageData&;

}; // namespace Common::Network