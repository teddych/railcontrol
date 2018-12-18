#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "datatypes.h"
#include "network/Select.h"
#include "railcontrol.h"
#include "text/converters.h"
#include "util.h"
#include "console/console.h"

using std::map;
using std::thread;
using std::to_string;
using std::string;
using std::stringstream;
using std::vector;

namespace console
{

	Console::Console(Manager& manager, const unsigned short port)
	:	CommandInterface(ControlTypeConsole),
		port(port),
		serverSocket(0),
		clientSocket(-1),
		run(false),
		manager(manager)
	{
		struct sockaddr_in6 server_addr;

		xlog("Starting console on port %i", port);

		// create server socket
		serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
		if (serverSocket < 0)
		{
			xlog("Unable to create socket for console. Unable to serve clients.");
			return;
		}

		// bind socket to an address (in6addr_any)
		memset((char *) &server_addr, 0, sizeof(server_addr));
		server_addr.sin6_family = AF_INET6;
		server_addr.sin6_addr = in6addr_any;
		server_addr.sin6_port = htons(port);

		int on = 1;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on)) < 0)
		{
			xlog("Unable to set console socket option SO_REUSEADDR.");
		}

		if (bind(serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		{
			xlog("Unable to bind socket for console to port %i. Unable to serve clients.", port);
			close(serverSocket);
			return;
		}

		// listen on the socket
		if (listen(serverSocket, 5) != 0)
		{
			xlog("Unable to listen on socket for console server on port %i. Unable to serve clients.", port);
			close(serverSocket);
			return;
		}

		// create seperate thread that handles the client requests
		run = true;
		serverThread = thread([this] { Worker(); });
	}

	Console::~Console()
	{
		if (!run)
		{
            return;
		}

        xlog("Stopping console");
        run = false;

        // join server thread
        serverThread.join();
	}

	// worker is a seperate thread listening on the server socket
	void Console::Worker()
	{
		fd_set set;
		struct timeval tv;
		struct sockaddr_in6 client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		while (run)
		{
			// wait for connection and abort on shutdown
			int ret;
			do
			{
				FD_ZERO(&set);
				FD_SET(serverSocket, &set);
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tv));
			} while (ret == 0 && run);
			if (ret > 0 && run)
			{
				// accept connection
				clientSocket = accept(serverSocket, (struct sockaddr *) &client_addr, &client_addr_len);
				if (clientSocket < 0)
				{
					xlog("Unable to accept client connection for console: %i, %i", clientSocket, errno);
				}
				else
				{
					// handle client and fill into vector
					HandleClient();
				}
			}
		}
	}

	void Console::ReadBlanks(string& s, size_t& i)
	{
		// read possible blanks
		while (s.length() > i)
		{
			unsigned char input = s[i];
			if (input != ' ')
			{
				break;
			}
			++i;
		}
	}

	char Console::ReadCharacterWithoutEating(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		return s[i];
	}

	char Console::ReadCommand(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		char c = s[i];
		++i;
		return c;
	}

	int Console::ReadNumber(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		int number = 0;
		while (s.length() > i)
		{
			unsigned char input = s[i];
			if ( input < '0' || input > '9')
			{
				break;
			}
			number *= 10;
			number += input - '0';
			++i;
		};
		return number;
	}

	bool Console::ReadBool(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		if (s.length() <= i)
		{
			return false;
		}

		if (s[i] != 'o')
		{
			return (bool)ReadNumber(s, i);
		}

		++i;
		if (s.length() <= i)
		{
			return false;
		}

		bool ret = s[i] == 'n';
		while (s.length() > i && s[i] != ' ')
		{
			++i;
		}
		return ret;
	}

	string Console::ReadText(string& s, size_t& i)
	{
		ReadBlanks(s, i);

		size_t start = i;
		size_t length = 0;
		bool escape = false;
		while (s.length() > i)
		{
			if (s[i] == '\n' || s[i] == '\r')
			{
				++i;
				break;
			}

			if (s[i] == '"' && escape == false)
			{
				escape = true;
				++i;
				++start;
				continue;
			}

			if (s[i] == '"' && escape == true)
			{
				++i;
				break;
			}

			if (escape == false && s[i] == 0x20)
			{
				++i;
				break;
			}

			++length;
			++i;
		}
		string text(s, start, length);
		return text;
	}

	hardwareType_t Console::ReadHardwareType(string& s, size_t& i)
	{
		string type = ReadText(s, i);

		if (type.compare("virt") == 0)
		{
			return HardwareTypeVirtual;
		}
		else if (type.compare("cs2") == 0)
		{
			return HardwareTypeCS2;
		}
		else if (type.compare("m6051") == 0)
		{
			return HardwareTypeM6051;
		}
		else
		{
			try
			{
				int in = std::stoi(s);
				if (in <= HardwareTypeNone || in >= HardwareTypeNumbers)
				{
					return HardwareTypeNone;
				}
				return static_cast<hardwareType_t>(in);
			}
			catch (...)
			{
				return HardwareTypeNone;
			}
		}
	}


	switchType_t Console::ReadSwitchType(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		if (s.length() <= i)
		{
			return SwitchTypeLeft;
		}
		switch (s[i])
		{
			case 'l':
			case 'L':
				++i;
				return SwitchTypeLeft;

			case 'r':
			case 'R':
				++i;
				return SwitchTypeRight;

			default:
				switchType_t type = static_cast<switchType_t>(ReadNumber(s, i));
				return (type == SwitchTypeRight ? SwitchTypeRight : SwitchTypeLeft);
		}
	}

	layoutRotation_t Console::ReadRotation(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		if (s.length() <= i)
		{
			return Rotation0;
		}
		switch (s[i])
		{
			case 'n':
			case 'N':
			case 't':
			case 'T': // north/top
				++i;
				return Rotation0;

			case 'e':
			case 'E':
			case 'r':
			case 'R': // east/right
				++i;
				return Rotation90;

			case 's':
			case 'S':
			case 'b':
			case 'B': // south/bottom
				++i;
				return Rotation180;

			case 'w':
			case 'W':
			case 'l':
			case 'L': // west/left
				++i;
				return Rotation270;

			default:
				uint16_t rotation = ReadNumber(s, i);
				switch (rotation)
				{
					case Rotation90:
					case Rotation180:
					case Rotation270:
						return static_cast<layoutRotation_t>(rotation);

					case 90:
						return Rotation90;

					case 180:
						return Rotation180;

					case 270:
						return Rotation270;

					default:
						return Rotation0;
				}
		}
	}

	direction_t Console::ReadDirection(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		if (s.length() <= i)
		{
			return DirectionLeft;
		}
		switch (s[i])
		{
			case 'l':
			case 'L':
			case '-':
				++i;
				return DirectionLeft;

			case 'r':
			case 'R':
			case '+':
				++i;
				return DirectionRight;

			default:
				unsigned char direction = ReadNumber(s, i);
				return (direction == 0 ? DirectionLeft : DirectionRight);
		}
	}

	accessoryState_t Console::ReadAccessoryState(string& s, size_t& i)
	{
		bool state = ReadBool(s, i);
		return (state ? AccessoryStateOn : AccessoryStateOff);
	}

	void Console::HandleClient()
	{
		AddUpdate("Welcome to railcontrol console!\nType h for help\n");
		char buffer_in[1024];
		memset(buffer_in, 0, sizeof(buffer_in));

		while(run)
		{
			size_t pos = 0;
			string s;
			while(run && pos < sizeof(buffer_in) - 1 && s.find("\n") == string::npos && s.find("\r") == string::npos)
			{
				ssize_t ret = recv_timeout(clientSocket, buffer_in + pos, sizeof(buffer_in) - 1 - pos, 0);
				if (ret > 0)
				{
					pos += ret;
					s = string(buffer_in);
					continue;
				}

				if (errno == ETIMEDOUT)
				{
					continue;
				}

				close(clientSocket);
				return;
			}

			HandleCommand(s);
		}
	}

	void Console::HandleCommand(string& s)
	{
		size_t i = 0;
		switch (ReadCommand(s, i))
		{
			case 'a':
			case 'A':
				HandleAccessoryCommand(s, i);
				break;

			case 'b':
			case 'B':
				HandleTrackCommand(s, i);
				break;

			case 'c':
			case 'C':
				HandleControlCommand(s, i);
				break;

			case 'f':
			case 'F':
				HandleFeedbackCommand(s, i);
				break;

			case 'l':
			case 'L':
				HandleLocoCommand(s, i);
				break;

			case 'h':
			case 'H':
				HandleHelp();
				break;

			case 'p':
			case 'P':
				HandlePrintLayout();
				break;

			case 'q':
			case 'Q':
				HandleQuit();
				return;

			case 's':
			case 'S':
				HandleShutdown();
				return;

			case 't':
			case 'T':
				HandleStreetCommand(s, i);
				break;

			case 'w':
			case 'W':
				HandleSwitchCommand(s, i);
				break;

			default:
				AddUpdate("Unknown command");
		}
	}

	void Console::HandleAccessoryCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'd':
			case 'D':
				HandleAccessoryDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleAccessoryList(s, i);
				break;

			case 'n':
			case 'N':
				HandleAccessoryNew(s, i);
				break;

			case 's':
			case 'S':
				HandleAccessorySwitch(s, i);
				break;

			default:
				AddUpdate("Unknown accessory command");
		}
	}

	void Console::HandleTrackCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'd':
			case 'D':
				HandleTrackDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleTrackList(s, i);
				break;

			case 'n':
			case 'N':
				HandleTrackNew(s, i);
				break;

			case 'r':
			case 'R':
				HandleTrackRelease(s, i);
				break;

			default:
				AddUpdate("Unknown track command");
		}
	}

	void Console::HandleControlCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'd':
			case 'D':
				HandleControlDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleControlList(s, i);
				break;

			case 'n':
			case 'N':
				HandleControlNew(s, i);
				break;

			default:
				AddUpdate("Unknown control command");
		}
	}

	void Console::HandleFeedbackCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'd':
			case 'D':
				HandleFeedbackDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleFeedbackList(s, i);
				break;

			case 'n':
			case 'N':
				HandleFeedbackNew(s, i);
				break;

			case 's':
			case 'S':
				HandleFeedbackSet(s, i);
				break;

			case 'r':
			case 'R':
				HandleFeedbackRelease(s, i);
				break;

			default:
				AddUpdate("Unknown feedback command");
		}
	}

	void Console::HandleLocoCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'a':
			case 'A':
				HandleLocoAutomode(s, i);
				break;

			case 'b':
			case 'B':
				HandleLocoTrack(s, i);
				break;

			case 'd':
			case 'D':
				HandleLocoDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleLocoList(s, i);
				break;

			case 'm':
			case 'M':
				HandleLocoManualmode(s, i);
				break;

			case 'n':
			case 'N':
				HandleLocoNew(s, i);
				break;

			case 's':
			case 'S':
				HandleLocoSpeed(s, i);
				break;

			case 'r':
			case 'R':
				HandleLocoRelease(s, i);
				break;

			default:
				AddUpdate("Unknown loco command");
		}
	}

	void Console::HandleStreetCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'd':
			case 'D':
				HandleStreetDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleStreetList(s, i);
				break;

			case 'n':
			case 'N':
				HandleStreetNew(s, i);
				break;

			case 'r':
			case 'R':
				HandleStreetRelease(s, i);
				break;

			default:
				AddUpdate("Unknown street command");
		}
	}

	void Console::HandleSwitchCommand(string& s, size_t& i)
	{
		switch (ReadCommand(s, i))
		{
			case 'd':
			case 'D':
				HandleSwitchDelete(s, i);
				break;

			case 'l':
			case 'L':
				HandleSwitchList(s, i);
				break;

			case 'n':
			case 'N':
				HandleSwitchNew(s, i);
				break;

			/*
			case 'r':
			case 'R':
				HandleSwitchRelease(s, i);
				break;

			*/
			default:
				AddUpdate("Unknown switch command");
		}
	}

	void Console::HandleAccessoryDelete(string& s, size_t& i)
	{
		accessoryID_t accessoryID = ReadNumber(s, i);
		if (!manager.accessoryDelete(accessoryID))
		{
			AddUpdate("Accessory not found or accessory in use");
			return;
		}
		AddUpdate("Accessory deleted");
	}

	void Console::HandleAccessoryList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all accessories
			std::map<accessoryID_t,datamodel::Accessory*> accessories = manager.accessoryList();
			stringstream status;
			for (auto accessory : accessories)
			{
				status << accessory.first << " " << accessory.second->name << "\n";
			}
			status << "Total number of accessorys: " << accessories.size();
			AddUpdate(status.str());
			return;
		}

		accessoryID_t accessoryID = ReadNumber(s, i);
		datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			AddUpdate("Unknown accessory");
			return;
		}

		stringstream status;
		status << accessoryID << " " << accessory->name << " (" << static_cast<int>(accessory->posX) << "/" << static_cast<int>(accessory->posY) << "/" << static_cast<int>(accessory->posZ) << ")";
		AddUpdate(status.str());
	}

	void Console::HandleAccessoryNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		layoutPosition_t posX = ReadNumber(s, i);
		layoutPosition_t posY = ReadNumber(s, i);
		layoutPosition_t posZ = ReadNumber(s, i);
		controlID_t controlID = ReadNumber(s, i);
		protocol_t protocol = static_cast<protocol_t>(ReadNumber(s, i));
		address_t address = ReadNumber(s, i);
		accessoryTimeout_t timeout = ReadNumber(s, i);
		bool inverted = ReadBool(s, i);
		string result;
		if (!manager.accessorySave(AccessoryNone, name, posX, posY, posZ, controlID, protocol, address, AccessoryTypeDefault, timeout, inverted, result))
		{
			AddUpdate(result);
			return;
		}
		stringstream status;
		status << "Accessory \"" << name << "\" added";
		AddUpdate(status.str());
	}

	void Console::HandleAccessorySwitch(string& s, size_t& i)
	{
		accessoryID_t accessoryID = ReadNumber(s, i);
		datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			AddUpdate("Unknown accessory");
			return;
		}

		accessoryState_t state = ReadAccessoryState(s, i);
		manager.accessory(ControlTypeConsole, accessoryID, state);
	}

	void Console::HandleTrackDelete(string& s, size_t& i)
	{
		trackID_t trackID = ReadNumber(s, i);
		if (!manager.trackDelete(trackID))
		{
			AddUpdate("Track not found or track in use");
			return;
		}
		AddUpdate("Track deleted");
	}

	void Console::HandleTrackList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all tracks
			std::map<trackID_t,datamodel::Track*> tracks = manager.trackList();
			stringstream status;
			for (auto track : tracks)
			{
				status << track.first << " " << track.second->name << "\n";
			}
			status << "Total number of tracks: " << tracks.size();
			AddUpdate(status.str());
			return;
		}

		// list one track
		trackID_t trackID = ReadNumber(s, i);
		datamodel::Track* track = manager.getTrack(trackID);
		if (track == nullptr)
		{
			AddUpdate("Unknown track");
			return;
		}
		stringstream status;
		status
			<< "Track ID: " << trackID
			<< "\nName:     " << track->name
			<< "\nX:        " << static_cast<int>(track->posX)
			<< "\nY:        " << static_cast<int>(track->posY)
			<< "\nZ:        " << static_cast<int>(track->posZ);
		string stateText;
		text::Converters::lockStatus(track->getState(), stateText);
		status << "\nStatus:   " << stateText;
		status << "\nLoco:     ";
		if (track->getLoco() == LocoNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getLocoName(track->getLoco()) << " (" << track->getLoco() << ")";
		}
		AddUpdate(status.str());
	}

	void Console::HandleTrackNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		layoutPosition_t posX = ReadNumber(s, i);
		layoutPosition_t posY = ReadNumber(s, i);
		layoutPosition_t posZ = ReadNumber(s, i);
		layoutItemSize_t width = ReadNumber(s, i);
		layoutRotation_t rotation = ReadRotation(s, i);
		string result;
		if (!manager.trackSave(TrackNone, name, posX, posY, posZ, width, rotation, result))
		{
			AddUpdate(result);
			return;
		}
		stringstream status;
		status << "Track \"" << name << "\" added";
		AddUpdate(status.str());
	}

	void Console::HandleTrackRelease(string& s, size_t& i)
	{
		trackID_t trackID = ReadNumber(s, i);
		if (!manager.trackRelease(trackID))
		{
			AddUpdate("Track not found or track in use");
			return;
		}
		AddUpdate("Track released");
	}

	void Console::HandleControlDelete(string& s, size_t& i)
	{
		controlID_t controlID = ReadNumber(s, i);
		if (!manager.controlDelete(controlID))
		{
			AddUpdate("Control not found or control in use");
			return;
		}
		AddUpdate("Control deleted");
	}

	void Console::HandleControlList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all controls
			std::map<controlID_t,hardware::HardwareParams*> params = manager.controlList();
			stringstream status;
			for (auto param : params)
			{
				status << static_cast<int>(param.first) << " " << param.second->name << "\n";
			}
			status << "Total number of controls: " << params.size();
			AddUpdate(status.str());
			return;
		}

		controlID_t controlID = ReadNumber(s, i);
		hardware::HardwareParams* param = manager.getHardware(controlID);
		if (param == nullptr)
		{
			AddUpdate("Unknown Control");
			return;
		}

		stringstream status;
		status << static_cast<int>(controlID) << " " << param->name;
		AddUpdate(status.str());
	}

	void Console::HandleControlNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		hardwareType_t hardwareType = ReadHardwareType(s, i);
		if (hardwareType == HardwareTypeNone)
		{
			AddUpdate("Unknown hardwaretype");
			return;
		}

		string arg1 = ReadText(s, i);

		string result;
		if (!manager.controlSave(ControlIdNone, hardwareType, name, arg1, result))
		{
			AddUpdate(result);
			return;
		}

		stringstream status;
		status << "Control \"" << name << "\" added";
		AddUpdate(status.str());
	}

	void Console::HandleFeedbackDelete(string& s, size_t& i)
	{
		feedbackID_t feedbackID = ReadNumber(s, i);
		if (!manager.feedbackDelete(feedbackID))
		{
			AddUpdate("Feedback not found or feedback in use");
			return;
		}
		AddUpdate("Feedback deleted");
	}

	void Console::HandleFeedbackList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all feedbacks
			std::map<feedbackID_t,datamodel::Feedback*> feedbacks = manager.feedbackList();
			stringstream status;
			for (auto feedback : feedbacks)
			{
				status << feedback.first << " " << feedback.second->name << "\n";
			}
			status << "Total number of feedbacks: " << feedbacks.size();
			AddUpdate(status.str());
			return;
		}

		// list one feedback
		feedbackID_t feedbackID = ReadNumber(s, i);
		datamodel::Feedback* feedback = manager.getFeedback(feedbackID);
		if (feedback == nullptr)
		{
			AddUpdate("Unknown feedback");
			return;
		}

		stringstream status;
		status
			<< "FeedbackID" << feedbackID
			<< "\nName:     " << feedback->name
			<< "\nControl:  " << manager.getControlName(feedback->controlID)
			<< "\nPin:      " << feedback->pin
			<< "\nX:        " << static_cast<int>(feedback->posX)
			<< "\nY:        " << static_cast<int>(feedback->posY)
			<< "\nZ:        " << static_cast<int>(feedback->posZ);
		string stateText;
		text::Converters::feedbackStatus(feedback->getState(), stateText);
		status << "\nStatus:   " << stateText;
		status << "\nLoco:     ";
		if (feedback->getLoco() == LocoNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getLocoName(feedback->getLoco()) << " (" << feedback->getLoco() << ")";
		}
		AddUpdate(status.str());
	}

	void Console::HandleFeedbackNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		layoutPosition_t posX = ReadNumber(s, i);
		layoutPosition_t posY = ReadNumber(s, i);
		layoutPosition_t posZ = ReadNumber(s, i);
		controlID_t control = ReadNumber(s, i);
		feedbackPin_t pin = ReadNumber(s, i);
		bool inverted = ReadBool(s, i);
		string result;
		if(!manager.feedbackSave(FeedbackNone, name, posX, posY, posZ, control, pin, inverted, result))
		{
			AddUpdate(result);
			return;
		}
		stringstream status;
		status << "Feedback \"" << name << "\" added";
		AddUpdate(status.str());
	}

	void Console::HandleFeedbackSet(string& s, size_t& i)
	{
		feedbackID_t feedbackID = ReadNumber(s, i);
		unsigned char input = ReadCharacterWithoutEating(s, i);
		feedbackState_t state;
		char* text;
		if (input == 'X' || input == 'x')
		{
			state = FeedbackStateOccupied;
			text = (char*)"ON";
		}
		else
		{
			state = FeedbackStateFree;
			text = (char*)"OFF";
		}
		manager.feedback(ControlTypeConsole, feedbackID, state);
		stringstream status;
		status << "Feedback \"" << manager.getFeedbackName(feedbackID) << "\" turned " << text;
		AddUpdate(status.str());
	}

	void Console::HandleFeedbackRelease(string& s, size_t& i)
	{
		feedbackID_t feedbackID = ReadNumber(s, i);
		if (!manager.feedbackRelease(feedbackID))
		{
			AddUpdate("Feedback not found");
			return;
		}
		AddUpdate("Feedback released");
	}

	void Console::HandleLocoAutomode(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{ // set all locos to automode
			manager.locoStartAll();
			return;
		}

		// set specific loco to auto mode
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.locoStart(locoID))
		{
			// FIXME: bether errormessage
			AddUpdate("Unknown loco or loco is not in a track");
		}
	}

	void Console::HandleLocoTrack(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		trackID_t trackID = ReadNumber(s, i);
		if (!manager.locoIntoTrack(locoID, trackID))
		{
			// FIXME: bether errormessage
			AddUpdate("Unknown loco or unknown track");
		}
	}

	void Console::HandleLocoDelete(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.locoDelete(locoID))
		{
			AddUpdate("Loco not found or loco in use");
			return;
		}
		AddUpdate("Loco deleted");
	}

	void Console::HandleLocoList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all locos
			std::map<locoID_t,datamodel::Loco*> locos = manager.locoList();
			stringstream status;
			for (auto loco : locos) {
				status << loco.first << " " << loco.second->name << "\n";
			}
			status << "Total number of locos: " << locos.size();
			AddUpdate(status.str());
			return;
		}

		// list one loco
		locoID_t locoID = ReadNumber(s, i);
		datamodel::Loco* loco = manager.getLoco(locoID);
		if (loco == nullptr)
		{
			AddUpdate("Unknown loco");
			return;
		}
		stringstream status;
		status
			<< "Loco ID:  " << locoID
			<< "\nName:     " << loco->name
			<< "\nSpeed:    " << manager.locoSpeed(locoID)
			<< "\nControl:  " << manager.getControlName(loco->controlID)
			<< "\nProtocol: " << protocolSymbols[loco->protocol]
			<< "\nAddress:  " << loco->address;
		const char* const locoStateText = loco->getStateText();
		status << "\nStatus:   " << locoStateText;
		status << "\nTrack:    ";
		if (loco->track() == TrackNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getTrackName(loco->track()) << " (" << loco->track() << ")";
		}
		status << "\nStreet:   ";
		if (loco->street() == StreetNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getStreetName(loco->street()) << " (" << loco->street() << ")";
		}
		AddUpdate(status.str());
	}

	void Console::HandleLocoManualmode(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// set all locos to manual mode
			manager.locoStopAll();
			return;
		}
		// set specific loco to manual mode
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.locoStop(locoID))
		{
			// FIXME: bether errormessage
			AddUpdate("Unknown loco");
		}
	}

	void Console::HandleLocoNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		controlID_t control = ReadNumber(s, i);
		protocol_t protocol = static_cast<protocol_t>(ReadNumber(s, i));
		address_t address = ReadNumber(s, i);
		function_t functions = ReadNumber(s, i);
		string result;
		if (!manager.locoSave(LocoNone, name, control, protocol, address, functions, result))
		{
			AddUpdate(result);
			return;
		}
		stringstream status;
		status << "Loco \"" << name << "\" added";
		AddUpdate(status.str());
	}

	void Console::HandleLocoSpeed(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		LocoSpeed speed = ReadNumber(s, i);
		if (!manager.locoSpeed(ControlTypeConsole, locoID, speed))
		{
			// FIXME: bether errormessage
			AddUpdate("Unknown loco");
		}
	}

	void Console::HandleLocoRelease(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.locoRelease(locoID))
		{
			// FIXME: bether errormessage
			AddUpdate("Loco not found or track in use");
			return;
		}
		AddUpdate("Loco released");
	}

	void Console::HandleHelp()
	{
		string status("Available console commands:\n"
				"\n"
				"Accessory commands\n"
				"A D accessory#                    Delete accessory\n"
				"A L A                             List all accessories\n"
				"A L accessory#                    List accessory\n"
				"A N Name X Y Z Control Protocol Address Timeout(ms) Inverted\n"
				"                                  New Accessory\n"
				"A S accessory# state              Switch accessory\n"
				"\n"
				"Track commands\n"
				"B D track#                        Delete track\n"
				"B L A                             List all tracks\n"
				"B L track#                        List track\n"
				"B N Name X Y Z Width Rotation     New track\n"
				"B R track#                        Release track\n"
				"\n"
				"Control commands\n"
				"C D control#                      Delete control\n"
				"C L A                             List all controls\n"
				"C L control#                      List control\n"
				"C N Name Type Arg1                New control\n"
				"\n"
				"Feedback commands\n"
				"F D feedback#                     Delete feedback\n"
				"F L A                             List all feedbacks\n"
				"F L feedback#                     List feedback\n"
				"F N Name X Y Z Control Pin/Address Invert\n"
				"                                  New feedback\n"
				"L R feedback#                     Release feedback\n"
				"F S feedback# [X]                 Turn feedback on (with X) or off (without X)\n"
				"\n"
				"Loco commands\n"
				"L A A                             Start all locos into automode\n"
				"L A loco#                         Start loco into automode\n"
				"L B loco# track#                  Set loco on track\n"
				"L D loco#                         Delete loco\n"
				"L L A                             List all locos\n"
				"L L loco#                         List loco\n"
				"L M A                             Stop all locos and go to manual mode\n"
				"L M loco#                         Stop loco and go to manual mode\n"
				"L N Name Control Protocol Address NrOfFunctions New loco\n"
				"L R loco#                         Release loco\n"
				"L S loco# speed                   Set loco speed between 0 and 1024\n"
				"\n"
				"Street commands\n"
				"T D street#                       Delete street\n"
				"T L A                             List all streets\n"
				"T L street#                       List street\n"
				"T N Name FromTrack FromDirektion ToTrack ToDirection FeedbackStop\n"
				"T R street#                       Release street\n"
				"                                  New Feedback\n"
				"\n"
				"Switch commands\n"
				"W D switch#                       Delete switch\n"
				"W L A                             List all switches\n"
				"W L switch#                       List switch\n"
				"W N Name X Y Z Rotation Control Protocol Address Type(L/R) Timeout(ms) Inverted\n"
				"                                  New Switch\n"
				"\n"
				"Other commands\n"
				"H                                 Show this help\n"
				"P                                 Print layout\n"
				"Q                                 Quit console\n"
				"S                                 Shut down railcontrol\n");
		AddUpdate(status);
	}

	void Console::HandlePrintLayout()
	{
		stringstream status;
		status << "\033[2J";
		status << "\033[0;0H";
		status << "Layout 0";
		// print tracks
		const map<trackID_t,datamodel::Track*>& tracks = manager.trackList();
		for (auto track : tracks)
		{
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t w;
			layoutItemSize_t h;
			layoutRotation_t r;
			track.second->position(posX, posY, posZ, w, h, r);
			if (posZ != 0)
			{
				continue;
			}
			status << "\033[" << (int)(posY + 2) << ";" << (int)(posX + 1) << "H";
			status << "Bloc";
		}
		// print switches
		const map<switchID_t,datamodel::Switch*>& switches = manager.switchList();
		for (auto mySwitch : switches)
		{
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t w;
			layoutItemSize_t h;
			layoutRotation_t r;
			mySwitch.second->position(posX, posY, posZ, w, h, r);
			if (posZ != 0)
			{
				continue;
			}
			status << "\033[" << (int)(posY + 2) << ";" << (int)(posX + 1) << "H";
			status << "S";
		}
		// print accessories
		const map<accessoryID_t,datamodel::Accessory*>& accessories = manager.accessoryList();
		for (auto accessory : accessories)
		{
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t w;
			layoutItemSize_t h;
			layoutRotation_t r;
			accessory.second->position(posX, posY, posZ, w, h, r);
			if (posZ != 0)
			{
				continue;
			}
			status << "\033[" << (int)(posY + 2) << ";" << (int)(posX + 1) << "H";
			status << "A";
		}
		// print cursor at correct position
		status << "\033[20;0H";
		AddUpdate(status.str());
	}

	void Console::HandleQuit()
	{
		AddUpdate("Quit railcontrol console");
		close(clientSocket);
	}

	void Console::HandleShutdown()
	{
		AddUpdate("Shutting down railcontrol");
		stopRailControlConsole();
		close(clientSocket);
	}

	void Console::HandleStreetDelete(string& s, size_t& i)
	{
		streetID_t streetID = ReadNumber(s, i);
		if (!manager.streetDelete(streetID))
		{
			AddUpdate("Street not found or street in use");
			return;
		}
		AddUpdate("Street deleted");
	}

	void Console::HandleStreetList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all streetes
			std::map<streetID_t,datamodel::Street*> streets = manager.streetList();
			stringstream status;
			for (auto street : streets) {
				status << street.first << " " << street.second->name << "\n";
			}
			status << "Total number of streets: " << streets.size();
			AddUpdate(status.str());
			return;
		}

		// list one street
		streetID_t streetID = ReadNumber(s, i);
		datamodel::Street* street = manager.getStreet(streetID);
		if (street == nullptr)
		{
			AddUpdate("Unknown street");
			return;
		}
		stringstream status;
		status
			<< "Street ID " << streetID
			<< "\nName:     " << street->name
			<< "\nStart:    ";
		if (street->fromTrack == TrackNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getTrackName(street->fromTrack) << " (" << street->fromTrack << ") " << (street->fromDirection ? ">" : "<");
		}
		status << "\nEnd:      ";
		if (street->toTrack == TrackNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getTrackName(street->toTrack) << " (" << street->toTrack << ") " << (street->toDirection ? ">" : "<");
		}
		string stateText;
		text::Converters::lockStatus(street->getState(), stateText);
		status << "\nStatus:   " << stateText;
		status << "\nLoco:     ";
		if (street->getLoco() == LocoNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getLocoName(street->getLoco()) << " (" << street->getLoco() << ")";
		}
		AddUpdate(status.str());
	}

	void Console::HandleStreetNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		trackID_t fromTrack = ReadNumber(s, i);
		direction_t fromDirection = ReadDirection(s, i);
		trackID_t toTrack = ReadNumber(s, i);
		direction_t toDirection = ReadDirection(s, i);
		feedbackID_t feedbackID = ReadNumber(s, i);
		string result;
		if (!manager.streetSave(StreetNone, name, fromTrack, fromDirection, toTrack, toDirection, feedbackID, result))
		{
			AddUpdate(result);
			return;
		}
		stringstream status;
		status << "Street \"" << name << "\" added";
		AddUpdate(status.str());
	}

	void Console::HandleStreetRelease(string& s, size_t& i)
	{
		streetID_t streetID = ReadNumber(s, i);
		if (!manager.streetRelease(streetID))
		{
			AddUpdate("Street not found or track in use");
			return;
		}
		AddUpdate("Street released");
	}

	void Console::HandleSwitchDelete(string& s, size_t& i)
	{
		switchID_t switchID = ReadNumber(s, i);
		if (!manager.switchDelete(switchID))
		{
			AddUpdate("Switch not found or switch in use");
			return;
		}
		AddUpdate("Switch deleted");
	}

	void Console::HandleSwitchList(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// list all switches
			std::map<switchID_t,datamodel::Switch*> switches = manager.switchList();
			stringstream status;
			for (auto mySwitch : switches) {
				status << mySwitch.first << " " << mySwitch.second->name << "\n";
			}
			status << "Total number of switches: " << switches.size();
			AddUpdate(status.str());
			return;
		}

		// list one switch
		switchID_t switchID = ReadNumber(s, i);
		datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		if (mySwitch == nullptr)
		{
			AddUpdate("Unknown switch");
			return;
		}
		stringstream status;
		status
			<< "Switch ID " << switchID
			<< "\nName:     " << mySwitch->name
			<< "\nX:        " << static_cast<int>(mySwitch->posX)
			<< "\nY:        " << static_cast<int>(mySwitch->posY)
			<< "\nZ:        " << static_cast<int>(mySwitch->posZ)
			<< "\nRotation: ";
		switch (mySwitch->rotation) {
			case Rotation0:
				status << "0";
				break;

			case Rotation90:
				status << "90";
				break;

			case Rotation180:
				status << "180";
				break;

			case Rotation270:
				status << "270";
				break;

			default:
				status << "unknown";
		}
		string state;
		text::Converters::switchStatus(static_cast<switchState_t>(mySwitch->state), state);
		status << "\nState:    " << state;
		/*
		status << "\nLoco:     ";
		if (mySwitch->getLoco() == LOCO_NONE)
		{
			status << "-";
		}
		else
		{
			status << manager.getLocoName(mySwitch->getLoco()) << " (" << mySwitch->getLoco() << ")";
		}
		*/
		AddUpdate(status.str());
	}

	void Console::HandleSwitchNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		layoutPosition_t posX = ReadNumber(s, i);
		layoutPosition_t posY = ReadNumber(s, i);
		layoutPosition_t posZ = ReadNumber(s, i);
		layoutRotation_t rotation = ReadRotation(s, i);
		controlID_t controlID = ReadNumber(s, i);
		protocol_t protocol = static_cast<protocol_t>(ReadNumber(s, i));
		address_t address = ReadNumber(s, i);
		switchType_t type = ReadSwitchType(s, i);
		accessoryTimeout_t timeout = ReadNumber(s, i);
		bool inverted = ReadBool(s, i);
		string result;
		if (!manager.switchSave(SwitchNone, name, posX, posY, posZ, rotation, controlID, protocol, address, type, timeout, inverted, result))
		{
			AddUpdate(result);
			return;
		}
		stringstream status;
		status << "Switch \"" << name << "\" added";
		AddUpdate(status.str());
	}

	/*
	void Console::HandleSwitchRelease(string& s, size_t& i)
	{
		switchID_t switchID = readNumber(s, i);
		if (!manager.switchRelease(switchID))
		{
			addUpdate("Switch not found");
			return;
		}
		addUpdate("Switch released");
	}
	*/

	void Console::AddUpdate(const string& status)
	{
		if (clientSocket < 0)
		{
			return;
		}
		string s(status);
		s.append("\n> ");
		send_timeout(clientSocket, s.c_str(), s.length(), 0);
	}

	void Console::booster(const controlType_t managerID, const boosterStatus_t status)
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

	void Console::locoSpeed(const controlType_t managerID, const locoID_t locoID, const LocoSpeed speed)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " speed is " << speed;
		AddUpdate(status.str());
	}

	void Console::locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction)
	{
		std::stringstream status;
		const char* directionText = (direction ? "forward" : "reverse");
		status << manager.getLocoName(locoID) << " direction is " << directionText;
		AddUpdate(status.str());
	}

	void Console::locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool state)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " f" << (unsigned int)function << " is " << (state ? "on" : "off");
		AddUpdate(status.str());
	}

	void Console::accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on)
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

	void Console::feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state)
	{
		std::stringstream status;
		status << "Feedback " << pin << " is " << (state ? "on" : "off");
		AddUpdate(status.str());
	}

	void Console::track(const controlType_t managerID, const trackID_t trackID, const lockState_t lockState)
	{
		std::stringstream status;
		string stateText;
		text::Converters::lockStatus(lockState, stateText);
		status << manager.getTrackName(trackID) << " is " << stateText;
		AddUpdate(status.str());
	}

	void Console::handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state, const bool on)
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

	void Console::locoIntoTrack(const locoID_t locoID, const trackID_t trackID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " is in track " << manager.getTrackName(trackID);
		AddUpdate(status.str());
	}

	void Console::locoRelease(const locoID_t locoID)
	{
		stringstream status;
		status << manager.getLocoName(locoID) << " is not in a track anymore";
		AddUpdate(status.str());
	};

	void Console::trackRelease(const trackID_t trackID)
	{
		stringstream status;
		status << manager.getTrackName(trackID) << " is released";
		AddUpdate(status.str());
	};

	void Console::streetRelease(const streetID_t streetID)
	{
		stringstream status;
		status << manager.getStreetName(streetID) << " is  released";
		AddUpdate(status.str());
	};

	void Console::locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " runs on street " << manager.getStreetName(streetID) << " with destination track " << manager.getTrackName(trackID);
		AddUpdate(status.str());
	}

	void Console::locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " has reached the destination track " << manager.getTrackName(trackID) << " on street " << manager.getStreetName(streetID);
		AddUpdate(status.str());
	}

	void Console::locoStart(const locoID_t locoID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " is in auto mode";
		AddUpdate(status.str());
	}

	void Console::locoStop(const locoID_t locoID)
	{
		std::stringstream status;
		status << manager.getLocoName(locoID) << " is in manual mode";
		AddUpdate(status.str());
	}

}; // namespace console
