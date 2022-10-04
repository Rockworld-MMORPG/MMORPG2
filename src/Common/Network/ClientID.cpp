#include "Common/Network/ClientID.hpp"
#include <functional>

namespace Common::Network
{

	ClientID::ClientID() :
	    m_id(-1) {}

	ClientID::ClientID(const ClientID_t clientID) :
	    m_id(clientID)
	{
	}

	auto ClientID::get() const -> ClientID_t
	{
		return m_id;
	}

	ClientID::operator ClientID_t() const
	{
		return m_id;
	}

	auto ClientIDHash::operator()(const ClientID& clientID) const noexcept -> std::size_t
	{
		return std::hash<ClientID_t>{}(clientID.get());
	}

} // namespace Common::Network