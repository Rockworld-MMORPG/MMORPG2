#include "Login/LoginManager.hpp"
#include "entt/core/hashed_string.hpp"
#include <Argon2/Argon2.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <nlohmann/json.hpp>
#include <random>

namespace Server
{

	LoginManager::LoginManager(DatabaseManager& databaseManager) :
	    m_databaseManager(databaseManager)
	{
	}

	auto LoginManager::createUser(const std::string& username, const std::string& password) -> Login::CreateResult
	{
		auto filter = nlohmann::json();
		filter.emplace("username", username);

		auto optData = m_databaseManager.get("rockworld_testing", "logins", filter);
		if (optData.has_value())
		{
			return Login::CreateResult::UsernameTaken;
		}

		spdlog::debug("Creating login for {}", username);

		auto randomEngine = std::mt19937_64();
		auto salt         = std::string();
		salt.reserve(32);
		for (auto i = 0; i < 32; ++i)
		{
			auto character = static_cast<char>(randomEngine());
			salt.push_back(character);
		}

		auto hashedPassword = hashString(password, salt);
		auto json           = nlohmann::json();
		json.emplace("username", username);
		json.emplace("salt", std::vector<std::uint8_t>(salt.begin(), salt.end()));
		json.emplace("password", std::vector<std::uint8_t>(hashedPassword.begin(), hashedPassword.end()));

		m_databaseManager.insert("rockworld_testing", "logins", json);


		return Login::CreateResult::Created;
	}

	auto LoginManager::authenticate(const std::string& username, const std::string& password) -> Login::AuthenticationResult
	{
		spdlog::debug("Authenticating user {}", username);

		if (isLoggedIn(username))
		{
			spdlog::debug("Username is already logged in");
			return Login::AuthenticationResult::AlreadyLoggedIn;
		}

		auto query     = nlohmann::json::parse("{\"username\": \"" + username + "\"}");
		auto optResult = m_databaseManager.get("rockworld_testing", "logins", query);
		if (!optResult.has_value())
		{
			spdlog::debug("User does not exist");
			return Login::AuthenticationResult::InvalidUsername;
		}

		auto salt = std::string();
		salt.reserve(32);
		for (const auto& value : optResult->at("salt"))
		{
			salt.push_back(value.get<std::uint8_t>());
		}

		auto hashedPassword = hashString(password, salt);

		auto remotePassword = std::string();

		for (const auto& value : optResult->at("password"))
		{
			remotePassword.push_back(value.get<std::uint8_t>());
		}

		if (hashedPassword != remotePassword)
		{
			spdlog::debug("Invalid password");
			return Login::AuthenticationResult::InvalidPassword;
		}

		return Login::AuthenticationResult::Valid;
	}

	auto LoginManager::isLoggedIn(const std::string& username) -> bool
	{
		auto hashedUsername = entt::hashed_string(username.c_str());
		return m_usersLoggedIn.contains(hashedUsername.value());
	}

	auto LoginManager::login(const std::string& username) -> void
	{
		spdlog::debug("Logging in user {}", username);
		auto hashedUsername = entt::hashed_string(username.c_str());
		m_usersLoggedIn.emplace(hashedUsername.value());
	}

	auto LoginManager::logout(const std::string& username) -> void
	{
		spdlog::debug("Logging out user {}", username);
		auto hashedUsername = entt::hashed_string(username.c_str());
		m_usersLoggedIn.erase(hashedUsername.value());
	}

	auto LoginManager::hashString(const std::string& input, const std::string& salt) -> std::string
	{
		const auto TIME_COST        = std::uint32_t(2);
		const auto MEMORY_COST      = std::uint32_t(1 << 16);
		const auto PARALLELISM_COST = std::uint32_t(1);
		const auto HASH_LENGTH      = std::uint32_t(32);

		auto output = std::vector<uint8_t>(HASH_LENGTH);

		auto vInput = std::vector<std::uint8_t>(input.begin(), input.end());
		auto vSalt  = std::vector<std::uint8_t>(salt.begin(), salt.end());

		Argon2::id_hash_raw(TIME_COST, MEMORY_COST, PARALLELISM_COST, vInput, vSalt, output);
		return {output.begin(), output.end()};
	}

} // namespace Server