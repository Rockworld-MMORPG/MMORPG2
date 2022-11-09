#pragma once

#include "Common/Export.hpp"
#include "Common/Network/MessageData.hpp"
#include <cstdint>

namespace Common::Network
{

	using MessageType_t = std::uint16_t;
	enum class MessageType : MessageType_t
	{
		// 0x0 - Control messages
		None,
		Command,

		Client_PublicKey,
		Client_Connect,
		Client_Disconnect,
		Client_Authenticate,
		Client_GetClientID,
		Client_Spawn,
		Client_Action,
		Client_InputState,
		Client_GetWorldState,

		Server_PublicKey,
		Server_Authenticate,
		Server_SetClientID,
		Server_Disconnect,
		Server_CreateEntity,
		Server_DestroyEntity,
		Server_WorldState,

		Server_InputState,
		Server_Stats,
		Server_Position,
		Server_Name
	};

	COMMON_API auto operator<<(MessageData& messageData, MessageType messageType) -> MessageData&;
	COMMON_API auto operator>>(MessageData& messageData, MessageType& messageType) -> MessageData&;

}; // namespace Common::Network