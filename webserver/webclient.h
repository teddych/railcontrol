#ifndef WEBSERVER_WEBCLIENT_H
#define WEBSERVER_WEBCLIENT_H

#include <thread>
#include <string>
#include <vector>

#include "../manager.h"

namespace webserver {

class WebServer;

class WebClient {
	public:
		WebClient(const unsigned int id, int socket, WebServer &webserver, Manager& manager);
		~WebClient();
		void worker();
		int stop();
	private:
		void getCommand(const std::string& str, std::string& method, std::string& uri, std::string& protocol);
    void deliverFile(const int socket, const std::string& file);
		void handleLocoList(const int socket, const std::vector<std::string>& uri_parts);
		void handleLocoProperties(const int socket, const std::vector<std::string>& uri_parts);
		void handleLocoCommand(const int socket, const std::vector<std::string>& uri_parts);
		void handleLoco(const int socket, const std::string& uri);
		unsigned int id;
		int clientSocket;
		volatile unsigned char run;
		WebServer &server;
		std::thread clientThread;
		Manager& manager;
};

}; // namespace webserver

#endif // WEBSERVER_WEBCLIENT_H

