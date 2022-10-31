#include "Database/DatabaseManager.hpp"
#include "SQLiteCpp/Database.h"

namespace Server
{

	DatabaseManager::DatabaseManager(const std::filesystem::path& databasePath) :
	    m_databaseConnection(databasePath, SQLite::OPEN_READWRITE)
	{
	}

	auto DatabaseManager::createTables() -> void
	{
		auto statement = SQLite::Statement(m_databaseConnection,
		                                   R"(
    CREATE TABLE IF NOT EXISTS login_data (
      id        INTEGER		NOT NULL	UNIQUE,
      username  TEXT			NOT NULL	UNIQUE,
      password  TEXT			NOT NULL,
      salt      TEXT			NOT NULL,
			PRIMARY KEY("id" AUTOINCREMENT)
    )
    )");
		statement.exec();

		statement = SQLite::Statement(m_databaseConnection,
		                              R"(
    CREATE TABLE IF NOT EXISTS players ( 
      id          INTEGER 	NOT NULL	UNIQUE,
      name        TEXT 			NOT NULL	UNIQUE,
      instance    INT  			NOT NULL,
      x_position  FLOAT			NOT NULL,
      y_position  FLOAT			NOT NULL,
			PRIMARY KEY("id" AUTOINCREMENT)
    )
    )");
		statement.exec();
	}

	auto DatabaseManager::createQuery(const std::string& query) -> SQLite::Statement
	{
		auto statement = SQLite::Statement(m_databaseConnection, query);
		return statement;
	}

} // namespace Server