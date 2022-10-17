#pragma once

namespace Server
{

	class Server;

	class Manager
	{
	public:
		Manager(Server& server) :
		    server(server){};
		virtual ~Manager() = default;

		virtual auto update() -> void {}

	protected:
		Server& server;
	};

} // namespace Server