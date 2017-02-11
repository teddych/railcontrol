#include <algorithm>
#include <cstring>		//memset
#include <map>
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
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
	clientThread(thread([this] { worker(); })),
	manager(m) {
}

WebClient::~WebClient() {
	// inform thread to stop
	run = false;
	// join thread
	clientThread.join();
}

void WebClient::getCommand(const string& str, string& method, string& uri, string& protocol) {
	vector<string> list;
	str_split(str, string(" "), list);
	if (list.size() == 3) {
		method = list[0];
		uri = list[1];
		protocol = list[2];
	}
}

static const char* html_header_template = "HTTP/1.0 %s\r\nContent-Type: text/html; charset=utf-8\r\n\r\n<!DOCTYPE html><html><head><title>RailControl</title></head><body><h1>%s</h1>%s<p><a href=\"/quit/\">Shut down RailControl</a></p></body></html>";

void WebClient::deliverFile(const int socket, const string& file) {
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), html_header_template, "200 OK", "RailControl", file.c_str());
	send(socket, buffer, strlen(buffer), 0);
	/*
	FILE* f = fopen(file.c_str(), "r");
	if (f) {
		fclose(f);
	}
	*/
}

void WebClient::handleLocoList(const int socket) {
	const map<locoID_t,Loco*> locos = manager.locoList();
	stringstream ss;
	ss << "<h2>List of Locos</h2>";
	/*
	for (auto loco : locos) {
		ss << "<p>" << loco->name << " " << loco->address << "</p>";
	}
	*/
	char buffer_out[1024];
	snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", ss.str().c_str());
	send(socket, buffer_out, strlen(buffer_out), 0);
}

/*
void WebClient::handleLocoProperties(const int socket, const vector<string>& uri_parts) {
	unsigned int loco_id = std::stoi(uri_parts[2]);
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "<p>Properties of loco %u</p>", loco_id);
	char buffer_out[1024];
	snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", buffer);
	send(socket, buffer_out, strlen(buffer_out), 0);
}
*/
/*
void WebClient::handleLoco(const int socket, const string& uri) {
	else {
		char buffer_out[1024];
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "404 Not found", "RailControl", "<p>Unknown loco command</p>");
		send(socket, buffer_out, strlen(buffer_out), 0);
	}
}
*/
	/*
	}
	else if (uri_parts[3].substr(0, 1).compare("f") == 0) {
		unsigned char fx = std::stoi(uri_parts[3].substr(1));
		bool fon = (uri_parts[4].compare("on") == 0);
		snprintf(buffer, sizeof(buffer), "<p>loco %u f%i is now set to %s</p>", locoID, fx, (fon ? "on" : "off"));
	}
	*/

void WebClient::handleLocoSpeed(const int socket, const map<string,string>& arguments) {
	locoID_t locoID = 0;
	speed_t speed = 0;
	if (arguments.count("locoid")) locoID = std::stoi(arguments.at("locoid"));
	if (arguments.count("speed")) speed = std::stoi(arguments.at("speed"));

	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "<p>loco %u speed is now set to %i</p>", locoID, speed);
	manager.locoSpeed(MANAGER_ID_WEBSERVER, locoID, speed);
	char buffer_out[1024];
	snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", buffer);
	send(socket, buffer_out, strlen(buffer_out), 0);
}

// worker is the thread that handles client requests
void WebClient::worker() {
	xlog("Executing webclient");
	run = true;

	char buffer_in[1024];
	char buffer_out[1024];

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

	if (method.compare("GET") != 0) {
		xlog("Method not implemented");
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "501 Not implemented", "Not implemented", "<p>This request method is not implemented</p>");
		send(clientSocket, buffer_out, strlen(buffer_out), 0);
		close(clientSocket);
		return;
	}

	if (uri.compare("/") == 0) {
		// handle base request
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>Railcontrol is running</p>");
		send(clientSocket, buffer_out, strlen(buffer_out), 0);
	}
	else {
		// read GET-arguments
		vector<string> uri_parts;
		str_split(uri, "?", uri_parts);
		if (uri_parts.size() == 2) {
			vector<string> argumentStrings;
			str_split(uri_parts[1], "&", argumentStrings);
			map<string,string> arguments;
			for (auto argument : argumentStrings) {
				vector<string> argumentParts;
				str_split(argument, "=", argumentParts);
				arguments[argumentParts[0]] = argumentParts[1];
			}

			if (arguments["cmd"].compare("quit") == 0) {
				snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>Railcontrol is shutting down</p>");
				send(clientSocket, buffer_out, strlen(buffer_out), 0);
			}
			else if (arguments["cmd"].compare("on") == 0) {
				snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>Turning on Booster</p>");
				send(clientSocket, buffer_out, strlen(buffer_out), 0);
				manager.go(MANAGER_ID_WEBSERVER);
			}
			else if (arguments["cmd"].compare("off") == 0) {
				snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>Turning off Booster</p>");
				send(clientSocket, buffer_out, strlen(buffer_out), 0);
				manager.stop(MANAGER_ID_WEBSERVER);
			}
			else if (arguments["cmd"].compare("locolist") == 0) {
				handleLocoList(clientSocket);
			}
			else if (arguments["cmd"].compare("speed") == 0) {
				handleLocoSpeed(clientSocket, arguments);
			}
		}
		else if ((uri.compare("/favicon.ico") == 0) || (uri.substr(0, 5).compare("/css/") == 0)) {
			deliverFile(clientSocket, uri);
		}
		else {
			snprintf(buffer_out, sizeof(buffer_out), html_header_template, "404 Not found", "RailControl", "<p>The file can not be found</p>");
			send(clientSocket, buffer_out, strlen(buffer_out), 0);
		}
	}

	xlog("Terminating webclient");
	close(clientSocket);
}

int WebClient::stop() {
	// inform thread to stop
	run = false;
	return 0;
}

}; // namespace webserver
