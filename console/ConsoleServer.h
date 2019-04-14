#pragma once

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "ControlInterface.h"
#include "Logger/Logger.h"
#include "manager.h"
#include "network/TcpServer.h"

namespace console
{
	class ConsoleClient;

	class ConsoleServer : public ControlInterface, private Network::TcpServer
	{
		public:
			ConsoleServer(Manager& manager, const unsigned short port);
			~ConsoleServer();

			void Work(Network::TcpConnection* connection) override;

			const std::string GetName() const override { return "Console"; }
			void Booster(const controlType_t controlType, const boosterState_t status) override;
			void LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed) override;
			void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction) override;
			void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on) override;
			void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void FeedbackState(const std::string& name, const feedbackID_t feedbackID, const feedbackState_t state) override;
			void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on) override;
			void LocoIntoTrack(const locoID_t locoID, const trackID_t trackID) override;
			void LocoRelease(const locoID_t locoID) override;
			void StreetRelease(const streetID_t streetID) override;
			void LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
			void LocoStart(const locoID_t locoID) override;
			void LocoStop(const locoID_t locoID) override;

		private:
			void AddUpdate(const std::string& status);

			Logger::Logger* logger;
			volatile unsigned char run;
			std::vector<ConsoleClient*> clients;
			Manager& manager;
	};
}; // namespace console

