#pragma once

namespace Server
{

	class Server;

	/**
	 * \class Manager Manager.hpp "Server/Manager.hpp"
	 * \brief Base class for any managers to be attached to a Server object
	 */
	class Manager
	{
	public:
		/**
		 * \brief Construct a new Manager object
		 *
		 * \param server A reference to the Server object the manager should affect
		 */
		Manager(Server& server) :
		    server(server){};

		/**
		 * \brief Destroy the Manager object
		 *
		 */
		virtual ~Manager() = default;

		/**
		 * \brief Update the manager
		 *
		 */
		virtual auto update() -> void {}

	protected:
		Server& server;
	};

} // namespace Server