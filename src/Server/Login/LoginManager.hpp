#pragma once
#include "Database/DatabaseManager.hpp"
#include <unordered_set>

namespace Server
{

	namespace Login
	{
		/**
		 * \struct UserData LoginManager.hpp "Login/LoginManager.hpp"
		 * \brief The data used by the user to log in and authenticate
		 */
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
		/**
		 * \brief Construct a new Login Manager object
		 *
		 * \param databaseManager A reference to the server's database manager
		 */
		LoginManager(DatabaseManager& databaseManager);

		/**
		 * \brief Create a user on the database
		 *
		 * \param username The username the user wants to log in with
		 * \param password The unhashed password the user wants to log in with
		 * \return Login::CreateResult
		 */
		auto createUser(const std::string& username, const std::string& password) -> Login::CreateResult;

		/**
		 * \brief Check whether a username and password combination are valid in the database
		 *
		 * \param username The username the user is trying to log in with
		 * \param password The unhashed password the user is trying to log in with
		 * \return Login::AuthenticationResult An AuthenticationResult which encodes the result of the operation
		 */
		auto authenticate(const std::string& username, const std::string& password) -> Login::AuthenticationResult;

		/**
		 * \brief Get whether a username is already logged in
		 *
		 * \param username The username to check if it's logged in
		 * \return true The username is logged in
		 * \return false The username is not logged in
		 */
		auto isLoggedIn(const std::string& username) -> bool;

		/**
		 * \brief Log a username in
		 *
		 * \param username The username to log in
		 */
		auto login(const std::string& username) -> void;

		/**
		 * \brief Log a username out
		 *
		 * \param username The username to log out
		 */
		auto logout(const std::string& username) -> void;

	private:
		/**
		 * \brief Hash a string using a salt
		 *
		 * \param input The string to hash
		 * \param salt The salt to use to hash the string
		 * \return std::string The hashed string
		 */
		auto hashString(const std::string& input, const std::string& salt) -> std::string;

		DatabaseManager& m_databaseManager;
		std::unordered_set<std::uint32_t> m_usersLoggedIn;
	};

} // namespace Server