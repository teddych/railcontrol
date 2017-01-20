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

webserver_client::webserver_client(unsigned int id, int socket, webserver &webserver) :
	id(id),
	socket(socket),
	run(false),
	server(webserver),
	client_thread(thread([this] { worker(); })) {
}

webserver_client::~webserver_client() {
}

void webserver_client::getCommand(const string& str, string& method, string& uri, string& protocol) {
	vector<string> list;
	str_split(str, string(" "), list);
	if (list.size() == 3) {
		method = list[0];
		uri = list[1];
		protocol = list[2];
	}
}

// worker is the thread that handles client requests
void webserver_client::worker() {
	xlog("Executing webclient");
	run = true;

	char buffer_in[1024];
	char buffer_out[1024];

	recv(socket, buffer_in, sizeof(buffer_in), 0);
	string s(buffer_in);
	str_replace(s, string("\r\n"), string("\n"));
	str_replace(s, string("\r"), string("\n"));
	vector<string> lines;
	str_split(s, string("\n"), lines);

	if (lines.size() < 1) {
		xlog("Invalid request");
		close(socket);
		return;
	}
	string method;
	string uri;
	string protocol;
	getCommand(lines[0], method, uri, protocol);

	std::transform(method.begin(), method.end(), method.begin(), ::toupper);

	if (method.compare("GET") != 0) {
		xlog("Method not implemented");
		snprintf(buffer_out, sizeof(buffer_out), "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Method not implemented</h1></body></html>");
		send(socket, buffer_out, strlen(buffer_out), 0);
		close(socket);
		return;
	}

	/*
	for (auto line : lines) {
		xlog(line.c_str());
	}
	*/

	if (uri.compare("/") == 0) {
		snprintf(buffer_out, sizeof(buffer_out), "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>RailControl</h1><p>RailControl start page</p></body></html>");
		send(socket, buffer_out, strlen(buffer_out), 0);
	}
	else if (uri.compare("/quit/") == 0) {
		snprintf(buffer_out, sizeof(buffer_out), "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>RailControl</h1><p>RailControl stoppted</p></body></html>");
		send(socket, buffer_out, strlen(buffer_out), 0);
		stop_all();
	}
	else {
		snprintf(buffer_out, sizeof(buffer_out), "HTTP/1.0 404 Not found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>RailControl</h1><p>File %s not found</p></body></html>", uri.c_str());
		send(socket, buffer_out, strlen(buffer_out), 0);
	}

	xlog("Terminating webclient");
	close(socket);
}

int webserver_client::stop() {
	// inform thread to stop
	run = false;
	// join thread
	client_thread.join();
	return 0;
}


// Server part
// ***********

webserver::webserver(unsigned short port) :
	port(port),
	socket_server(0),
	run(false),
	last_client_id(0) {
}

webserver::~webserver() {
}


// worker is a seperate thread listening on the server socket
void webserver::worker() {
	fd_set set;
	struct timeval tv;
	struct sockaddr_in6 client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	while (run) {
		// wait for connection and abort on shutdown
		int ret;
		do {
			FD_ZERO(&set);
			FD_SET(socket_server, &set);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tv));
		} while (ret == 0 && run);
		if (ret > 0 && run) {
			// accept connection
			int socket_client = accept(socket_server, (struct sockaddr *) &client_addr, &client_addr_len);
			if (socket_client < 0) {
				xlog("Unable to accept client connection: %i, %i", socket_client, errno);
			}
			else {
				// create client and fill into vector
				clients.push_back(new webserver_client(++last_client_id, socket_client, *this));
			}
		}
	}
}

int webserver::start() {
	run = true;
	struct sockaddr_in6 server_addr;

	xlog("Starting webserver on port %i", port);

	// create server socket
	socket_server = socket(AF_INET6, SOCK_STREAM, 0);
	if (socket_server < 0) {
		xlog("Unable to create socket for webserver. Unable to serve clients.");
		return 1;
	}

	// bind socket to an address (in6addr_any)
	memset((char *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(port);

	int on = 1;
	if (setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on)) < 0) {
		xlog("Unable to set webserver socket option SO_REUSEADDR.");
	}

	while (run && bind(socket_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		xlog("Unable to bind socket for webserver to port %i. Retrying later.", port);
		sleep(1);
	}
	if (!run) {
		close(socket_server);
		return 1;
	}

	// listen on the socket
	if (listen(socket_server, 5) != 0) {
		xlog("Unable to listen on socket for client server on port %i. Unable to serve clients.", port);
		close(socket_server);
		return 1;
	}

	// create seperate thread that handles the client requests
	webserver_thread = thread([this] { worker(); });
	return 0;
}

int webserver::stop() {
	xlog("Stopping webserver");
	run = false;

	// stopping all clients
	for(auto client : clients) {
		client->stop();
	}

	// delete all client memory
	while (clients.size()) {
		webserver_client* client = clients.back();
		clients.pop_back();
		delete client;
	}

	// join server thread
	webserver_thread.join();
	return 0;
}
