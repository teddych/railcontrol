#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <thread>

class webserver {
	public:
		webserver(unsigned short port);
		int start();
		int stop();
	private:
	  void worker();
		unsigned short port;
		int socket_server;
		int socket_client;
		unsigned char run;
		std::thread webserver_thread;
};

#endif // WEBSERVER_H

