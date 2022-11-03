#pragma once

#include <cstdint>

namespace Client::Discord
{

	auto getClientID() -> std::int64_t;
	auto getApplicationID() -> std::int64_t;

} // namespace Client::Discord