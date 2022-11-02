#pragma once

#include <SQLiteCpp/Database.h>

namespace Server
{
	/**
	 * \class DatabaseManager DatabaseManager.hpp "Database/DatabaseManager.hpp"
	 * \brief Manages access to the server's database
	 */
	class DatabaseManager
	{
	public:
		/**
		 * \brief Construct a new Database Manager object
		 *
		 * \param databasePath The filepath of the server's database file
		 */
		DatabaseManager(const std::filesystem::path& databasePath);

		/**
		 * \brief Create the tables used by the server
		 *
		 */
		auto createTables() -> void;

		/**
		 * \brief Create an SQLite statement object that can affect the server's database
		 *
		 * \param query The SQL query to execute
		 * \return SQLite::Statement A statement object that can be executed on the database
		 */
		auto createQuery(const std::string& query) -> SQLite::Statement;

	private:
		SQLite::Database m_databaseConnection;
	};

} // namespace Server