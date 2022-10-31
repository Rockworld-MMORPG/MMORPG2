#include "Login/LoginManager.hpp"
#include "entt/core/hashed_string.hpp"
#include <Argon2/Argon2.hpp>
#include <random>

namespace Server
{

	LoginManager::LoginManager(DatabaseManager& databaseManager) :
	    m_databaseManager(databaseManager)
	{
	}

	auto LoginManager::createUser(const std::string& username, const std::string& password) -> Login::CreateResult
	{
		spdlog::debug("Creating login for {}", username);

		auto query = m_databaseManager.executeQuery("SELECT * from login_data WHERE username=@USERNAME");
		query.bind("@USERNAME", username);

		if (query.executeStep())
		{
			spdlog::debug("Login already exists");
			return Login::CreateResult::UsernameTaken;
		}

		auto randomEngine = std::mt19937_64();
		auto salt         = std::string();
		salt.reserve(32);
		for (auto i = 0; i < 32; ++i)
		{
			auto character = static_cast<char>(randomEngine());
			salt.push_back(character);
		}

		auto hashedPassword = hashString(password, salt);

		query = m_databaseManager.executeQuery("INSERT INTO login_data (username, password, salt) VALUES (@USERNAME, @PASSWORD, @SALT)");
		query.bind("@USERNAME", username);
		query.bind("@PASSWORD", hashedPassword);
		query.bind("@SALT", salt);
		if (query.exec() != 1)
		{
			spdlog::warn("Failed to create user {}", username);
			return Login::CreateResult::UsernameTaken;
		}

		return Login::CreateResult::Created;
	}

	auto LoginManager::authenticate(const std::string& username, const std::string& password) -> Login::AuthenticationResult
	{
		spdlog::debug("Authenticating user {}", username);

		auto query = m_databaseManager.executeQuery("SELECT salt from login_data WHERE username=@USERNAME");
		query.bind("@USERNAME", username);

		const auto* salt = "";
		if (query.executeStep())
		{
			salt = query.getColumn(0);
		}
		else
		{
			return Login::AuthenticationResult::InvalidUsername;
		}

		auto hashedPassword = hashString(password, salt);

		query = m_databaseManager.executeQuery("SELECT * from login_data where username=@USERNAME and password=@PASSWORD");
		query.bind("@USERNAME", username);
		query.bind("@PASSWORD", hashedPassword);

		if (!query.executeStep())
		{
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
		std::string output;
		std::vector<std::uint8_t> vInput(input.begin(), input.end());
		std::vector<std::uint8_t> vSalt(salt.begin(), salt.end());

		Argon2::id_hash_encoded(3, 1024 * 32, 1, vInput, vSalt, 32, output);
		return output;
	}

} // namespace Server