#pragma once

#include <cstdint>

namespace Common::Network
{

	using ClientID_t = std::uint64_t;

	struct ClientID
	{
	public:
		ClientID();
		explicit ClientID(ClientID_t clientID);
		[[nodiscard]] auto get() const -> ClientID_t;

		operator ClientID_t() const;

	private:
		ClientID_t m_id;
	};

	struct ClientIDHash
	{
		auto operator()(const ClientID& clientID) const noexcept -> std::size_t;
	};

} // namespace Common::Network