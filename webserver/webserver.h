#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <thread>
#include <vector>

#include "manager_interface.h"
#include "manager.h"

namespace webserver {

class WebClient;

class WebServer : public ManagerInterface {
	public:
		WebServer(Manager& manager, const unsigned short port);
		~WebServer();
		void go(const managerID_t managerID) override;
		void stop(const managerID_t managerID) override;
		void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) override;
		void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
		void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
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

