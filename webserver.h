#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <thread>
#include <string>
#include <vector>

class webserver;

class webserver_client {
	public:
		webserver_client(unsigned int id, int socket, webserver &webserver);
		~webserver_client();
		void worker();
		int stop();
	private:
		void getCommand(const std::string& str, std::string& method, std::string& uri, std::string& protocol);
		unsigned int id;
		int socket;
		volatile unsigned char run;
		webserver &server;
		std::thread client_thread;
};

class webserver {
	public:
		webserver(unsigned short port);
		~webserver();
		int start();
		int stop();
	private:
	  void worker();
		unsigned short port;
		int socket_server;
		int socket_client;
		volatile unsigned char run;
		unsigned int last_client_id;
		std::thread webserver_thread;
		std::vector<webserver_client*> clients;
};

#endif // WEBSERVER_H

