#pragma once

namespace Server
{

	class Server;

	class Manager
	{
	public:
		Manager(Server& server) :
		    server(server){};

		virtual auto update() -> void {}

	protected:
		Server& server;
	};

} // namespace Server