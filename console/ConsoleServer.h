#pragma once

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "command_interface.h"
#include "Logger/Logger.h"
#include "manager.h"
#include "network/TcpServer.h"

namespace console
{
	class ConsoleClient;

	class ConsoleServer : public CommandInterface, private Network::TcpServer
	{
		public:
			ConsoleServer(Manager& manager, const unsigned short port);
			~ConsoleServer();

			void Work(Network::TcpConnection* connection) override;

			const std::string getName() const override { return "Console"; }
			void booster(const controlType_t controlType, const boosterStatus_t status) override;
			void locoSpeed(const controlType_t controlType, const locoID_t locoID, const LocoSpeed speed) override;
			void locoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void FeedbackStatus(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state) override;
			void track(const controlType_t controlType, const trackID_t trackID, const lockState_t lockState) override;
			void handleSwitch(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on) override;
			void locoIntoTrack(const locoID_t locoID, const trackID_t trackID) override;
			void locoRelease(const locoID_t locoID) override;
			void trackRelease(const trackID_t trackID) override;
			void streetRelease(const streetID_t streetID) override;
			void locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
			void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
			void locoStart(const locoID_t locoID) override;
			void locoStop(const locoID_t locoID) override;

		private:
			void AddUpdate(const std::string& status);

			Logger::Logger* logger;
			volatile unsigned char run;
			std::vector<ConsoleClient*> clients;
			Manager& manager;
	};
}; // namespace console

