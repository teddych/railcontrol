#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "datatypes.h"
#include "railcontrol.h"
#include "text/converters.h"
#include "util.h"
#include "webserver/webclient.h"
#include "webserver/webserver.h"

using std::map;
using std::thread;
using std::string;
using std::stringstream;
using std::vector;

namespace webserver {

	WebServer::WebServer(Manager& manager, const unsigned short port) :
		CommandInterface(ControlTypeWebserver),
		Network::TcpServer(port),
		run(false),
		lastClientID(0),
		manager(manager),
		updateID(1)
	{

		updates[updateID] = "data: status=Railcontrol started";

		run = true;
	}

	WebServer::~WebServer() {
		if (run == false)
		{
			return;
		}
		xlog("Stopping webserver");
		{
			std::lock_guard<std::mutex> lock(updateMutex);
			updates[++updateID] = "data: status=Stopping Railcontrol";
		}
		sleep(1);
		run = false;

		// stopping all clients
		for(auto client : clients)
		{
			client->stop();
		}

		// delete all client memory
		while (clients.size())
		{
			WebClient* client = clients.back();
			clients.pop_back();
			delete client;
		}
	}

	void WebServer::Work(Network::TcpConnection* connection)
	{
		clients.push_back(new WebClient(++lastClientID, connection, *this, manager));
	}

	void WebServer::booster(const controlType_t managerID, const boosterStatus_t status)
	{
		if (status)
		{
			addUpdate("booster;on=true", "Booster is on");
		}
		else
		{
			addUpdate("booster;on=false", "Booster is off");
		}
	}

	void WebServer::locoSpeed(const controlType_t managerID, const locoID_t locoID, const speed_t speed) {
		stringstream command;
		stringstream status;
		command << "locospeed;loco=" << locoID << ";speed=" << speed;
		status << manager.getLocoName(locoID) << " speed is " << speed;
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction)
	{
		stringstream command;
		stringstream status;
		command << "locodirection;loco=" << locoID << ";direction=" << (direction ? "true" : "false");
		status << manager.getLocoName(locoID) << " direction is " << (direction ? "right" : "left");
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool state) {
		stringstream command;
		stringstream status;
		command << "locofunction;loco=" << locoID << ";function=" << (unsigned int)function << ";on=" << (state ? "true" : "false");
		status << manager.getLocoName(locoID) << " f" << (unsigned int)function << " is " << (state ? "on" : "off");
		addUpdate(command.str(), status.str());
	}

	void WebServer::accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on)
	{
		if (on == false)
		{
			return;
		}
		stringstream command;
		stringstream status;
		string stateText;
		text::Converters::accessoryStatus(state, stateText);
		command << "accessory;accessory=" << accessoryID << ";state=" << stateText;
		status << manager.getAccessoryName(accessoryID) << " is " << stateText;
		addUpdate(command.str(), status.str());
	}

	void WebServer::accessorySettings(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ)
	{
		stringstream command;
		stringstream status;
		command << "accessory;accessory=" << accessoryID << ";posx=" << static_cast<int>(posX) << ";posy=" << static_cast<int>(posY) << ";posz=" << static_cast<int>(posZ);
		status << name << " has new position: " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ);
		addUpdate(command.str(), status.str());
	}

	void WebServer::feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state) {
		stringstream command;
		stringstream status;
		command << "feedback;pin=" << pin << ";state=" << (state ? "on" : "off");
		status << "Feedback " << pin << " is " << (state ? "on" : "off");
		addUpdate(command.str(), status.str());
	}

	void WebServer::block(const controlType_t managerID, const blockID_t blockID, const lockState_t state) {
		stringstream command;
		stringstream status;
		string stateText;
		text::Converters::lockStatus(state, stateText);
		command << "block;block=" << blockID << ";state=" << stateText;
		status << manager.getBlockName(blockID) << " is " << stateText;
		addUpdate(command.str(), status.str());
	}

	void WebServer::handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state) {
		stringstream command;
		stringstream status;
		string stateText;
		text::Converters::switchStatus(state, stateText);
		command << "switch;switch=" << switchID << ";state=" << stateText;
		status << manager.getSwitchName(switchID) << " is " << stateText;
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoIntoBlock(const locoID_t locoID, const blockID_t blockID) {
		stringstream command;
		stringstream status;
		command << "locoIntoBlock;loco=" << locoID << ";block=" << blockID;
		status << manager.getLocoName(locoID) << " is in block " << manager.getBlockName(blockID);
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoRelease(const locoID_t locoID) {
		stringstream command;
		stringstream status;
		command << "locoRelease;loco=" << locoID;
		status << manager.getLocoName(locoID) << " is not in a block anymore";
		addUpdate(command.str(), status.str());
	};

	void WebServer::blockRelease(const blockID_t blockID) {
		stringstream command;
		stringstream status;
		command << "blockRelease;block=" << blockID;
		status << manager.getBlockName(blockID) << " is released";
		addUpdate(command.str(), status.str());
	};

	void WebServer::streetRelease(const streetID_t streetID) {
		stringstream command;
		stringstream status;
		command << "streetRelease;street=" << streetID;
		status << manager.getStreetName(streetID) << " is  released";
		addUpdate(command.str(), status.str());
	};


	void WebServer::locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) {
		stringstream command;
		stringstream status;
		command << "locoStreet;loco=" << locoID << ";street=" << streetID << ";block=" << blockID;
		status << manager.getLocoName(locoID) << " runs on street " << manager.getStreetName(streetID) << " with destination block " << manager.getBlockName(blockID);
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) {
		stringstream command;
		stringstream status;
		command << "locoDestinationReached;loco=" << locoID << ";street=" << streetID << ";block=" << blockID;
		status << manager.getLocoName(locoID) << " has reached the destination block " << manager.getBlockName(blockID) << " on street " << manager.getStreetName(streetID);
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoStart(const locoID_t locoID) {
		stringstream command;
		stringstream status;
		command << "locoStart;loco=" << locoID;
		status << manager.getLocoName(locoID) << " is in auto mode";
		addUpdate(command.str(), status.str());
	}

	void WebServer::locoStop(const locoID_t locoID) {
		stringstream command;
		stringstream status;
		command << "locoStop;loco=" << locoID;
		status << manager.getLocoName(locoID) << " is in manual mode";
		addUpdate(command.str(), status.str());
	}

	void WebServer::addUpdate(const string& command, const string& status) {
		stringstream ss;
		ss << "data: command=" << command << ";status=" << status << "\r\n\r\n";
		std::lock_guard<std::mutex> lock(updateMutex);
		updates[++updateID] = ss.str();
		updates.erase(updateID - MaxUpdates);
	}

	bool WebServer::nextUpdate(const unsigned int updateIDClient, string& s)
	{
		std::lock_guard<std::mutex> lock(updateMutex);

		unsigned int internalID = updateIDClient;
		if (internalID + MaxUpdates < updateID)
		{
			internalID = updateID - MaxUpdates;
		}

		if(updates.count(internalID) == 1)
		{
			s = updates.at(internalID);
			return true;
		}

		return false;
	}

}; // namespace webserver
