#include "Common/Network/MessageType.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Network
{

	auto operator<<(MessageData& messageData, const MessageType messageType) -> MessageData&
	{
		return messageData << static_cast<std::uint8_t>(messageType);
	}

	auto operator>>(MessageData& messageData, MessageType& messageType) -> MessageData&
	{
		std::uint8_t mType = 0;
		messageData >> mType;
		messageType = static_cast<MessageType>(mType);
		return messageData;
	}

} // namespace Common::Network