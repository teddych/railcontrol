#pragma once

#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "console/ConsoleServer.h"
#include "command_interface.h"
#include "Logger/Logger.h"
#include "manager.h"
#include "network/TcpServer.h"

namespace console
{
	class ConsoleClient
	{
		public:
			ConsoleClient(Network::TcpConnection* connection, ConsoleServer &consoleServer, Manager& manager)
			:	logger(Logger::Logger::GetLogger("Console")),
				connection(connection),
				run(false),
				server(consoleServer),
				clientThread(std::thread([this] {Worker();})),
				manager(manager)
			{}

			~ConsoleClient()
			{
				run = false;
				clientThread.join();
				connection->Terminate();
			}

			void SendAndPrompt(const std::string s);

		private:
			void Worker();
			void WorkerImpl();
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
			void HandleTrackCommand(std::string& s, size_t& i);
			void HandleControlCommand(std::string& s, size_t& i);
			void HandleFeedbackCommand(std::string& s, size_t& i);
			void HandleLocoCommand(std::string& s, size_t& i);
			void HandleStreetCommand(std::string& s, size_t& i);
			void HandleSwitchCommand(std::string& s, size_t& i);
			void HandleAccessoryDelete(std::string& s, size_t& i);
			void HandleAccessoryList(std::string& s, size_t& i);
			void HandleAccessoryNew(std::string& s, size_t& i);
			void HandleAccessorySwitch(std::string& s, size_t& i);
			void HandleTrackDelete(std::string& s, size_t& i);
			void HandleTrackList(std::string& s, size_t& i);
			void HandleTrackNew(std::string& s, size_t& i);
			void HandleTrackRelease(std::string& s, size_t& i);
			void HandleControlDelete(std::string& s, size_t& i);
			void HandleControlList(std::string& s, size_t& i);
			void HandleControlNew(std::string& s, size_t& i);
			void HandleFeedbackDelete(std::string& s, size_t& i);
			void HandleFeedbackList(std::string& s, size_t& i);
			void HandleFeedbackNew(std::string& s, size_t& i);
			void HandleFeedbackSet(std::string& s, size_t& i);
			void HandleFeedbackRelease(std::string& s, size_t& i);
			void HandleLocoAutomode(std::string& s, size_t& i);
			void HandleLocoTrack(std::string& s, size_t& i);
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

			Logger::Logger* logger;
			Network::TcpConnection* connection;
			volatile unsigned char run;
			ConsoleServer& server;
			std::thread clientThread;
			Manager& manager;
	};
}; // namespace console

