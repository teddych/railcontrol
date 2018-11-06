#pragma once

#include <map>
#include <mutex>
#include <vector>

#include "command_interface.h"
#include "manager.h"
#include "network/TcpServer.h"

namespace webserver {

	class WebClient;

	class WebServer : public CommandInterface, private Network::TcpServer {
		public:
			WebServer(Manager& manager, const unsigned short port);
			~WebServer();

			void Work(Network::TcpConnection* connection) override;

			const std::string getName() const override { return "Webserver"; }
			bool nextUpdate(const unsigned int updateIDClient, std::string& s);
			void booster(const controlType_t managerID, const boosterStatus_t status) override;
			void locoSpeed(const controlType_t managerID, const locoID_t locoID, const speed_t speed) override;
			void locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
			void block(const controlType_t managerID, const blockID_t blockID, const lockState_t state) override;
			void handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state) override;
			void locoIntoBlock(const locoID_t locoID, const blockID_t blockID) override;
			void locoRelease(const locoID_t locoID) override;
			void blockRelease(const blockID_t blockID) override;
			void streetRelease(const streetID_t streetID) override;
			void locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) override;
			void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) override;
			void locoStart(const locoID_t locoID) override;
			void locoStop(const locoID_t locoID) override;

		private:
			void addUpdate(const std::string& command, const std::string& status);

			volatile bool run;
			unsigned int lastClientID;
			std::vector<WebClient*> clients;
			Manager& manager;

			std::map<unsigned int,std::string> updates;
			std::mutex updateMutex;
			unsigned int updateID;
			const unsigned int MaxUpdates = 10;
	};

}; // namespace webserver

