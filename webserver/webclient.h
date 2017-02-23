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
		void interpretClientRequest(const std::vector<std::string>& lines, std::string& method, std::string& uri, std::string& protocol, std::map<std::string,std::string>& arguments, std::map<std::string,std::string>& headers);
		std::string select(const std::string& name, const std::map<std::string,std::string>& options, const std::string& cmd = "", const std::string& target = "", const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
		std::string slider(const std::string& name, const std::string& cmd, const unsigned int min, const unsigned int max, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
		std::string button(const std::string& value, const std::string& cmd, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
		void printLoco(const std::map<std::string, std::string>& arguments);
		void printMainHTML();
		void simpleReply(const std::string& text, const std::string& code = "200 OK");
    void deliverFile(const std::string& file);
		void handleLocoSpeed(const std::map<std::string,std::string>& arguments);
		void handleLocoDirection(const std::map<std::string,std::string>& arguments);
		void handleLocoFunction(const std::map<std::string, std::string>& arguments);
		void handleUpdater(const std::map<std::string,std::string>& arguments);

		unsigned int id;
		int clientSocket;
		volatile unsigned char run;
		WebServer &server;
		std::thread clientThread;
		Manager& manager;
		bool headOnly;
		unsigned int buttonID;
};

}; // namespace webserver

#endif // WEBSERVER_WEBCLIENT_H

