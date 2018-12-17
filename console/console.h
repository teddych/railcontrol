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
			void accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state) override;
			void block(const controlType_t managerID, const blockID_t blockID, const lockState_t lockState) override;
			void handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state, const bool on) override;
			void locoIntoBlock(const locoID_t locoID, const blockID_t blockID) override;
			void locoRelease(const locoID_t locoID) override;
			void blockRelease(const blockID_t blockID) override;
			void streetRelease(const streetID_t streetID) override;
			void locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) override;
			void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) override;
			void locoStart(const locoID_t locoID) override;
			void locoStop(const locoID_t locoID) override;
		private:
			void Worker();
			void AddUpdate(const std::string& status);
			static void ReadBlanks(std::string& s, size_t& i);
			static char ReadCharacterWithoutEating(std::string& s, size_t& i);
			static char ReadCommand(std::string& s, size_t& i);
			static int ReadNumber(std::string& s, size_t& i);
			static bool ReadBool(std::string& s, size_t& i);
			static std::string ReadText(std::string& s, size_t& i);
			static hardwareType_t ReadHardwareType(std::string& s, size_t& i);
			static switchType_t ReadSwitchType(std::string& s, size_t& i);
			static layoutRotation_t ReadRotation(std::string& s, size_t& i);
			static direction_t ReadDirection(std::string& s, size_t& i);
			static accessoryState_t ReadAccessoryState(std::string& s, size_t& i);
			void HandleClient();
			void HandleCommand(std::string& s);
			void HandleAccessoryCommand(std::string& s, size_t& i);
			void HandleBlockCommand(std::string& s, size_t& i);
			void HandleControlCommand(std::string& s, size_t& i);
			void HandleFeedbackCommand(std::string& s, size_t& i);
			void HandleLocoCommand(std::string& s, size_t& i);
			void HandleStreetCommand(std::string& s, size_t& i);
			void HandleSwitchCommand(std::string& s, size_t& i);
			void HandleAccessoryDelete(std::string& s, size_t& i);
			void HandleAccessoryList(std::string& s, size_t& i);
			void HandleAccessoryNew(std::string& s, size_t& i);
			void HandleAccessorySwitch(std::string& s, size_t& i);
			void HandleBlockDelete(std::string& s, size_t& i);
			void HandleBlockList(std::string& s, size_t& i);
			void HandleBlockNew(std::string& s, size_t& i);
			void HandleBlockRelease(std::string& s, size_t& i);
			void HandleControlDelete(std::string& s, size_t& i);
			void HandleControlList(std::string& s, size_t& i);
			void HandleControlNew(std::string& s, size_t& i);
			void HandleFeedbackDelete(std::string& s, size_t& i);
			void HandleFeedbackList(std::string& s, size_t& i);
			void HandleFeedbackNew(std::string& s, size_t& i);
			void HandleFeedbackSet(std::string& s, size_t& i);
			void HandleFeedbackRelease(std::string& s, size_t& i);
			void HandleLocoAutomode(std::string& s, size_t& i);
			void HandleLocoBlock(std::string& s, size_t& i);
			void HandleLocoDelete(std::string& s, size_t& i);
			void HandleLocoList(std::string& s, size_t& i);
			void HandleLocoManualmode(std::string& s, size_t& i);
			void HandleLocoNew(std::string& s, size_t& i);
			void HandleLocoSpeed(std::string& s, size_t& i);
			void HandleLocoRelease(std::string& s, size_t& i);
			void HandleHelp();
			void HandlePrintLayout();
			void HandleQuit();
			void HandleShutdown();
			void HandleStreetDelete(std::string& s, size_t& i);
			void HandleStreetList(std::string& s, size_t& i);
			void HandleStreetNew(std::string& s, size_t& i);
			void HandleStreetRelease(std::string& s, size_t& i);
			void HandleSwitchDelete(std::string& s, size_t& i);
			void HandleSwitchList(std::string& s, size_t& i);
			void HandleSwitchNew(std::string& s, size_t& i);
			//void HandleSwitchRelease(std::string& s, size_t& i);

			unsigned short port;
			int serverSocket;
			int clientSocket;
			volatile unsigned char run;
			std::thread serverThread;
			Manager& manager;
	};

}; // namespace console

