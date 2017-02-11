#include <algorithm>
#include <cstring>		//memset
#include <map>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "datamodel/datamodel.h"
#include "railcontrol.h"
#include "util.h"
#include "webclient.h"

using std::map;
using std::stoi;
using std::string;
using std::stringstream;
using std::thread;
using std::vector;
using datamodel::Loco;

namespace webserver {

	WebClient::WebClient(const unsigned int id, int socket, WebServer& webserver, Manager& m) :
		id(id),
		clientSocket(socket),
		run(false),
		server(webserver),
		clientThread(thread([this] {worker();})),
		manager(m) {
	}

	WebClient::~WebClient() {
		// inform thread to stop
		run = false;
		// join thread
		clientThread.join();
	}

	void WebClient::getCommand(const string& str, string& method, string& uri, string& protocol) {
		// FIXME: move argument evaluation to here
		vector<string> list;
		str_split(str, string(" "), list);
		if (list.size() == 3) {
			method = list[0];
			uri = list[1];
			protocol = list[2];
		}
	}

	void WebClient::deliverFile(const int socket, const string& virtualFile) {
		stringstream ss;
		char workingDir[128];
		int rc;
		if (getcwd(workingDir, sizeof(workingDir))) {
			ss << workingDir << "/html" << virtualFile;
		}
		string sFile = ss.str();
		const char* realFile = sFile.c_str();
		xlog(realFile);
		FILE* f = fopen(realFile, "r");
		if (f) {
			struct stat s;
			rc = stat(realFile, &s);
			if (rc == 0) {
				size_t length = virtualFile.length();
				const char* contentType = NULL;
				if (length > 3 && virtualFile[length - 3] == '.' && virtualFile[length - 2] == 'j' && virtualFile[length - 1] == 's') {
					contentType = "application/javascript";
				}
				else if (length > 4 && virtualFile[length - 4] == '.') {
					if (virtualFile[length - 3] == 'i' && virtualFile[length - 2] == 'c' && virtualFile[length - 1] == 'o') {
						contentType = "image/x-icon";
					}
					else if (virtualFile[length - 3] == 'c' && virtualFile[length - 2] == 's' && virtualFile[length - 1] == 's') {
						contentType = "text/css";
					}
				}
				char header[1024];
				snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nContent-Lenth: %lu\r\nContent-Type: %s\r\n\r\n", s.st_size, contentType);
				send(socket, header, strlen(header), 0);
				if (headOnly == false) {
					char* buffer = static_cast<char*>(malloc(s.st_size));
					if (buffer) {
						size_t r = fread(buffer, 1, s.st_size, f);
						send(socket, buffer, r, 0);
						free(buffer);
					}
				}
			}
			fclose(f);
		}
	}

	void WebClient::handleLocoSpeed(const int socket, const map<string, string>& arguments) {
/*
		locoID_t locoID = 0;
		speed_t speed = 0;
		if (arguments.count("locoid")) locoID = std::stoi(arguments.at("locoid"));
		if (arguments.count("speed")) speed = std::stoi(arguments.at("speed"));

		char buffer[1024];
		snprintf(buffer, sizeof(buffer), "<p>loco %u speed is now set to %i</p>", locoID, speed);
		manager.locoSpeed(MANAGER_ID_WEBSERVER, locoID, speed);
		char buffer_out[1024];
		snprintf(buffer_out, sizeof(buffer_out), htmlTemplate, "200 OK", "RailControl", "", buffer);
		send(socket, buffer_out, strlen(buffer_out), 0);
	*/
	}

	// worker is the thread that handles client requests
	void WebClient::worker() {
		xlog("Executing webclient");
		run = true;

		char buffer_in[1024];
//		char buffer_out[1024];

		recv(clientSocket, buffer_in, sizeof(buffer_in), 0);
		string s(buffer_in);
		str_replace(s, string("\r\n"), string("\n"));
		str_replace(s, string("\r"), string("\n"));
		vector<string> lines;
		str_split(s, string("\n"), lines);

		if (lines.size() < 1) {
			xlog("Invalid request");
			close(clientSocket);
			return;
		}
		string method;
		string uri;
		string protocol;
		getCommand(lines[0], method, uri, protocol);
		xlog(method.c_str());
		xlog(uri.c_str());

		// transform method to uppercase
		std::transform(method.begin(), method.end(), method.begin(), ::toupper);

		if ((method.compare("GET") != 0) && (method.compare("HEAD") != 0)) {
			xlog("Method not implemented");
			// FIXME: send response
//			snprintf(buffer_out, sizeof(buffer_out), htmlTemplate, "501 Not implemented", "Not implemented", "", "<p>This request method is not implemented</p>");
//			send(clientSocket, buffer_out, strlen(buffer_out), 0);
			close(clientSocket);
			return;
		}

		headOnly = false;
		if (method.compare("HEAD") == 0) {
			headOnly = true;
		}

		// read GET-arguments
		map<string, string> arguments;
		{ // we have several temp Objects
			vector<string> uri_parts;
			str_split(uri, "?", uri_parts);
			if (uri_parts.size() == 2) {
				vector<string> argumentStrings;
				str_split(uri_parts[1], "&", argumentStrings);
				for (auto argument : argumentStrings) {
					vector<string> argumentParts;
					str_split(argument, "=", argumentParts);
					arguments[argumentParts[0]] = argumentParts[1];
				}
			}
		}

		/*
		xlog(arguments["cmd"].c_str());
		for (auto argument : arguments) {
			xlog("%s=%s", argument.first.c_str(), argument.second.c_str());
		}
		*/

		// handle requests
		if (arguments["cmd"].compare("quit") == 0) {
			//snprintf(buffer_out, sizeof(buffer_out), htmlTemplate, "200 OK", "RailControl", "", "<p>Railcontrol is shutting down</p>");
			//send(clientSocket, buffer_out, strlen(buffer_out), 0);
			stopRailControl(SIGTERM);
		}
		else if (arguments["cmd"].compare("on") == 0) {
			//snprintf(buffer_out, sizeof(buffer_out), htmlTemplate, "200 OK", "RailControl", "", "<p>Turning on Booster</p>");
			//send(clientSocket, buffer_out, strlen(buffer_out), 0);
			manager.go(MANAGER_ID_WEBSERVER);
		}
		else if (arguments["cmd"].compare("off") == 0) {
			//snprintf(buffer_out, sizeof(buffer_out), htmlTemplate, "200 OK", "RailControl", "", "<p>Turning off Booster</p>");
			//send(clientSocket, buffer_out, strlen(buffer_out), 0);
			manager.stop(MANAGER_ID_WEBSERVER);
		}
		else if (arguments["cmd"].compare("speed") == 0) {
			handleLocoSpeed(clientSocket, arguments);
		}
		else if (uri.compare("/") == 0) {
			printMainHTML();
		}
		else {
			deliverFile(clientSocket, uri);
		}

		xlog("Terminating webclient");
		close(clientSocket);
	}

	void WebClient::printMainHTML() {
			// handle base request
			stringstream ss;
			ss << "HTTP/1.0 200 OK\r\n"
			"Content-Type: text/html; charset=utf-8\r\n\r\n"
			"<!DOCTYPE html><html><head><title>RailControl</title></head>"
			"<body>"
			"<h1>Rocrail</h1>"
			"<div name=\"locolist\">";
			// locolist
			const map<locoID_t, Loco*>& locos = manager.locoList();
			ss << "<select name=\"locolist\" onChange=\">";
			for (auto locoTMP : locos) {
				Loco* loco = locoTMP.second;
				ss << "<option value=\"" << loco->locoID << "\">" << loco->name << "</option>";
			}
			ss << "</select>"
			"<div class=\"loco\">Loco</div>"
			"<div class=\"popup\">Popup</div>"
			"<p><a href=\"/?cmd=quit\">Shut down RailControl</a></p></body></html>";
			const char* html = ss.str().c_str();
			send(clientSocket, html, strlen(html), 0);
	}

	int WebClient::stop() {
		// inform thread to stop
		run = false;
		return 0;
	}

}
;
// namespace webserver
