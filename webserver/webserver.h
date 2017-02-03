#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <thread>
#include <vector>

#include "../control.h"
#include "../manager.h"

namespace webserver {

class WebClient;

class WebServer : public Control {
	public:
		WebServer(Manager& manager, const unsigned short port);
		~WebServer();
		void go(const controlID_t controlID) override;
		void stop(const controlID_t controlID) override;
		void locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) override;
	private:
	  void worker();
		unsigned short port;
		int serverSocket;
		volatile unsigned char run;
		unsigned int lastClientID;
		std::thread serverThread;
		std::vector<WebClient*> clients;
		Manager& manager;
};

}; // namespace webserver

#endif // WEBSERVER_WEBSERVER_H

