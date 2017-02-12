#include <algorithm>
#include <cstring>		//memset
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

	void WebClient::interpretClientRequest(const string& str, string& method, string& uri, string& protocol, map<string,string>& arguments) {
		vector<string> list;
		str_split(str, string(" "), list);
		if (list.size() == 3) {
			method = list[0];
			// transform method to uppercase
			std::transform(method.begin(), method.end(), method.begin(), ::toupper);
			// if method == HEAD set membervariable
			headOnly = false;
			if (method.compare("HEAD") == 0) {
				headOnly = true;
			}
			// set uri and protocol
			uri = list[1];
			protocol = list[2];
			// read GET-arguments from uri
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
	}

	void WebClient::deliverFile(const string& virtualFile) {
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
				snprintf(header, sizeof(header),
					"HTTP/1.0 200 OK\r\n"
					"Cache-Control: max-age=3600"
					"Content-Lenth: %lu\r\n"
					"Content-Type: %s\r\n\r\n",
					s.st_size, contentType);
				send_timeout(clientSocket, header, strlen(header), 0);
				if (headOnly == false) {
					char* buffer = static_cast<char*>(malloc(s.st_size));
					if (buffer) {
						size_t r = fread(buffer, 1, s.st_size, f);
						send_timeout(clientSocket, buffer, r, 0);
						free(buffer);
						fclose(f);
						return;
					}
				}
			}
			fclose(f);
		}
		char reply[1024];
		snprintf(reply, sizeof(reply),
			"HTTP/1.0 404 Not found\r\n\r\n"
			"<!DOCTYPE html><html><head><title>404 Not found</title></head><body><p>File %s not found</p></body></html>",
			virtualFile.c_str());
		send_timeout(clientSocket, reply, strlen(reply), 0);
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
		send_timeout(socket, buffer_out, strlen(buffer_out), 0);
	*/
	}

	void WebClient::simpleReply(const string& text, const string& code) {
		char reply[1024];
		snprintf(reply, sizeof(reply),
			"HTTP/1.0 %s\r\n"
			"Cache-Control: no-cache, must-revalidate\r\n"
			"Pragma: no-cache\r\n"
			"Expires: Sun, 12 Feb 2016 00:00:00 GMT\r\n"
			"Content-Type: text/html; charset=utf-8\r\n\r\n"
			"%s",
			code.c_str(), text.c_str());
		send_timeout(clientSocket, reply, strlen(reply), 0);
	}

	// worker is the thread that handles client requests
	void WebClient::worker() {
		xlog("Executing webclient");
		run = true;

		char buffer_in[1024];
		memset(buffer_in, 0, sizeof(buffer_in));

		size_t pos = 0;
		string s;
		while(pos < sizeof(buffer_in) - 1 && s.find("\n\n") == string::npos) {
			pos += recv_timeout(clientSocket, buffer_in + pos, sizeof(buffer_in) - 1 - pos, 0);
			s = string(buffer_in);
			str_replace(s, string("\r\n"), string("\n"));
			str_replace(s, string("\r"), string("\n"));
		}
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
		map<string, string> arguments;
		interpretClientRequest(lines[0], method, uri, protocol, arguments);
		xlog("%s %s", method.c_str(), uri.c_str());

		// if method is not implemented
		if ((method.compare("GET") != 0) && (method.compare("HEAD") != 0)) {
			xlog("Method %s not implemented", method.c_str());
			const char* reply =
				"HTTP/1.0 501 Not implemented\r\n\r\n"
				"<!DOCTYPE html><html><head><title>501 Not implemented</title></head><body><p>Method not implemented</p></body></html>";
			send_timeout(clientSocket, reply, strlen(reply), 0);
			close(clientSocket);
			return;
		}

		/*
		xlog(arguments["cmd"].c_str());
		for (auto argument : arguments) {
			xlog("%s=%s", argument.first.c_str(), argument.second.c_str());
		}
		*/

		// handle requests
		if (arguments["cmd"].compare("quit") == 0) {
			simpleReply("Stopping Railcontrol");
			manager.stop(MANAGER_ID_WEBSERVER);
			stopRailControl(SIGTERM);
		}
		else if (arguments["cmd"].compare("on") == 0) {
			simpleReply("Turning booster on");
			manager.go(MANAGER_ID_WEBSERVER);
		}
		else if (arguments["cmd"].compare("off") == 0) {
			simpleReply("Turning booster off");
			manager.stop(MANAGER_ID_WEBSERVER);
		}
		else if (arguments["cmd"].compare("speed") == 0) {
			handleLocoSpeed(clientSocket, arguments);
		}
		else if (uri.compare("/") == 0) {
			printMainHTML();
		}
		else {
			deliverFile(uri);
		}

		xlog("Terminating webclient");
		close(clientSocket);
	}

	string WebClient::button(const string& value, const string& cmd) {
		stringstream ss;
		ss <<
			"<input class=\"button\" id=\"" << cmd << "\" type=\"submit\" value=\"" << value << "\">"
			"<script>\n$(function() {\n$(\"#" << cmd << "\").on(\"click\", function() {\n$(\"#status\").load(\"/?cmd=" << cmd << "\");\nreturn false;\n })\n})\n</script>";
		return ss.str();
	}

	void WebClient::printMainHTML() {
			// handle base request
			stringstream ss;
			ss << "HTTP/1.0 200 OK\r\n"
			"Cache-Control: no-cache, must-revalidate\r\n"
			"Pragma: no-cache\r\n"
			"Expires: Sun, 12 Feb 2016 00:00:00 GMT\r\n"
			"Content-Type: text/html; charset=utf-8\r\n\r\n"
			"<!DOCTYPE html><html><head>"
			"<title>RailControl</title>"
			"<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\" />"
			"<script src=\"/jquery-3.1.1.min.js\"></script>"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			"<meta name=\"robots\" content=\"noindex,nofollow\">"
			"</head>"
			"<body>"
			"<h1>Railcontrol</h1>"
			"<div class=\"menu\">";
			ss << button("X", "quit");
			ss << button("On", "on");
			ss << button("Off", "off");
			ss << "</div>"
			"<div class=\"locolist\">"
			"<select name=\"locolist\">";
			// locolist
			const map<locoID_t, Loco*>& locos = manager.locoList();
			for (auto locoTMP : locos) {
				Loco* loco = locoTMP.second;
				ss << "<option value=\"" << loco->locoID << "\">" << loco->name << "</option>";
			}
			ss << "</select>"
			"</div>"
			"<div class=\"loco\">Loco</div>"
			"<div class=\"popup\">Popup</div>"
			"<div class=\"status\" id=\"status\">Status</div>"
			"</body></html>";
			string sOut = ss.str();
			const char* html = sOut.c_str();
			send_timeout(clientSocket, html, strlen(html), 0);
	}

	int WebClient::stop() {
		// inform thread to stop
		run = false;
		return 0;
	}

} ; // namespace webserver
