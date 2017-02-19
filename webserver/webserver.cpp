#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "railcontrol.h"
#include "util.h"
#include "webclient.h"
#include "webserver.h"

using std::thread;
using std::string;
using std::vector;

namespace webserver {

WebServer::WebServer(Manager& manager, const unsigned short port) :
	port(port),
	serverSocket(0),
	run(false),
	lastClientID(0),
	manager(manager) {

	run = true;
	struct sockaddr_in6 server_addr;

	xlog("Starting webserver on port %i", port);

	// create server socket
	serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		xlog("Unable to create socket for webserver. Unable to serve clients.");
		run = false;
		return;
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

	if (bind(serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		xlog("Unable to bind socket for webserver to port %i. Unable to serve clients.", port);
		close(serverSocket);
		run = false;
		return;
	}

	// listen on the socket
	if (listen(serverSocket, 5) != 0) {
		xlog("Unable to listen on socket for client server on port %i. Unable to serve clients.", port);
		close(serverSocket);
		run = false;
		return;
	}

	// create seperate thread that handles the client requests
	serverThread = thread([this] { worker(); });
}

WebServer::~WebServer() {
	if (run) {
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
	}
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

void WebServer::go(const managerID_t managerID) {
	if (managerID != MANAGER_ID_WEBSERVER) {
		xlog("go not yet implemented in Webserver");
	}
}

void WebServer::stop(const managerID_t managerID) {
	if (managerID != MANAGER_ID_WEBSERVER) {
		xlog("stop not yet implemented in Webserver");
	}
}

void WebServer::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
	if (managerID != MANAGER_ID_WEBSERVER) {
		xlog("locoSpeed not yet implemented in Webserver");
	}
}

void WebServer::locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) {
	if (managerID != MANAGER_ID_WEBSERVER) {
		xlog("locoFunction not yet implemented in Webserver");
	}
}

}; // namespace webserver
