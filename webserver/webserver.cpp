#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "datatypes.h"
#include "Logger/Logger.h"
#include "railcontrol.h"
#include "text/converters.h"
#include "webserver/webclient.h"
#include "webserver/webserver.h"

using std::map;
using std::thread;
using std::string;
using std::stringstream;
using std::vector;

namespace webserver {

	WebServer::WebServer(Manager& manager, const unsigned short port)
	:	ControlInterface(ControlTypeWebserver),
		Network::TcpServer(port),
		run(false),
		lastClientID(0),
		manager(manager),
		updateID(1)
	{
		Logger::Logger::GetLogger("Webserver")->Info("Starting server");
		updates[updateID] = "data: status=Railcontrol started";
		run = true;
	}

	WebServer::~WebServer()
	{
		if (run == false)
		{
			return;
		}
		Logger::Logger::GetLogger("Webserver")->Info("Stopping server");
		{
			std::lock_guard<std::mutex> lock(updateMutex);
			updates[++updateID] = "data: status=Stopping Railcontrol";
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		run = false;

		// stopping all clients
		for (auto client : clients)
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

	void WebServer::Booster(const controlType_t controlType, const boosterState_t status)
	{
		if (status)
		{
			AddUpdate("booster;on=true", "Booster is on");
		}
		else
		{
			AddUpdate("booster;on=false", "Booster is off");
		}
	}

	void WebServer::LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed)
	{
		stringstream command;
		stringstream status;
		command << "locospeed;loco=" << locoID << ";speed=" << speed;
		status << manager.LocoName(locoID) << " speed is " << speed;
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction)
	{
		stringstream command;
		stringstream status;
		command << "locodirection;loco=" << locoID << ";direction=" << (direction ? "true" : "false");
		status << manager.LocoName(locoID) << " direction is " << (direction ? "right" : "left");
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool state)
	{
		stringstream command;
		stringstream status;
		command << "locofunction;loco=" << locoID << ";function=" << (unsigned int) function << ";on=" << (state ? "true" : "false");
		status << manager.LocoName(locoID) << " f" << (unsigned int) function << " is " << (state ? "on" : "off");
		AddUpdate(command.str(), status.str());
	}

	void WebServer::AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on)
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
		AddUpdate(command.str(), status.str());
	}

	void WebServer::AccessorySettings(const accessoryID_t accessoryID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "accessorysettings;accessory=" << accessoryID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::AccessoryDelete(const accessoryID_t accessoryID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "accessorydelete;accessory=" << accessoryID;
		status << name << " deleted";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state)
	{
		stringstream command;
		stringstream status;
		command << "feedback;feedback=" << feedbackID << ";state=" << (state ? "on" : "off");
		status << "Feedback " << feedbackID << " is " << (state ? "on" : "off");
		AddUpdate(command.str(), status.str());
	}

	void WebServer::FeedbackSettings(const feedbackID_t feedbackID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "feedbacksettings;feedback=" << feedbackID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::FeedbackDelete(const feedbackID_t feedbackID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "feedbackdelete;feedback=" << feedbackID;
		status << name << " deleted";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::TrackState(const controlType_t controlType, const trackID_t trackID, const lockState_t state)
	{
		stringstream command;
		stringstream status;
		string stateText;
		text::Converters::lockStatus(state, stateText);
		command << "track;track=" << trackID << ";state=" << stateText;
		status << manager.getTrackName(trackID) << " is " << stateText;
		AddUpdate(command.str(), status.str());
	}

	void WebServer::StreetSettings(const streetID_t streetID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "streetsettings;street=" << streetID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::StreetDelete(const streetID_t streetID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "streetdelete;street=" << streetID;
		status << name << " deleted";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on)
	{
		if (on == false)
		{
			return;
		}
		stringstream command;
		stringstream status;
		string stateText;
		text::Converters::switchStatus(state, stateText);
		command << "switch;switch=" << switchID << ";state=" << stateText;
		status << manager.getSwitchName(switchID) << " is " << stateText;
		AddUpdate(command.str(), status.str());
	}

	void WebServer::SwitchSettings(const switchID_t switchID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "switchsettings;switch=" << switchID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::SwitchDelete(const switchID_t switchID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "switchdelete;switch=" << switchID;
		status << name << " deleted";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::TrackSettings(const trackID_t trackID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "tracksettings;track=" << trackID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::TrackDelete(const trackID_t trackID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "trackdelete;strack=" << trackID;
		status << name << " deleted";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoIntoTrack(const locoID_t locoID, const trackID_t trackID)
	{
		stringstream command;
		stringstream status;
		command << "locointotrack;loco=" << locoID << ";track=" << trackID;
		status << manager.LocoName(locoID) << " is on track " << manager.getTrackName(trackID);
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoRelease(const locoID_t locoID)
	{
		stringstream command;
		stringstream status;
		command << "locorelease;loco=" << locoID;
		status << manager.LocoName(locoID) << " is not on a track anymore";
		AddUpdate(command.str(), status.str());
	}
	;

	void WebServer::TrackRelease(const trackID_t trackID)
	{
		stringstream command;
		stringstream status;
		command << "trackrelease;track=" << trackID;
		status << manager.getTrackName(trackID) << " is released";
		AddUpdate(command.str(), status.str());
	}
	;

	void WebServer::StreetRelease(const streetID_t streetID)
	{
		stringstream command;
		stringstream status;
		command << "streetRelease;street=" << streetID;
		status << manager.getStreetName(streetID) << " is  released";
		AddUpdate(command.str(), status.str());
	}
	;

	void WebServer::LocoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
	{
		stringstream command;
		stringstream status;
		command << "locoStreet;loco=" << locoID << ";street=" << streetID << ";track=" << trackID;
		status << manager.LocoName(locoID) << " runs on street " << manager.getStreetName(streetID) << " with destination track " << manager.getTrackName(trackID);
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
	{
		stringstream command;
		stringstream status;
		command << "locoDestinationReached;loco=" << locoID << ";street=" << streetID << ";track=" << trackID;
		status << manager.LocoName(locoID) << " has reached the destination track " << manager.getTrackName(trackID) << " on street " << manager.getStreetName(streetID);
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoStart(const locoID_t locoID)
	{
		stringstream command;
		stringstream status;
		command << "locoStart;loco=" << locoID;
		status << manager.LocoName(locoID) << " is in auto mode";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoStop(const locoID_t locoID)
	{
		stringstream command;
		stringstream status;
		command << "locoStop;loco=" << locoID;
		status << manager.LocoName(locoID) << " is in manual mode";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoSettings(const locoID_t locoID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "locosettings;loco=" << locoID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LocoDelete(const locoID_t locoID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "locodelete;loco=" << locoID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LayerSettings(const layerID_t layerID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "layersettings;layer=" << layerID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::LayerDelete(const layerID_t layerID, const std::string& name)
	{
		stringstream command;
		stringstream status;
		command << "layerdelete;layer=" << layerID;
		status << name << " updated";
		AddUpdate(command.str(), status.str());
	}

	void WebServer::AddUpdate(const string& command, const string& status)
	{
		stringstream ss;
		ss << "data: command=" << command << ";status=" << status << "\r\n\r\n";
		std::lock_guard<std::mutex> lock(updateMutex);
		updates[++updateID] = ss.str();
		updates.erase(updateID - MaxUpdates);
	}

	bool WebServer::NextUpdate(unsigned int& updateIDClient, string& s)
	{
		std::lock_guard<std::mutex> lock(updateMutex);

		if (updateIDClient + MaxUpdates <= updateID)
		{
			updateIDClient = updateID - MaxUpdates + 1;
		}

		if (updates.count(updateIDClient) == 1)
		{
			s = updates.at(updateIDClient);
			return true;
		}

		return false;
	}

}; // namespace webserver
