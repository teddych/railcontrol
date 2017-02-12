#ifndef WEBSERVER_WEBCLIENT_H
#define WEBSERVER_WEBCLIENT_H

#include <map>
#include <thread>
#include <string>
#include <vector>

#include "manager.h"

namespace webserver {

class WebServer;

class WebClient {
	public:
		WebClient(const unsigned int id, int socket, WebServer &webserver, Manager& manager);
		~WebClient();
		void worker();
		int stop();
	private:
		void printMainHTML();
		void interpretClientRequest(const std::string& str, std::string& method, std::string& uri, std::string& protocol, std::map<std::string,std::string>& arguments);
		void simpleReply(const std::string& text, const std::string& code = "200 OK");
    void deliverFile(const std::string& file);
		void handleLocoSpeed(const int socket, const std::map<std::string,std::string>& arguments);
		unsigned int id;
		int clientSocket;
		volatile unsigned char run;
		WebServer &server;
		std::thread clientThread;
		Manager& manager;
		bool headOnly;
};

}; // namespace webserver

#endif // WEBSERVER_WEBCLIENT_H

