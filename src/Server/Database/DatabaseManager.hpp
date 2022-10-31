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
		DatabaseManager(const std::filesystem::path& databasePath);

		auto createTables() -> void;
		auto executeQuery(const std::string& query) -> SQLite::Statement;

	private:
		SQLite::Database m_databaseConnection;
	};

} // namespace Server