#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "railcontrol.h"
#include "util.h"
#include "webserver.h"

using std::thread;
using std::string;
using std::vector;


// Client part
// ***********

WebClient::WebClient(const unsigned int id, int socket, WebServer& webserver, Manager& m) :
	id(id),
	clientSocket(socket),
	run(false),
	server(webserver),
	clientThread(thread([this] { worker(); })),
	manager(m) {
}

WebClient::~WebClient() {
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

void WebClient::handleLocoList(const int socket, const vector<string>& uri_parts) {
	char buffer_out[1024];
	snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>List of locos</p>");
	send(socket, buffer_out, strlen(buffer_out), 0);
}

void WebClient::handleLocoProperties(const int socket, const vector<string>& uri_parts) {
	unsigned int loco_id = std::stoi(uri_parts[2]);
	char buffer[1024];
	snprintf(buffer, sizeof(buffer), "<p>Properties of loco %u</p>", loco_id);
	char buffer_out[1024];
	snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", buffer);
	send(socket, buffer_out, strlen(buffer_out), 0);
}

void WebClient::handleLocoCommand(const int socket, const vector<string>& uri_parts) {
	unsigned int locoID = std::stoi(uri_parts[2]);
	char buffer[1024];
	if (uri_parts[3].compare("speed") == 0) {
		int speed = std::stoi(uri_parts[4]);
		snprintf(buffer, sizeof(buffer), "<p>loco %u speed is now set to %i</p>", locoID, speed);
		manager.locoSpeed(CONTROL_ID_WEBSERVER, locoID, speed);
	}
	else if (uri_parts[3].substr(0, 1).compare("f") == 0) {
		unsigned char fx = std::stoi(uri_parts[3].substr(1));
		bool fon = (uri_parts[4].compare("on") == 0);
		snprintf(buffer, sizeof(buffer), "<p>loco %u f%i is now set to %s</p>", locoID, fx, (fon ? "on" : "off"));
	}
	char buffer_out[1024];
	snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", buffer);
	send(socket, buffer_out, strlen(buffer_out), 0);
}

void WebClient::handleLoco(const int socket, const string& uri) {
	vector<string> uri_parts;
	str_split(uri, "/", uri_parts);
	if (uri_parts.size() == 3) {
		handleLocoList(socket, uri_parts);
	}
	else if (uri_parts.size() == 4) {
		handleLocoProperties(socket, uri_parts);
	}
	else if (uri_parts.size() == 6) {
		handleLocoCommand(socket, uri_parts);
	}
	else {
		char buffer_out[1024];
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "404 Not found", "RailControl", "<p>Unknown loco command</p>");
		send(socket, buffer_out, strlen(buffer_out), 0);
	}
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

	/*
		 for (auto line : lines) {
		 xlog(line.c_str());
		 }
	 */

	if (uri.compare("/") == 0) {
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>Railcontrol is running</p>");
		send(clientSocket, buffer_out, strlen(buffer_out), 0);
	}
	else if (uri.compare("/quit/") == 0) {
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "200 OK", "RailControl", "<p>Railcontrol is shutting down</p>");
		send(clientSocket, buffer_out, strlen(buffer_out), 0);
		stopRailControl();
	}
	else if ((uri.compare("/favicon.ico") == 0) || (uri.substr(0, 5).compare("/css/") == 0)) {
		deliverFile(clientSocket, uri);
	}
	else if (uri.substr(0, 6).compare("/loco/") == 0) {
		handleLoco(clientSocket, uri);
	}
	else {
		snprintf(buffer_out, sizeof(buffer_out), html_header_template, "404 Not found", "RailControl", "<p>The file can not be found</p>");
		send(clientSocket, buffer_out, strlen(buffer_out), 0);
	}

	xlog("Terminating webclient");
	close(clientSocket);
}

int WebClient::stop() {
	// inform thread to stop
	run = false;
	// join thread
	clientThread.join();
	return 0;
}


// Server part
// ***********

WebServer::WebServer(Manager& m, const unsigned short port) :
  Control(CONTROL_ID_WEBSERVER),
	port(port),
	serverSocket(0),
	run(false),
	lastClientID(0),
	manager(m) {

  start();
}

WebServer::~WebServer() {
  stop();
}


// worker is a seperate thread listening on the server socket
void WebServer::worker() {
	fd_set set;
	struct timeval tv;
	struct sockaddr_in6 client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	while (run) {
		// wait for connection and abort on shutdown
		int ret;
		do {
			FD_ZERO(&set);
			FD_SET(serverSocket, &set);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tv));
		} while (ret == 0 && run);
		if (ret > 0 && run) {
			// accept connection
			int socket_client = accept(serverSocket, (struct sockaddr *) &client_addr, &client_addr_len);
			if (socket_client < 0) {
				xlog("Unable to accept client connection: %i, %i", socket_client, errno);
			}
			else {
				// create client and fill into vector
				clients.push_back(new WebClient(++lastClientID, socket_client, *this, manager));
			}
		}
	}
}

int WebServer::start() {
	run = true;
	struct sockaddr_in6 server_addr;

	xlog("Starting webserver on port %i", port);

	// create server socket
	serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		xlog("Unable to create socket for webserver. Unable to serve clients.");
		return 1;
	}

	// bind socket to an address (in6addr_any)
	memset((char *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(port);

	int on = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on)) < 0) {
		xlog("Unable to set webserver socket option SO_REUSEADDR.");
	}

	while (run && bind(serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		xlog("Unable to bind socket for webserver to port %i. Retrying later.", port);
		sleep(1);
	}
	if (!run) {
		close(serverSocket);
		return 1;
	}

	// listen on the socket
	if (listen(serverSocket, 5) != 0) {
		xlog("Unable to listen on socket for client server on port %i. Unable to serve clients.", port);
		close(serverSocket);
		return 1;
	}

	// create seperate thread that handles the client requests
	serverThread = thread([this] { worker(); });
	return 0;
}

int WebServer::stop() {
	xlog("Stopping webserver");
	run = false;

	// stopping all clients
	for(auto client : clients) {
		client->stop();
	}

	// delete all client memory
	while (clients.size()) {
		WebClient* client = clients.back();
		clients.pop_back();
		delete client;
	}

	// join server thread
	serverThread.join();
	return 0;
}
