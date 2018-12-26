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
			bool nextUpdate(unsigned int& updateIDClient, std::string& s);
			void booster(const controlType_t managerID, const boosterStatus_t status) override;
			void locoSpeed(const controlType_t managerID, const locoID_t locoID, const LocoSpeed speed) override;
			void locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void accessorySettings(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t x, const layoutPosition_t y, const layoutPosition_t z) override;
			void accessoryDelete(const accessoryID_t accessoryID, const std::string& name) override;
			void feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
			void track(const controlType_t managerID, const trackID_t trackID, const lockState_t state) override;
			void handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state, const bool on) override;
			void switchSettings(const switchID_t switchID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const std::string& rotation);
			void switchDelete(const switchID_t switchID, const std::string& name) override;
			void locoIntoTrack(const locoID_t locoID, const trackID_t trackID) override;
			void locoRelease(const locoID_t locoID) override;
			void trackRelease(const trackID_t trackID) override;
			void trackSettings(const switchID_t switchID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const std::string& rotation);
			void streetRelease(const streetID_t streetID) override;
			void locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
			void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
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

