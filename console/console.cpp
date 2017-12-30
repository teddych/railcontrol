#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "datatypes.h"
#include "railcontrol.h"
#include "util.h"
#include "console.h"

using std::map;
using std::thread;
using std::string;
using std::stringstream;
using std::vector;

namespace console {

Console::Console(Manager& manager, const unsigned short port) :
	ManagerInterface(MANAGER_ID_CONSOLE),
	port(port),
	serverSocket(0),
	clientSocket(-1),
	run(false),
	manager(manager) {

	run = true;
	struct sockaddr_in6 server_addr;

	xlog("Starting console on port %i", port);

	// create server socket
	serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		xlog("Unable to create socket for console. Unable to serve clients.");
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
		xlog("Unable to set console socket option SO_REUSEADDR.");
	}

	if (bind(serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		xlog("Unable to bind socket for console to port %i. Unable to serve clients.", port);
		close(serverSocket);
		run = false;
		return;
	}

	// listen on the socket
	if (listen(serverSocket, 5) != 0) {
		xlog("Unable to listen on socket for console server on port %i. Unable to serve clients.", port);
		close(serverSocket);
		run = false;
		return;
	}

	// create seperate thread that handles the client requests
	serverThread = thread([this] { worker(); });
}

Console::~Console() {
	if (run) {
		xlog("Stopping console");
		run = false;

		// join server thread
		serverThread.join();
	}
}


// worker is a seperate thread listening on the server socket
void Console::worker() {
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
			clientSocket = accept(serverSocket, (struct sockaddr *) &client_addr, &client_addr_len);
			if (clientSocket < 0) {
				xlog("Unable to accept client connection for console: %i, %i", clientSocket, errno);
			}
			else {
				// handle client and fill into vector
				handleClient();
			}
		}
	}
}

void Console::readBlanks(string& s, size_t& i) {
	// read possible blanks
	while (s.length() > i) {
		unsigned char input = s[i];
		if (input != ' ') {
			break;
		}
		++i;
	}
}

int Console::readNumber(string& s, size_t& i) {
	int number = 0;
	while (s.length() > i) {
		unsigned char input = s[i];
		if ( input < '0' || input > '9') {
			break;
		}
		number *= 10;
		number += input - '0';
		++i;
	};
	return number;
}

void Console::handleClient() {
	addUpdate("Welcome to railcontrol console!\nType h for help\n");
	char buffer_in[1024];
	memset(buffer_in, 0, sizeof(buffer_in));

	while(run) {
		size_t pos = 0;
		string s;
		while(run && pos < sizeof(buffer_in) - 1 && (s.find("\n") == string::npos || s.find("\r") == string::npos)) {
			pos += recv_timeout(clientSocket, buffer_in + pos, sizeof(buffer_in) - 1 - pos, 0);
			s = string(buffer_in);
		}


		switch (s[0])
		{
			case 'a': // start automode
			{
				size_t i = 1;
				readBlanks(s, i);
				if (s[i] == 'a') {
					manager.locoStartAll();
					break;
				}
				locoID_t locoID = readNumber(s, i);
				manager.locoStart(locoID);
				break;
			}
			case 'l': // loco speed
			{
				size_t i = 1;
				readBlanks(s, i);
				locoID_t locoID = readNumber(s, i);
				readBlanks(s, i);
				speed_t speed = readNumber(s, i);
				manager.locoSpeed(MANAGER_ID_CONSOLE, locoID, speed);
				break;
			}
			case 'i': // loco into block
			{
				size_t i = 1;
				readBlanks(s, i);
				locoID_t locoID = readNumber(s, i);
				readBlanks(s, i);
				blockID_t blockID = readNumber(s, i);
				manager.locoIntoBlock(locoID, blockID);

				break;
			}
			case 'f': // feedback
			{
				size_t i = 1;
				readBlanks(s, i);
				feedbackID_t feedbackID = readNumber(s, i);
				readBlanks(s, i);
				// read state
				unsigned char input = s[i];
				feedbackState_t state = FEEDBACK_STATE_FREE;
				if (input == 'X' || input == 'x') {
					state = FEEDBACK_STATE_OCCUPIED;
				}
				manager.feedback(MANAGER_ID_CONSOLE, feedbackID, state);
				break;
			}
			case 'h': // help
			{
				string status("Available console commands:\n"
				"a loco#        Start loco into automode\n"
				"f pin# [X]     Turn feedback on (with X) or of (without X)\n"
				"h              Show this help\n"
				"i loco# block# Set loco into block\n"
				"l loco# speed  Set loco speed between 0 and 1024\n"
				"m loco#        Stop loco and got to manual mode\n"
				"q              Quit\n");
				addUpdate(status);
				break;
			}
			case 'm': // start manual mode and leave automode
			{
				size_t i = 1;
				readBlanks(s, i);
				if (s[i] == 'a') {
					manager.locoStopAll();
					break;
				}
				locoID_t locoID = readNumber(s, i);
				manager.locoStop(locoID);
				break;
			}
			case 'q': // quit
			{
				string status("Quit railcontrol console\n");
				addUpdate(status);
				close(clientSocket);
				return;
			}
			default: {
				string status("Unknown command\n");
				addUpdate(status);
			}
		}
	}
}

void Console::addUpdate(const string& status) {
	if (clientSocket < 0) {
		return;
	}
	string s(status);
	s += '\n';
	send_timeout(clientSocket, s.c_str(), s.length(), 0);
}

void Console::booster(const managerID_t managerID, const boosterStatus_t status) {
	if (status) {
		addUpdate("Booster is on\n");
	}
	else {
		addUpdate("Booster is off\n");
	}
}

void Console::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " speed is " << speed << "\n";
	addUpdate(status.str());
}

void Console::locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) {
	std::stringstream status;
	const char* directionText = (direction ? "forward" : "reverse");
	status << manager.getLocoName(locoID) << " direction is " << directionText << "\n";
	addUpdate(status.str());
}

void Console::locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool state) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " f" << (unsigned int)function << " is " << (state ? "on" : "off") << "\n";
	addUpdate(status.str());
}

void Console::accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) {
	std::stringstream status;
	unsigned char color;
	unsigned char on;
	char* colorText;
	char* stateText;
	datamodel::Accessory::getAccessoryTexts(state, color, on, colorText, stateText);
	status << manager.getAccessoryName(accessoryID) << " " << colorText << " is " << stateText << "\n";
	addUpdate(status.str());
}

void Console::feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) {
	std::stringstream status;
	status << "Feedback " << pin << " is " << (state ? "on" : "off") << "\n";
	addUpdate(status.str());
}

void Console::block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) {
	std::stringstream status;
	char* stateText;
	datamodel::Block::getTexts(state, stateText);
	status << manager.getBlockName(blockID) << " is " << stateText << "\n";
	addUpdate(status.str());
}

void Console::handleSwitch(const managerID_t managerID, const switchID_t switchID, const switchState_t state) {
	std::stringstream status;
	char* stateText;
	datamodel::Switch::getTexts(state, stateText);
	status << manager.getSwitchName(switchID) << " is " << stateText << "\n";
	addUpdate(status.str());
}

void Console::locoIntoBlock(const locoID_t locoID, const blockID_t blockID) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " is in block " << manager.getBlockName(blockID);
	addUpdate(status.str());
}

void Console::locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " runs on street " << manager.getStreetName(streetID) << " with destination block " << manager.getBlockName(blockID) << "\n";
	addUpdate(status.str());
}

void Console::locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " has reached the destination block " << manager.getBlockName(blockID) << " on street " << manager.getStreetName(streetID) << "\n";
	addUpdate(status.str());
}

void Console::locoStart(const locoID_t locoID) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " is in auto mode\n";
	addUpdate(status.str());
}

void Console::locoStop(const locoID_t locoID) {
	std::stringstream status;
	status << manager.getLocoName(locoID) << " is in manual mode\n";
	addUpdate(status.str());
}

}; // namespace console
