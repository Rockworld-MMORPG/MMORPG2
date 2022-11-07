#pragma once

#include <bsoncxx/document/view.hpp>
#include <mongocxx/client.hpp>
#include <nlohmann/json.hpp>

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
		 */
		DatabaseManager();

		/**
		 * \brief Connect to the database server
		 *
		 */
		auto connect() -> void;

		/**
		 * \brief Insert an object into a database
		 *
		 * \param databaseName The name of the database to insert into
		 * \param tableName The name of the table to insert into
		 * \param data The data to insert into the table
		 */
		auto insert(std::string databaseName, std::string tableName, nlohmann::json data) -> void;

		/**
		 * \brief Update an object in the database
		 *
		 * \param databaseName The name of the database to update
		 * \param tableName The name of the table to update
		 * \param update The pipeline used to update the object
		 */
		auto update(std::string databaseName, std::string tableName, nlohmann::json filter, mongocxx::pipeline update) -> void;

		/**
		 * \brief Replace an object in the database
		 *
		 * \param databaseName The name of the database to update
		 * \param tableName The name of the table to update
		 * \param filter The data that the object to be updated should contain
		 * \param data The new data for the object
		 */
		auto replace(std::string databaseName, std::string tableName, nlohmann::json filter, nlohmann::json data) -> void;

		/**
		 * \brief Get an object from a database
		 *
		 * \param databaseName The name of the database to insert into
		 * \param tableName The name of the table to insert into
		 * \param filter The data that the object to be found should contain
		 *
		 * \return A BSON document containing the requested document's data
		 */
		auto get(std::string databaseName, std::string tableName, nlohmann::json filter) -> std::optional<nlohmann::json>;

	private:
		mongocxx::client m_client;
	};

} // namespace Server