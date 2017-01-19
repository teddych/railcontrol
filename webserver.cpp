#include <cstring>		//memset
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"
#include "webserver.h"

using std::thread;


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

void webserver_client::worker() {
	xlog("Executing new client");
	run = true;
	do {
		sleep(1);
	} while(run);
	xlog("Terminating client");
	close(socket);
}

int webserver_client::stop() {
	run = false;
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
			// accept conneciton
			int socket_client = accept(socket_server, (struct sockaddr *) &client_addr, &client_addr_len);
			if (socket_client < 0) {
				xlog("Unable to accept client connection: %i, %i", socket_client, errno);
			}
			else {
				// do work
				xlog("Webserver client connected");
				clients.push_back(new webserver_client(++last_client_id, socket_client, *this));
			}
		}
	}
}

int webserver::start() {
	run = true;
	struct sockaddr_in6 server_addr;

	xlog("Starting webserver on port %i", port);
	socket_server = socket(AF_INET6, SOCK_STREAM, 0);
	if (socket_server < 0) {
		xlog("Unable to create socket for webserver. Unable to serve clients.");
		return 1;
	}
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
	if (listen(socket_server, 5) != 0) {
		xlog("Unable to listen on socket for client server on port %i. Unable to serve clients.", port);
		close(socket_server);
		return 1;
	}
	webserver_thread = thread([this] { worker(); });
	xlog("webserver is up and listening on port %i", port);
	return 0;
}

int webserver::stop() {
	xlog("Stopping webserver");
	run = false;

	// stopping all clients
	for(auto client : clients) {
		client->stop();
	}

	webserver_thread.join();
	return 0;
}
