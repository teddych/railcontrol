#pragma once

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "../command_interface.h"
#include "manager.h"

namespace console {

	class Console : public CommandInterface {
		public:
			Console(Manager& manager, const unsigned short port);
			~Console();
			const std::string getName() const override { return "Console"; }
			void booster(const controlType_t managerID, const boosterStatus_t status) override;
			void locoSpeed(const controlType_t managerID, const locoID_t locoID, const speed_t speed) override;
			void locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction) override;
			void locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool on) override;
			void accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) override;
			void feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
			void block(const controlType_t managerID, const blockID_t blockID, const lockState_t lockState) override;
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
			void worker();
			void handleClient();
			void addUpdate(const std::string& status);
			static void readBlanks(std::string& s, size_t& i);
			static int readNumber(std::string& s, size_t& i);
			static bool readBool(std::string& s, size_t& i);
			static std::string readText(std::string& s, size_t& i);
			static switchType_t readSwitchType(std::string& s, size_t& i);
			static layoutRotation_t readRotation(std::string& s, size_t& i);
			static direction_t readDirection(std::string& s, size_t& i);

			unsigned short port;
			int serverSocket;
			int clientSocket;
			volatile unsigned char run;
			std::thread serverThread;
			Manager& manager;
	};

}; // namespace console

