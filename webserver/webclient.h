#pragma once

#include <map>
#include <thread>
#include <string>
#include <vector>

#include "manager.h"
#include "network/TcpConnection.h"
#include "webserver/Response.h"

namespace webserver {

	class WebServer;

	class WebClient {
		public:
			WebClient(const unsigned int id, Network::TcpConnection* connection, WebServer &webserver, Manager& manager);
			~WebClient();
			void worker();
			int stop();

		private:
			void interpretClientRequest(const std::vector<std::string>& lines, std::string& method, std::string& uri, std::string& protocol, std::map<std::string,std::string>& arguments, std::map<std::string,std::string>& headers);
			HtmlTag selectLoco();
			std::string select(const std::string& name, const std::map<std::string,std::string>& options, const std::string& defaultValue);
			std::string slider(const std::string& name, const std::string& cmd, const unsigned int min, const unsigned int max, const unsigned int value, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
			std::string button(const std::string& value, const std::string& cmd, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
			std::string buttonPopup(const std::string& value, const std::string& cmd, const std::map<std::string,std::string>& arguments = std::map<std::string,std::string>());
			std::string buttonPopupCancel();
			std::string buttonPopupOK();
			void printLoco(const std::map<std::string, std::string>& arguments);
			void printMainHTML();
			void simpleReply(const std::string& text);
			void deliverFile(const std::string& file);
			void deliverFileInternal(FILE* f, const char* realFile, const std::string& file);
			void handleLocoSpeed(const std::map<std::string,std::string>& arguments);
			void handleLocoDirection(const std::map<std::string,std::string>& arguments);
			void handleLocoFunction(const std::map<std::string, std::string>& arguments);
			void handleLocoEdit(const std::map<std::string, std::string>& arguments);
			void handleLocoSave(const std::map<std::string, std::string>& arguments);
			void handleUpdater(const std::map<std::string,std::string>& arguments);
			void handleProtocol(const std::map<std::string, std::string>& arguments);
			void UrlDecode(std::string& argumentValue);
			char ConvertHexToInt(char c);
			void WorkerImpl();

			unsigned int id;
			Network::TcpConnection* connection;
			volatile unsigned char run;
			WebServer &server;
			std::thread clientThread;
			Manager& manager;
			bool headOnly;
			unsigned int buttonID;
	};

}; // namespace webserver

