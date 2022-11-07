#include "Database/DatabaseManager.hpp"
#include "Database/Secrets.hpp"
#include "mongocxx/options/tls.hpp"
#include <bsoncxx/json.hpp>
#include <bsoncxx/view_or_value.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>
#include <nlohmann/json.hpp>

namespace Server
{

	class DBLogger : public mongocxx::logger
	{
		auto operator()(mongocxx::log_level level, mongocxx::stdx::string_view domain, mongocxx::stdx::string_view message) noexcept -> void override
		{
			switch (level)
			{
				case mongocxx::log_level::k_error:
					spdlog::error("MongoDB({}): {}", domain, message);
					break;
				case mongocxx::log_level::k_critical:
					spdlog::critical("MongoDB({}): {}", domain, message);
					break;
				case mongocxx::log_level::k_warning:
					spdlog::warn("MongoDB({}): {}", domain, message);
					break;
				case mongocxx::log_level::k_message:
					spdlog::info("MongoDB({}): {}", domain, message);
					break;
				case mongocxx::log_level::k_info:
					spdlog::info("MongoDB({}): {}", domain, message);
					break;
				case mongocxx::log_level::k_debug:
					spdlog::debug("MongoDB({}): {}", domain, message);
					break;
				case mongocxx::log_level::k_trace:
					spdlog::trace("MongoDB({}): {}", domain, message);
					break;
			}
		}
	};

	DatabaseManager::DatabaseManager()
	{
		static bool instanceCreated = false;
		if (!instanceCreated)
		{
			spdlog::debug("Creating MongoDB instance");
			mongocxx::instance::current() = mongocxx::instance(std::make_unique<DBLogger>());
			instanceCreated               = true;
		}

		connect();
	}


	auto DatabaseManager::connect() -> void
	{
		spdlog::debug("Connecting to database");
		auto uri = mongocxx::uri(Database::getConnectionString());
		m_client = mongocxx::client(uri);

		for (auto database : m_client.list_databases())
		{
			spdlog::debug(bsoncxx::to_json(database));
		}
	}

	/**
	 * \brief Insert an object into a database
	 *
	 * \param databaseName The name of the database to insert into
	 * \param tableName The name of the table to insert into
	 * \param data The data to insert into the table
	 */
	auto DatabaseManager::insert(std::string databaseName, std::string tableName, nlohmann::json data) -> void
	{
		auto database   = m_client.database(databaseName);
		auto collection = database.collection(tableName);

		collection.insert_one(bsoncxx::from_json(data.dump()));
	}

	/**
	 * \brief Update an object in the database
	 *
	 * \param databaseName The name of the database to update
	 * \param tableName The name of the table to update
	 * \param data The data to update in the table
	 */
	auto DatabaseManager::update(std::string databaseName, std::string tableName, nlohmann::json filter, mongocxx::pipeline update) -> void
	{
		auto database   = m_client.database(databaseName);
		auto collection = database.collection(tableName);

		collection.update_one(bsoncxx::from_json(filter.dump()), update);
	}

	auto DatabaseManager::replace(std::string databaseName, std::string tableName, nlohmann::json filter, nlohmann::json data) -> void
	{
		auto database   = m_client.database(databaseName);
		auto collection = database.collection(tableName);

		collection.replace_one(bsoncxx::from_json(filter.dump()), bsoncxx::from_json(data.dump()));
	}

	/**
	 * \brief Insert an object into a database
	 *
	 * \param databaseName The name of the database to insert into
	 * \param tableName The name of the table to insert into
	 * \param data The data to insert into the table
	 *
	 * \return A BSON document containing the requested document's data
	 */
	auto DatabaseManager::get(std::string databaseName, std::string tableName, nlohmann::json filter) -> std::optional<nlohmann::json>
	{
		auto database   = m_client.database(databaseName);
		auto collection = database.collection(tableName);

		auto optBSON = collection.find_one(bsoncxx::from_json(filter.dump()));
		if (optBSON.has_value())
		{
			return nlohmann::json::parse(bsoncxx::to_json(*optBSON));
		}

		return {};
	}

} // namespace Server