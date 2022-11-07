#pragma once

namespace Server::Database
{

	/**
	 * \brief Get the database connection URI
	 *
	 */
	auto getURI() -> std::string;

	/**
	 * \brief Get the database connection string
	 *
	 */
	auto getConnectionString() -> std::string;

	/**
	 * \brief Get the database username
	 *
	 */
	auto getUsername() -> std::string;

	/**
	 * \brief Get the database password
	 *
	 */
	auto getPassword() -> std::string;

} // namespace Server::Database