#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

#include "console/ConsoleClient.h"
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
	void ConsoleClient::Worker()
	{
		logger->Info("Open connection");
		WorkerImpl();
		logger->Info("Close connection");
	}

	void ConsoleClient::WorkerImpl()
	{
		run = true;
		SendAndPrompt("> Welcome to railcontrol! Press H for help.");
		while (run)
		{
			char buffer_in[1024];
			memset(buffer_in, 0, sizeof(buffer_in));

			size_t pos = 0;
			string s;
			while (pos < sizeof(buffer_in) - 1 && s.find("\n") == string::npos && run)
			{
				size_t ret = connection->Receive(buffer_in + pos, sizeof(buffer_in) - 1 - pos, 0);
				if (ret == static_cast<size_t>(-1))
				{
					if (errno == ETIMEDOUT)
					{
						continue;
					}
					return;
				}
				pos += ret;
				s = string(buffer_in);
				str_replace(s, string("\r\n"), string("\n"));
				str_replace(s, string("\r"), string("\n"));
			}
			HandleCommand(s);
		}
	}

	void ConsoleClient::SendAndPrompt(const string str)
	{
		string s(str);
		s.append("\n> ");
		connection->Send(s);
	}

	void ConsoleClient::ReadBlanks(string& s, size_t& i)
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

	char ConsoleClient::ReadCharacterWithoutEating(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		return s[i];
	}

	char ConsoleClient::ReadCommand(string& s, size_t& i)
	{
		ReadBlanks(s, i);
		char c = s[i];
		++i;
		return c;
	}

	int ConsoleClient::ReadNumber(string& s, size_t& i)
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

	bool ConsoleClient::ReadBool(string& s, size_t& i)
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

	string ConsoleClient::ReadText(string& s, size_t& i)
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

	hardwareType_t ConsoleClient::ReadHardwareType(string& s, size_t& i)
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
		else if (type.compare("rm485") == 0)
		{
			return HardwareTypeRM485;
		}
		else
		{
			try
			{
				int in = Util::StringToInteger(s);
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


	switchType_t ConsoleClient::ReadSwitchType(string& s, size_t& i)
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

	layoutRotation_t ConsoleClient::ReadRotation(string& s, size_t& i)
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

	direction_t ConsoleClient::ReadDirection(string& s, size_t& i)
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

	accessoryState_t ConsoleClient::ReadAccessoryState(string& s, size_t& i)
	{
		bool state = ReadBool(s, i);
		return (state ? AccessoryStateOn : AccessoryStateOff);
	}

	/*
	void ConsoleClient::HandleClient()
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
	*/

	void ConsoleClient::HandleCommand(string& s)
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
				SendAndPrompt("Unknown command");
		}
	}

	void ConsoleClient::HandleAccessoryCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown accessory command");
		}
	}

	void ConsoleClient::HandleTrackCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown track command");
		}
	}

	void ConsoleClient::HandleControlCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown control command");
		}
	}

	void ConsoleClient::HandleFeedbackCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown feedback command");
		}
	}

	void ConsoleClient::HandleLocoCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown loco command");
		}
	}

	void ConsoleClient::HandleStreetCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown street command");
		}
	}

	void ConsoleClient::HandleSwitchCommand(string& s, size_t& i)
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
				SendAndPrompt("Unknown switch command");
		}
	}

	void ConsoleClient::HandleAccessoryDelete(string& s, size_t& i)
	{
		accessoryID_t accessoryID = ReadNumber(s, i);
		if (!manager.accessoryDelete(accessoryID))
		{
			SendAndPrompt("Accessory not found or accessory in use");
			return;
		}
		SendAndPrompt("Accessory deleted");
	}

	void ConsoleClient::HandleAccessoryList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		accessoryID_t accessoryID = ReadNumber(s, i);
		datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			SendAndPrompt("Unknown accessory");
			return;
		}

		stringstream status;
		status << accessoryID << " " << accessory->name << " (" << static_cast<int>(accessory->posX) << "/" << static_cast<int>(accessory->posY) << "/" << static_cast<int>(accessory->posZ) << ")";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleAccessoryNew(string& s, size_t& i)
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
			SendAndPrompt(result);
			return;
		}
		stringstream status;
		status << "Accessory \"" << name << "\" added";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleAccessorySwitch(string& s, size_t& i)
	{
		accessoryID_t accessoryID = ReadNumber(s, i);
		datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			SendAndPrompt("Unknown accessory");
			return;
		}

		accessoryState_t state = ReadAccessoryState(s, i);
		manager.AccessoryState(ControlTypeConsole, accessoryID, state);
	}

	void ConsoleClient::HandleTrackDelete(string& s, size_t& i)
	{
		trackID_t trackID = ReadNumber(s, i);
		if (!manager.TrackDelete(trackID))
		{
			SendAndPrompt("Track not found or track in use");
			return;
		}
		SendAndPrompt("Track deleted");
	}

	void ConsoleClient::HandleTrackList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		// list one track
		trackID_t trackID = ReadNumber(s, i);
		datamodel::Track* track = manager.GetTrack(trackID);
		if (track == nullptr)
		{
			SendAndPrompt("Unknown track");
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
		text::Converters::lockStatus(track->GetLockState(), stateText);
		status << "\nStatus:   " << stateText;
		status << "\nLoco:     ";
		if (track->GetLoco() == LocoNone)
		{
			status << "-";
		}
		else
		{
			status << manager.LocoName(track->GetLoco()) << " (" << track->GetLoco() << ")";
		}
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleTrackNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		layoutPosition_t posX = ReadNumber(s, i);
		layoutPosition_t posY = ReadNumber(s, i);
		layoutPosition_t posZ = ReadNumber(s, i);
		layoutItemSize_t length = ReadNumber(s, i);
		layoutRotation_t rotation = ReadRotation(s, i);
		vector<feedbackID_t> feedbacks;
		string result;
		if (manager.TrackSave(TrackNone, name, posX, posY, posZ, length, rotation, TrackTypeStraight, feedbacks, result) > TrackNone)
		{
			SendAndPrompt(result);
			return;
		}
		stringstream status;
		status << "Track \"" << name << "\" added";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleTrackRelease(string& s, size_t& i)
	{
		trackID_t trackID = ReadNumber(s, i);
		if (!manager.TrackRelease(trackID))
		{
			SendAndPrompt("Track not found or track in use");
			return;
		}
		SendAndPrompt("Track released");
	}

	void ConsoleClient::HandleControlDelete(string& s, size_t& i)
	{
		controlID_t controlID = ReadNumber(s, i);
		if (!manager.controlDelete(controlID))
		{
			SendAndPrompt("Control not found or control in use");
			return;
		}
		SendAndPrompt("Control deleted");
	}

	void ConsoleClient::HandleControlList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		controlID_t controlID = ReadNumber(s, i);
		hardware::HardwareParams* param = manager.getHardware(controlID);
		if (param == nullptr)
		{
			SendAndPrompt("Unknown Control");
			return;
		}

		stringstream status;
		status << static_cast<int>(controlID) << " " << param->name;
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleControlNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		hardwareType_t hardwareType = ReadHardwareType(s, i);
		if (hardwareType == HardwareTypeNone)
		{
			SendAndPrompt("Unknown hardwaretype");
			return;
		}

		string arg1 = ReadText(s, i);
		string arg2 = ReadText(s, i);
		string arg3 = ReadText(s, i);
		string arg4 = ReadText(s, i);
		string arg5 = ReadText(s, i);

		string result;
		if (!manager.controlSave(ControlIdNone, hardwareType, name, arg1, arg2, arg3, arg4, arg5, result))
		{
			SendAndPrompt(result);
			return;
		}

		stringstream status;
		status << "Control \"" << name << "\" added";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleFeedbackDelete(string& s, size_t& i)
	{
		feedbackID_t feedbackID = ReadNumber(s, i);
		if (!manager.FeedbackDelete(feedbackID))
		{
			SendAndPrompt("Feedback not found or feedback in use");
			return;
		}
		SendAndPrompt("Feedback deleted");
	}

	void ConsoleClient::HandleFeedbackList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		// list one feedback
		feedbackID_t feedbackID = ReadNumber(s, i);
		datamodel::Feedback* feedback = manager.GetFeedback(feedbackID);
		if (feedback == nullptr)
		{
			SendAndPrompt("Unknown feedback");
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
		text::Converters::feedbackStatus(feedback->GetState(), stateText);
		status << "\nStatus:   " << stateText;
		status << "\nLoco:     ";
		if (feedback->GetLoco() == LocoNone)
		{
			status << "-";
		}
		else
		{
			status << manager.LocoName(feedback->GetLoco()) << " (" << feedback->GetLoco() << ")";
		}
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleFeedbackNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		layoutPosition_t posX = ReadNumber(s, i);
		layoutPosition_t posY = ReadNumber(s, i);
		layoutPosition_t posZ = ReadNumber(s, i);
		controlID_t control = ReadNumber(s, i);
		feedbackPin_t pin = ReadNumber(s, i);
		bool inverted = ReadBool(s, i);
		string result;
		if(manager.FeedbackSave(FeedbackNone, name, VisibleYes, posX, posY, posZ, control, pin, inverted, result) == FeedbackNone)
		{
			SendAndPrompt(result);
			return;
		}
		stringstream status;
		status << "Feedback \"" << name << "\" added";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleFeedbackSet(string& s, size_t& i)
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
		manager.FeedbackState(ControlTypeConsole, feedbackID, state);
		stringstream status;
		status << "Feedback \"" << manager.GetFeedbackName(feedbackID) << "\" turned " << text;
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleFeedbackRelease(string& s, size_t& i)
	{
		feedbackID_t feedbackID = ReadNumber(s, i);
		if (!manager.feedbackRelease(feedbackID))
		{
			SendAndPrompt("Feedback not found");
			return;
		}
		SendAndPrompt("Feedback released");
	}

	void ConsoleClient::HandleLocoAutomode(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{ // set all locos to automode
			manager.locoStartAll();
			return;
		}

		// set specific loco to auto mode
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.LocoStart(locoID))
		{
			// FIXME: bether errormessage
			SendAndPrompt("Unknown loco or loco is not in a track");
		}
	}

	void ConsoleClient::HandleLocoTrack(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		trackID_t trackID = ReadNumber(s, i);
		if (!manager.LocoIntoTrack(locoID, trackID))
		{
			// FIXME: bether errormessage
			SendAndPrompt("Unknown loco or unknown track");
		}
	}

	void ConsoleClient::HandleLocoDelete(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.locoDelete(locoID))
		{
			SendAndPrompt("Loco not found or loco in use");
			return;
		}
		SendAndPrompt("Loco deleted");
	}

	void ConsoleClient::HandleLocoList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		// list one loco
		locoID_t locoID = ReadNumber(s, i);
		datamodel::Loco* loco = manager.GetLoco(locoID);
		if (loco == nullptr)
		{
			SendAndPrompt("Unknown loco");
			return;
		}
		stringstream status;
		status
			<< "Loco ID:  " << locoID
			<< "\nName:     " << loco->name
			<< "\nSpeed:    " << manager.LocoSpeed(locoID)
			<< "\nControl:  " << manager.getControlName(loco->controlID)
			<< "\nProtocol: " << protocolSymbols[loco->protocol]
			<< "\nAddress:  " << loco->address;
		const char* const locoStateText = loco->GetStateText();
		status << "\nStatus:   " << locoStateText;
		status << "\nTrack:    ";
		if (loco->GetTrack() == TrackNone)
		{
			status << "-";
		}
		else
		{
			status << manager.GetTrackName(loco->GetTrack()) << " (" << loco->GetTrack() << ")";
		}
		status << "\nStreet:   ";
		if (loco->GetStreet() == StreetNone)
		{
			status << "-";
		}
		else
		{
			status << manager.getStreetName(loco->GetStreet()) << " (" << loco->GetStreet() << ")";
		}
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleLocoManualmode(string& s, size_t& i)
	{
		if (ReadCharacterWithoutEating(s, i) == 'a')
		{
			// set all locos to manual mode
			manager.locoStopAll();
			return;
		}
		// set specific loco to manual mode
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.LocoStop(locoID))
		{
			// FIXME: bether errormessage
			SendAndPrompt("Unknown loco");
		}
	}

	void ConsoleClient::HandleLocoNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		controlID_t control = ReadNumber(s, i);
		protocol_t protocol = static_cast<protocol_t>(ReadNumber(s, i));
		address_t address = ReadNumber(s, i);
		function_t functions = ReadNumber(s, i);
		string result;
		if (!manager.locoSave(LocoNone, name, control, protocol, address, functions, result))
		{
			SendAndPrompt(result);
			return;
		}
		stringstream status;
		status << "Loco \"" << name << "\" added";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleLocoSpeed(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		locoSpeed_t speed = ReadNumber(s, i);
		if (!manager.LocoSpeed(ControlTypeConsole, locoID, speed))
		{
			// FIXME: bether errormessage
			SendAndPrompt("Unknown loco");
		}
	}

	void ConsoleClient::HandleLocoRelease(string& s, size_t& i)
	{
		locoID_t locoID = ReadNumber(s, i);
		if (!manager.LocoRelease(locoID))
		{
			// FIXME: bether errormessage
			SendAndPrompt("Loco not found or track in use");
			return;
		}
		SendAndPrompt("Loco released");
	}

	void ConsoleClient::HandleHelp()
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
		SendAndPrompt(status);
	}

	void ConsoleClient::HandlePrintLayout()
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
			track.second->Position(posX, posY, posZ, w, h, r);
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
			mySwitch.second->Position(posX, posY, posZ, w, h, r);
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
			accessory.second->Position(posX, posY, posZ, w, h, r);
			if (posZ != 0)
			{
				continue;
			}
			status << "\033[" << (int)(posY + 2) << ";" << (int)(posX + 1) << "H";
			status << "A";
		}
		// print cursor at correct position
		status << "\033[20;0H";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleQuit()
	{
		SendAndPrompt("Quit railcontrol console");
		connection->Terminate();
	}

	void ConsoleClient::HandleShutdown()
	{
		SendAndPrompt("Shutting down railcontrol");
		stopRailControlConsole();
		connection->Terminate();
	}

	void ConsoleClient::HandleStreetDelete(string& s, size_t& i)
	{
		streetID_t streetID = ReadNumber(s, i);
		if (!manager.streetDelete(streetID))
		{
			SendAndPrompt("Street not found or street in use");
			return;
		}
		SendAndPrompt("Street deleted");
	}

	void ConsoleClient::HandleStreetList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		// list one street
		streetID_t streetID = ReadNumber(s, i);
		datamodel::Street* street = manager.GetStreet(streetID);
		if (street == nullptr)
		{
			SendAndPrompt("Unknown street");
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
			status << manager.GetTrackName(street->fromTrack) << " (" << street->fromTrack << ") " << (street->fromDirection ? ">" : "<");
		}
		status << "\nEnd:      ";
		if (street->toTrack == TrackNone)
		{
			status << "-";
		}
		else
		{
			status << manager.GetTrackName(street->toTrack) << " (" << street->toTrack << ") " << (street->toDirection ? ">" : "<");
		}
		string stateText;
		text::Converters::lockStatus(street->GetState(), stateText);
		status << "\nStatus:   " << stateText;
		status << "\nLoco:     ";
		if (street->GetLoco() == LocoNone)
		{
			status << "-";
		}
		else
		{
			status << manager.LocoName(street->GetLoco()) << " (" << street->GetLoco() << ")";
		}
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleStreetNew(string& s, size_t& i)
	{
		string name = ReadText(s, i);
		trackID_t fromTrack = ReadNumber(s, i);
		direction_t fromDirection = ReadDirection(s, i);
		trackID_t toTrack = ReadNumber(s, i);
		direction_t toDirection = ReadDirection(s, i);
		feedbackID_t feedbackID = ReadNumber(s, i);
		string result;
		std::vector<datamodel::Relation*> relations;
		if (!manager.StreetSave(StreetNone, name, 250, relations, VisibleNo, 0, 0, 0, AutomodeNo, fromTrack, fromDirection, toTrack, toDirection, feedbackID, result))
		{
			SendAndPrompt(result);
			return;
		}
		stringstream status;
		status << "Street \"" << name << "\" added";
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleStreetRelease(string& s, size_t& i)
	{
		streetID_t streetID = ReadNumber(s, i);
		if (!manager.streetRelease(streetID))
		{
			SendAndPrompt("Street not found or track in use");
			return;
		}
		SendAndPrompt("Street released");
	}

	void ConsoleClient::HandleSwitchDelete(string& s, size_t& i)
	{
		switchID_t switchID = ReadNumber(s, i);
		if (!manager.switchDelete(switchID))
		{
			SendAndPrompt("Switch not found or switch in use");
			return;
		}
		SendAndPrompt("Switch deleted");
	}

	void ConsoleClient::HandleSwitchList(string& s, size_t& i)
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
			SendAndPrompt(status.str());
			return;
		}

		// list one switch
		switchID_t switchID = ReadNumber(s, i);
		datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		if (mySwitch == nullptr)
		{
			SendAndPrompt("Unknown switch");
			return;
		}
		stringstream status;
		status
			<< "Switch ID " << switchID
			<< "\nName:     " << mySwitch->name
			<< "\nX:        " << static_cast<int>(mySwitch->posX)
			<< "\nY:        " << static_cast<int>(mySwitch->posY)
			<< "\nZ:        " << static_cast<int>(mySwitch->posZ)
			<< "\nRotation: " << datamodel::LayoutItem::Rotation(mySwitch->rotation);
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
		SendAndPrompt(status.str());
	}

	void ConsoleClient::HandleSwitchNew(string& s, size_t& i)
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
			SendAndPrompt(result);
			return;
		}
		stringstream status;
		status << "Switch \"" << name << "\" added";
		SendAndPrompt(status.str());
	}
}; // namespace console
