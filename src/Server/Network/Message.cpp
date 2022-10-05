#include "Message.hpp"
#include "Common/Network/ClientID.hpp"

namespace Server
{

	Message::Message(const Common::Network::ClientID clientID, const Protocol protocol) :
	    clientID(clientID),
	    protocol(protocol)
	{
	}

} // namespace Server