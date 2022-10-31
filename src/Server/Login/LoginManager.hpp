#pragma once
#include "Database/DatabaseManager.hpp"
#include <unordered_set>

namespace Server
{

	namespace Login
	{
		struct UserData
		{
			std::string username;
		};

		enum class CreateResult : std::uint8_t
		{
			Created,
			UsernameTaken
		};

		enum class AuthenticationResult : std::uint8_t
		{
			Valid,
			InvalidUsername,
			InvalidPassword,
			AlreadyLoggedIn
		};
	} // namespace Login

	/**
	 * \class LoginManager LoginManager.hpp "Login/LoginManager.hpp"
	 * \brief Manages user login and authentication
	 */
	class LoginManager
	{
	public:
		LoginManager(DatabaseManager& databaseManager);

		auto createUser(const std::string& username, const std::string& password) -> Login::CreateResult;
		auto authenticate(const std::string& username, const std::string& password) -> Login::AuthenticationResult;

		auto isLoggedIn(const std::string& username) -> bool;
		auto login(const std::string& username) -> void;
		auto logout(const std::string& username) -> void;

	private:
		auto hashString(const std::string& input, const std::string& salt) -> std::string;

		DatabaseManager& m_databaseManager;
		std::unordered_set<std::uint32_t> m_usersLoggedIn;
	};

} // namespace Server