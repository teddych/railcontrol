#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "console/ConsoleClient.h"
#include "console/ConsoleServer.h"
#include "datatypes.h"
#include "railcontrol.h"
#include "text/converters.h"

using std::map;
using std::thread;
using std::to_string;
using std::string;
using std::stringstream;
using std::vector;

namespace console
{

	ConsoleServer::ConsoleServer(Manager& manager, const unsigned short port)
	:	CommandInterface(ControlTypeConsole),
		Network::TcpServer(port),
		logger(Logger::Logger::GetLogger("Console")),
		run(true),
		manager(manager)
	{
		logger->Info("Starting server");
	}

	ConsoleServer::~ConsoleServer()
	{
		if (run == false)
		{
			return;
		}
		logger->Info("Stopping server");

		run = false;

		// delete all client memory
		while (clients.size())
		{
			ConsoleClient* client = clients.back();
			clients.pop_back();
			delete client;
		}
	}

	void ConsoleServer::Work(Network::TcpConnection* connection)
	{
		clients.push_back(new ConsoleClient(connection, *this, manager));
	}

	void ConsoleServer::AddUpdate(const string& status)
	{
		for (auto client : clients)
		{
			client->SendAndPrompt(status);
		}
	}

	void ConsoleServer::Booster(const controlType_t controlType, const boosterState_t status)
	{
		if (status)
		{
			AddUpdate("Booster is on");
		}
		else
		{
			AddUpdate("Booster is off");
		}
	}

	void ConsoleServer::LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " speed is " << speed;
		AddUpdate(status.str());
	}

	void ConsoleServer::LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction)
	{
		std::stringstream status;
		const char* directionText = (direction ? "forward" : "reverse");
		status << manager.getLocoName(locoID) << " direction is " << directionText;
		AddUpdate(status.str());
	}

	void ConsoleServer::LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool state)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " f" << (unsigned int)function << " is " << (state ? "on" : "off");
		AddUpdate(status.str());
	}

	void ConsoleServer::AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on)
	{
		if (on == false)
		{
			return;
		}
		std::stringstream status;
		string stateText;
		text::Converters::accessoryStatus(state, stateText);
		status << manager.getAccessoryName(accessoryID)  << " is " << stateText;
		AddUpdate(status.str());
	}

	void ConsoleServer::FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state)
	{
		std::stringstream status;
		status << "Feedback " << feedbackID << " is " << (state ? "on" : "off");
		AddUpdate(status.str());
	}

	void ConsoleServer::track(const controlType_t controlType, const trackID_t trackID, const lockState_t lockState)
	{
		std::stringstream status;
		string stateText;
		text::Converters::lockStatus(lockState, stateText);
		status << manager.getTrackName(trackID) << " is " << stateText;
		AddUpdate(status.str());
	}

	void ConsoleServer::SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on)
	{
		if (on == false)
		{
			return;
		}
		std::stringstream status;
		string stateText;
		text::Converters::switchStatus(state, stateText);
		status << manager.getSwitchName(switchID) << " is " << stateText;
		AddUpdate(status.str());
	}

	void ConsoleServer::locoIntoTrack(const locoID_t locoID, const trackID_t trackID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " is in track " << manager.getTrackName(trackID);
		AddUpdate(status.str());
	}

	void ConsoleServer::locoRelease(const locoID_t locoID)
	{
		stringstream status;
		status << manager.getLocoName(locoID) << " is not in a track anymore";
		AddUpdate(status.str());
	};

	void ConsoleServer::trackRelease(const trackID_t trackID)
	{
		stringstream status;
		status << manager.getTrackName(trackID) << " is released";
		AddUpdate(status.str());
	};

	void ConsoleServer::streetRelease(const streetID_t streetID)
	{
		stringstream status;
		status << manager.getStreetName(streetID) << " is  released";
		AddUpdate(status.str());
	};

	void ConsoleServer::locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " runs on street " << manager.getStreetName(streetID) << " with destination track " << manager.getTrackName(trackID);
		AddUpdate(status.str());
	}

	void ConsoleServer::locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " has reached the destination track " << manager.getTrackName(trackID) << " on street " << manager.getStreetName(streetID);
		AddUpdate(status.str());
	}

	void ConsoleServer::locoStart(const locoID_t locoID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " is in auto mode";
		AddUpdate(status.str());
	}

	void ConsoleServer::locoStop(const locoID_t locoID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " is in manual mode";
		AddUpdate(status.str());
	}

}; // namespace console
