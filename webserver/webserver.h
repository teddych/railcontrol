#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <map>
#include <mutex>
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
			bool nextUpdate(unsigned int& updateID, std::string& s);
			void booster(const managerID_t managerID, const boosterStatus_t status) override;
			void locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) override;
			void locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) override;
			void feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
			void block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) override;
			void handleSwitch(const managerID_t managerID, const switchID_t switchID, const switchState_t state) override;
		private:
			void worker();
			void addUpdate(const std::string& command, const std::string& status);
			unsigned short port;
			int serverSocket;
			volatile unsigned char run;
			unsigned int lastClientID;
			std::thread serverThread;
			std::vector<WebClient*> clients;
			Manager& manager;

			std::map<unsigned int,std::string> updates;
			std::mutex updateMutex;
			unsigned int updateID;
	};

}; // namespace webserver

#endif // WEBSERVER_WEBSERVER_H

