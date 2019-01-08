#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "datamodel/datamodel.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webclient.h"
#include "webserver/webserver.h"
#include "webserver/HtmlFullResponse.h"
#include "webserver/HtmlResponse.h"
#include "webserver/HtmlResponseNotFound.h"
#include "webserver/HtmlResponseNotImplemented.h"
#include "webserver/HtmlTagAccessory.h"
#include "webserver/HtmlTagButtonCancel.h"
#include "webserver/HtmlTagButtonCommand.h"
#include "webserver/HtmlTagButtonCommandToggle.h"
#include "webserver/HtmlTagButtonOK.h"
#include "webserver/HtmlTagButtonPopup.h"
#include "webserver/HtmlTagInputCheckbox.h"
#include "webserver/HtmlTagInputHidden.h"
#include "webserver/HtmlTagInputSliderLocoSpeed.h"
#include "webserver/HtmlTagInputTextWithLabel.h"
#include "webserver/HtmlTagSelect.h"
#include "webserver/HtmlTagSwitch.h"
#include "webserver/HtmlTagTrack.h"

using std::map;
using std::stoi;
using std::string;
using std::stringstream;
using std::thread;
using std::to_string;
using std::vector;
using datamodel::Loco;

namespace webserver
{
	WebClient::~WebClient()
	{
		run = false;
		clientThread.join();
		connection->Terminate();
	}

	// worker is the thread that handles client requests
	void WebClient::Worker()
	{
		logger->Info("HTTP connection {0}: open", id);
		WorkerImpl();
		logger->Info("HTTP connection {0}: close", id);
	}

	void WebClient::WorkerImpl()
	{
		run = true;
		bool keepalive = true;

		while (run && keepalive)
		{
			char buffer_in[1024];
			memset(buffer_in, 0, sizeof(buffer_in));

			size_t pos = 0;
			string s;
			while (pos < sizeof(buffer_in) - 1 && s.find("\n\n") == string::npos && run)
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

			vector<string> lines;
			str_split(s, string("\n"), lines);

			if (lines.size() <= 1)
			{
				logger->Info("HTTP connection {0}: Ignoring invalid request", id);
				return;
			}

			string method;
			string uri;
			string protocol;
			map<string, string> arguments;
			map<string, string> headers;
			interpretClientRequest(lines, method, uri, protocol, arguments, headers);
			keepalive = (GetStringMapEntry(headers, "Connection", "close").compare("keep-alive") == 0);
			logger->Info("HTTP connection {0}: Request {1} {2}", id, method, uri);

			// if method is not implemented
			if ((method.compare("GET") != 0) && (method.compare("HEAD") != 0))
			{
				logger->Info("HTTP connection {0}: HTTP method {1} not implemented", id, method);
				HtmlResponseNotImplemented response(method);
				connection->Send(response);
				return;
			}

			/*
			 for (auto argument : arguments) {
			 xlog("Argument: %s=%s", argument.first.c_str(), argument.second.c_str());
			 }
			 for (auto header : headers) {
			 xlog("Header: %s=%s", header.first.c_str(), header.second.c_str());
			 }
			 */

			// handle requests
			if (arguments["cmd"].compare("quit") == 0)
			{
				HtmlReplyWithHeader(string("Stopping Railcontrol"));
				manager.booster(ControlTypeWebserver, BoosterStop);
				stopRailControlWebserver();
			}
			else if (arguments["cmd"].compare("booster") == 0)
			{
				bool on = GetBoolMapEntry(arguments, "on");
				if (on)
				{
					HtmlReplyWithHeader(string("Turning booster on"));
					manager.booster(ControlTypeWebserver, BoosterGo);
				}
				else
				{
					HtmlReplyWithHeader(string("Turning booster off"));
					manager.booster(ControlTypeWebserver, BoosterStop);
				}
			}
			else if (arguments["cmd"].compare("controledit") == 0)
			{
				handleControlEdit(arguments);
			}
			else if (arguments["cmd"].compare("controlsave") == 0)
			{
				handleControlSave(arguments);
			}
			else if (arguments["cmd"].compare("loco") == 0)
			{
				printLoco(arguments);
			}
			else if (arguments["cmd"].compare("locospeed") == 0)
			{
				handleLocoSpeed(arguments);
			}
			else if (arguments["cmd"].compare("locodirection") == 0)
			{
				handleLocoDirection(arguments);
			}
			else if (arguments["cmd"].compare("locofunction") == 0)
			{
				handleLocoFunction(arguments);
			}
			else if (arguments["cmd"].compare("locoedit") == 0)
			{
				handleLocoEdit(arguments);
			}
			else if (arguments["cmd"].compare("locosave") == 0)
			{
				handleLocoSave(arguments);
			}
			else if (arguments["cmd"].compare("protocol") == 0)
			{
				handleProtocol(arguments);
			}
			else if (arguments["cmd"].compare("layout") == 0)
			{
				handleLayout(arguments);
			}
			else if (arguments["cmd"].compare("accessoryedit") == 0)
			{
				handleAccessoryEdit(arguments);
			}
			else if (arguments["cmd"].compare("accessorysave") == 0)
			{
				handleAccessorySave(arguments);
			}
			else if (arguments["cmd"].compare("accessorystate") == 0)
			{
				handleAccessoryState(arguments);
			}
			else if (arguments["cmd"].compare("accessoryaskdelete") == 0)
			{
				handleAccessoryAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("accessorydelete") == 0)
			{
				handleAccessoryDelete(arguments);
			}
			else if (arguments["cmd"].compare("accessoryget") == 0)
			{
				handleAccessoryGet(arguments);
			}
			else if (arguments["cmd"].compare("switchedit") == 0)
			{
				handleSwitchEdit(arguments);
			}
			else if (arguments["cmd"].compare("switchsave") == 0)
			{
				handleSwitchSave(arguments);
			}
			else if (arguments["cmd"].compare("switchstate") == 0)
			{
				handleSwitchState(arguments);
			}
			else if (arguments["cmd"].compare("switchaskdelete") == 0)
			{
				handleSwitchAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("switchdelete") == 0)
			{
				handleSwitchDelete(arguments);
			}
			else if (arguments["cmd"].compare("switchget") == 0)
			{
				handleSwitchGet(arguments);
			}
			else if (arguments["cmd"].compare("trackedit") == 0)
			{
				handleTrackEdit(arguments);
			}
			else if (arguments["cmd"].compare("tracksave") == 0)
			{
				handleTrackSave(arguments);
			}
			else if (arguments["cmd"].compare("trackaskdelete") == 0)
			{
				handleTrackAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("trackdelete") == 0)
			{
				handleTrackDelete(arguments);
			}
			else if (arguments["cmd"].compare("trackget") == 0)
			{
				handleTrackGet(arguments);
			}
			else if (arguments["cmd"].compare("updater") == 0)
			{
				handleUpdater(headers);
			}
			else if (uri.compare("/") == 0)
			{
				printMainHTML();
			}
			else
			{
				deliverFile(uri);
			}
		}
	}

	int WebClient::stop()
	{
		// inform working thread to stop
		run = false;
		return 0;
	}

	char WebClient::ConvertHexToInt(char c)
	{
		if (c >= 'a')
		{
			c -= 'a' - 10;
		}
		else if (c >= 'A')
		{
			c -= 'A' - 10;
		}
		else if (c >= '0')
		{
			c -= '0';
		}

		if (c > 15)
		{
			return 0;
		}

		return c;
	}

	void WebClient::UrlDecode(string& argumentValue)
	{
		// decode %20 and similar
		while (true)
		{
			size_t pos = argumentValue.find('%');
			if (pos == string::npos || pos + 3 > argumentValue.length())
			{
				break;
			}
			char c = ConvertHexToInt(argumentValue[pos + 1]) * 16 + ConvertHexToInt(argumentValue[pos + 2]);
			argumentValue.replace(pos, 3, 1, c);
		}
	}

	void WebClient::interpretClientRequest(const vector<string>& lines, string& method, string& uri, string& protocol, map<string,string>& arguments, map<string,string>& headers)
	{
		if (lines.size() == 0)
		{
			return;
		}

		for (auto line : lines)
		{
			if (line.find("HTTP/1.") == string::npos)
			{
				vector<string> list;
				str_split(line, string(": "), list);
				if (list.size() == 2)
				{
					headers[list[0]] = list[1];
				}
				continue;
			}

			vector<string> list;
			str_split(line, string(" "), list);
			if (list.size() != 3)
			{
				continue;
			}

			method = list[0];
			// transform method to uppercase
			std::transform(method.begin(), method.end(), method.begin(), ::toupper);

			// if method == HEAD set membervariable
			headOnly = method.compare("HEAD") == 0;

			// set uri and protocol
			uri = list[1];
			UrlDecode(uri);
			protocol = list[2];

			// read GET-arguments from uri
			vector<string> uri_parts;
			str_split(uri, "?", uri_parts);
			if (uri_parts.size() != 2)
			{
				continue;
			}

			vector<string> argumentStrings;
			str_split(uri_parts[1], "&", argumentStrings);
			for (auto argument : argumentStrings)
			{
				vector<string> argumentParts;
				str_split(argument, "=", argumentParts);
				arguments[argumentParts[0]] = argumentParts[1];
			}
		}
	}

	void WebClient::deliverFile(const string& virtualFile)
	{
		stringstream ss;
		char workingDir[128];
		if (getcwd(workingDir, sizeof(workingDir)))
		{
			ss << workingDir << "/html" << virtualFile;
		}
		string sFile = ss.str();
		const char* realFile = sFile.c_str();
		FILE* f = fopen(realFile, "r");
		if (f == nullptr)
		{
			HtmlResponseNotFound response(virtualFile);
			connection->Send(response);
			return;
		}

		deliverFileInternal(f, realFile, virtualFile);
		fclose(f);
	}

	void WebClient::deliverFileInternal(FILE* f, const char* realFile, const string& virtualFile)
	{
		struct stat s;
		int rc = stat(realFile, &s);
		if (rc != 0)
		{
			return;
		}

		size_t length = virtualFile.length();
		const char* contentType = nullptr;
		if (length > 4 && virtualFile[length - 4] == '.')
		{
			if (virtualFile[length - 3] == 'i' && virtualFile[length - 2] == 'c' && virtualFile[length - 1] == 'o')
			{
				contentType = "image/x-icon";
			}
			else if (virtualFile[length - 3] == 'c' && virtualFile[length - 2] == 's' && virtualFile[length - 1] == 's')
			{
				contentType = "text/css";
			}
			else if (virtualFile[length - 3] == 'p' && virtualFile[length - 2] == 'n' && virtualFile[length - 1] == 'g')
			{
				contentType = "image/png";
			}
			else if (virtualFile[length - 3] == 't' && virtualFile[length - 2] == 't' && virtualFile[length - 1] == 'f')
			{
				contentType = "application/x-font-ttf";
			}
		}
		else if (length > 3 && virtualFile[length - 3] == '.' && virtualFile[length - 2] == 'j' && virtualFile[length - 1] == 's')
		{
			contentType = "application/javascript";
		}

		Response response(Response::OK, HtmlTag());
		response.AddHeader("Cache-Control", "max-age=3600");
		response.AddHeader("Content-Length", to_string(s.st_size));
		if (contentType != nullptr)
		{
			response.AddHeader("Content-Type", contentType);
		}
		connection->Send(response);

		if (headOnly == true)
		{
			return;
		}

		char* buffer = static_cast<char*>(malloc(s.st_size));
		if (buffer == nullptr)
		{
			return;
		}

		size_t r = fread(buffer, 1, s.st_size, f);
		connection->Send(buffer, r, 0);
		free(buffer);
	}

	HtmlTag WebClient::ControlArgumentTag(unsigned char argNr, argumentType_t type, string& value)
	{
		string argumentName;
		switch (type)
		{
			case IpAddress:
				argumentName = "IP Address:";
				break;

			case SerialPort:
				argumentName = "Serial Port:";
				break;

			case S88Modules:
				argumentName = "# of S88 Modules (8 port):";
				break;
		}
		return HtmlTagInputTextWithLabel("arg" + to_string(argNr), argumentName, value);
	}

	void WebClient::handleControlEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		controlID_t controlID = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		hardwareType_t hardwareType = HardwareTypeNone;
		string name("New Control");
		string arg1;
		string arg2;
		string arg3;
		string arg4;
		string arg5;

		if (controlID != ControlIdNone)
		{
			hardware::HardwareParams* params = manager.getHardware(controlID);
			if (params != nullptr)
			{
				hardwareType = params->hardwareType;
				name = params->name;
				arg1 = params->arg1;
				arg2 = params->arg2;
				arg3 = params->arg3;
				arg4 = params->arg4;
				arg5 = params->arg5;
			}
		}

		const std::map<hardwareType_t,string> hardwares = manager.hardwareListNames();
		std::map<string, string> hardwareOptions;
		for(auto hardware : hardwares)
		{
			hardwareOptions[to_string(hardware.first)] = hardware.second;
		}

		content.AddContent(HtmlTag("h1").AddContent("Edit control &quot;" + name + "&quot;"));
		HtmlTag form("form");
		form.AddAttribute("id", "editform");
		form.AddContent(HtmlTagInputHidden("cmd", "controlsave"));
		form.AddContent(HtmlTagInputHidden("control", to_string(controlID)));
		form.AddContent(HtmlTagInputTextWithLabel("name", "Control Name:", name));
		form.AddContent(HtmlTagLabel("Hardware type:", "hardwaretype"));
		form.AddContent(HtmlTagSelect("hardwaretype", hardwareOptions, to_string(hardwareType)));
		std::map<unsigned char,argumentType_t> argumentTypes = manager.ArgumentTypesOfControl(controlID);
		if (argumentTypes.count(1) == 1)
		{
			form.AddContent(ControlArgumentTag(1, argumentTypes.at(1), arg1));
		}
		if (argumentTypes.count(2) == 1)
		{
			form.AddContent(ControlArgumentTag(2, argumentTypes.at(2), arg2));
		}
		if (argumentTypes.count(3) == 1)
		{
			form.AddContent(ControlArgumentTag(3, argumentTypes.at(3), arg3));
		}
		if (argumentTypes.count(4) == 1)
		{
			form.AddContent(ControlArgumentTag(4, argumentTypes.at(4), arg4));
		}
		if (argumentTypes.count(5) == 1)
		{
			form.AddContent(ControlArgumentTag(5, argumentTypes.at(5), arg5));
		}
		content.AddContent(form);
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleControlSave(const map<string, string>& arguments)
	{
		stringstream ss;
		controlID_t controlID = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		string name = GetStringMapEntry(arguments, "name");
		hardwareType_t hardwareType = static_cast<hardwareType_t>(GetIntegerMapEntry(arguments, "hardwaretype", HardwareTypeNone));
		string arg1 = GetStringMapEntry(arguments, "arg1");
		string arg2 = GetStringMapEntry(arguments, "arg2");
		string arg3 = GetStringMapEntry(arguments, "arg3");
		string arg4 = GetStringMapEntry(arguments, "arg4");
		string arg5 = GetStringMapEntry(arguments, "arg5");
		string result;

		if (!manager.controlSave(controlID, hardwareType, name, arg1, arg2, arg3, arg4, arg5, result))
		{
			ss << result;
		}
		else
		{
			ss << "Control &quot;" << name << "&quot; saved.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleLocoSpeed(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		LocoSpeed speed = GetIntegerMapEntry(arguments, "speed", MinSpeed);

		manager.locoSpeed(ControlTypeWebserver, locoID, speed);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; is now set to speed " << speed;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoDirection(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		direction_t direction = (GetBoolMapEntry(arguments, "on") ? DirectionRight : DirectionLeft);

		manager.locoDirection(ControlTypeWebserver, locoID, direction);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; is now set to " << direction;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoFunction(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		function_t function = GetIntegerMapEntry(arguments, "function", 0);
		bool on = GetBoolMapEntry(arguments, "on");

		manager.locoFunction(ControlTypeWebserver, locoID, function, on);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; has now f";
		ss << function << " set to " << (on ? "on" : "off");
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		controlID_t controlID = ControlIdNone;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		string name("New Loco");
		function_t nrOfFunctions = 0;
		if (locoID > LocoNone)
		{
			const datamodel::Loco* loco = manager.getLoco(locoID);
			controlID = loco->controlID;
			protocol = loco->protocol;
			address = loco->address;
			name = loco->name;
			nrOfFunctions = loco->GetNrOfFunctions();
		}

		std::map<controlID_t,string> controls = manager.controlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlIdNone)
			{
				controlID = control.first;
			}
		}

		std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
		std::map<string, string> protocolOptions;
		for(auto protocol : protocols)
		{
			protocolOptions[to_string(protocol.first)] = protocol.second;
		}

		std::map<string, string> functionOptions;
		for(function_t i = 0; i <= datamodel::LocoFunctions::maxFunctions; ++i)
		{
			functionOptions[toStringWithLeadingZeros(i, 2)] = to_string(i);
		}

		content.AddContent(HtmlTag("h1").AddContent("Edit loco &quot;" + name + "&quot;"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "locosave"))
			.AddContent(HtmlTagInputHidden("loco", to_string(locoID)))
			.AddContent(HtmlTagInputTextWithLabel("name", "Loco Name:", name))
			.AddContent(HtmlTagLabel("Control:", "control"))
			.AddContent(HtmlTagSelect("control", controlOptions, to_string(controlID)))
			.AddContent(HtmlTagLabel("Protocol:", "protocol"))
			.AddContent(HtmlTagSelect("protocol", protocolOptions, to_string(protocol)))
			.AddContent(HtmlTagInputTextWithLabel("address", "Address:", to_string(address)))
			.AddContent(HtmlTagLabel("# of functions:", "function"))
			.AddContent(HtmlTagSelect("function", functionOptions, toStringWithLeadingZeros(nrOfFunctions, 2)))
			);
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLocoSave(const map<string, string>& arguments)
	{
		stringstream ss;
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		string name = GetStringMapEntry(arguments, "name");
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		protocol_t protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address_t address = GetIntegerMapEntry(arguments, "address", AddressNone);
		function_t nrOfFunctions = GetIntegerMapEntry(arguments, "function", 0);
		string result;

		if (!manager.locoSave(locoID, name, controlId, protocol, address, nrOfFunctions, result))
		{
			ss << result;
		}
		else
		{
			ss << "Loco &quot;" << name << "&quot; saved.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleProtocol(const map<string, string>& arguments)
	{
		stringstream ss;
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		if (controlId > ControlIdNone)
		{
			ss << "<label>Protocol:</label>";
			protocol_t selectedProtocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
			std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlId);
			std::map<string,string> protocolsTextMap;
			for(auto protocol : protocols)
			{
				protocolsTextMap[to_string(protocol.first)] = protocol.second;
			}
			ss << HtmlTagSelect("protocol", protocolsTextMap, to_string(selectedProtocol));
		}
		else
		{
			ss << "Unknown control";
		}
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	HtmlTag WebClient::selectLayout()
	{
		map<string,string> options;
		// FIXME: select layers with content
		options["0"] = "Layer 0";
		return HtmlTag("form").AddAttribute("method", "get").AddAttribute("action", "/").AddAttribute("id", "selectLayout_form")
		.AddContent(HtmlTagSelect("layout", options).AddAttribute("onchange", "loadDivFromForm('selectLayout_form', 'layout')"))
		.AddContent(HtmlTagInputHidden("cmd", "layout"));
	}

	void WebClient::handleLayout(const map<string, string>& arguments)
	{
		layoutPosition_t layer = static_cast<layoutPosition_t>(GetIntegerMapEntry(arguments, "layer", 0));
		HtmlTag content;
		const map<accessoryID_t,datamodel::Accessory*>& accessories = manager.accessoryList();
		for (auto accessory : accessories)
		{
			if (accessory.second->posZ != layer)
			{
				continue;
			}
			content.AddChildTag(HtmlTagAccessory(accessory.second));
		}

		const map<switchID_t,datamodel::Switch*>& switches = manager.switchList();
		for (auto mySwitch : switches)
		{
			if (mySwitch.second->posZ != layer)
			{
				continue;
			}
			content.AddChildTag(HtmlTagSwitch(mySwitch.second));
		}

		const map<switchID_t,datamodel::Track*>& tracks = manager.trackList();
		for (auto track : tracks)
		{
			if (track.second->posZ != layer)
			{
				continue;
			}
			content.AddChildTag(HtmlTagTrack(track.second));
		}
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleAccessoryEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		controlID_t controlID = ControlIdNone;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		string name("New Accessory");
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		// FIXME: layers not supported yet: layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		accessoryTimeout_t timeout = 100;
		bool inverted = false;
		if (accessoryID > AccessoryNone)
		{
			const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
			controlID = accessory->controlID;
			protocol = accessory->protocol;
			address = accessory->address;
			name = accessory->name;
			posx = accessory->posX;
			posy = accessory->posY;
			// FIXME: layers not supported yet: posz = accessory->posZ;
			inverted = accessory->IsInverted();
		}

		std::map<controlID_t,string> controls = manager.controlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlIdNone)
			{
				controlID = control.first;
			}
		}

		std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
		std::map<string, string> protocolOptions;
		for(auto protocol : protocols)
		{
			protocolOptions[to_string(protocol.first)] = protocol.second;
		}

		std::map<string, string> positionOptions;
		for(int i = 0; i < 50; ++i)
		{
			positionOptions[toStringWithLeadingZeros(i, 2)] = to_string(i);
		}

		std::map<string, string> timeoutOptions;
		timeoutOptions["0000"] = "0";
		timeoutOptions["0100"] = "100";
		timeoutOptions["0250"] = "250";
		timeoutOptions["1000"] = "1000";

		content.AddContent(HtmlTag("h1").AddContent("Edit accessory &quot;" + name + "&quot;"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "accessorysave"))
			.AddContent(HtmlTagInputHidden("accessory", to_string(accessoryID)))
			.AddContent(HtmlTagInputTextWithLabel("name", "Accessory Name:", name))
			.AddContent(HtmlTagLabel("Control:", "control"))
			.AddContent(HtmlTagSelect("control", controlOptions, to_string(controlID)))
			.AddContent(HtmlTagLabel("Protocol:", "protocol"))
			.AddContent(HtmlTagSelect("protocol", protocolOptions, to_string(protocol)))
			.AddContent(HtmlTagInputTextWithLabel("address", "Address:", to_string(address)))
			.AddContent(HtmlTagLabel("Pos X:", "posx"))
			.AddContent(HtmlTagSelect("posx", positionOptions, toStringWithLeadingZeros(posx, 2)))
			.AddContent(HtmlTagLabel("Pos Y:", "posy"))
			.AddContent(HtmlTagSelect("posy", positionOptions, toStringWithLeadingZeros(posy, 2)))
			/* FIXME: layers not supported
			.AddContent(HtmlTagLabel("Pos Z:", "posz"))
			.AddContent(HtmlTagSelect("posz", positionOptions, toStringWithLeadingZeros(posz, 2)))
			*/
			.AddContent(HtmlTagLabel("Timeout:", "timeout"))
			.AddContent(HtmlTagSelect("timeout", timeoutOptions, toStringWithLeadingZeros(timeout, 4)))
			.AddContent(HtmlTagLabel("Inverted:", "inverted"))
			.AddContent(HtmlTagInputCheckbox("inverted", "true", inverted))
		);
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleAccessoryGet(const map<string, string>& arguments)
	{
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory");
		const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		HtmlReplyWithHeader(HtmlTagAccessory(accessory));
	}

	void WebClient::handleAccessorySave(const map<string, string>& arguments)
	{
		stringstream ss;
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		string name = GetStringMapEntry(arguments, "name");
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		protocol_t protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address_t address = GetIntegerMapEntry(arguments, "address", AddressNone);
		layoutPosition_t posX = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posY = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posZ = GetIntegerMapEntry(arguments, "posz", 0);
		accessoryTimeout_t timeout = GetIntegerMapEntry(arguments, "timeout", 100);
		bool inverted = GetBoolMapEntry(arguments, "inverted");
		string result;
		if (!manager.accessorySave(accessoryID, name, posX, posY, posZ, controlId, protocol, address, AccessoryTypeDefault, timeout, inverted, result))
		{
			ss << result;
		}
		else
		{
			ss << "Accessory &quot;" << name << "&quot; saved.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleAccessoryState(const map<string, string>& arguments)
	{
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		accessoryState_t accessoryState = (GetStringMapEntry(arguments, "state", "off").compare("off") == 0 ? AccessoryStateOff : AccessoryStateOn);

		manager.accessory(ControlTypeWebserver, accessoryID, accessoryState);

		stringstream ss;
		ss << "Accessory &quot;" << manager.getAccessoryName(accessoryID) << "&quot; is now set to " << accessoryState;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleAccessoryAskDelete(const map<string, string>& arguments)
	{
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);

		if (accessoryID == AccessoryNone)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unknown accessory"));
			return;
		}

		const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unknown accessory"));
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete accessory &quot;" + accessory->name + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the accessory &quot;" + accessory->name + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "accessorydelete"))
			.AddContent(HtmlTagInputHidden("accessory", to_string(accessoryID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleAccessoryDelete(const map<string, string>& arguments)
	{
		stringstream ss;
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unable to delete accessory"));
			return;
		}

		string name = accessory->name;

		if (!manager.accessoryDelete(accessoryID))
		{
			ss << "Unable to delete accessory";
		}
		else
		{
			ss << "Accessory &quot;" << name << "&quot; deleted.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleSwitchEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);
		controlID_t controlID = ControlIdNone;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		string name("New Switch");
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		// FIXME: layers not supported yet: layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		switchType_t type = SwitchTypeLeft;
		accessoryTimeout_t timeout = 100;
		bool inverted = false;
		if (switchID > SwitchNone)
		{
			const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
			controlID = mySwitch->controlID;
			protocol = mySwitch->protocol;
			address = mySwitch->address;
			name = mySwitch->name;
			posx = mySwitch->posX;
			posy = mySwitch->posY;
			// FIXME: layers not supported yet: posz = mySwitch->posZ;
			rotation = mySwitch->rotation;
			type = mySwitch->GetType();
			timeout = mySwitch->timeout;
			inverted = mySwitch->IsInverted();
		}

		std::map<controlID_t,string> controls = manager.controlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlIdNone)
			{
				controlID = control.first;
			}
		}

		std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
		std::map<string, string> protocolOptions;
		for(auto protocol : protocols)
		{
			protocolOptions[to_string(protocol.first)] = protocol.second;
		}

		std::map<string, string> positionOptions;
		for(int i = 0; i < 50; ++i)
		{
			positionOptions[toStringWithLeadingZeros(i, 2)] = to_string(i);
		}

		std::map<string, string> rotationOptions;
		rotationOptions[to_string(Rotation0)] = "none";
		rotationOptions[to_string(Rotation90)] = "90 deg clockwise";
		rotationOptions[to_string(Rotation180)] = "180 deg";
		rotationOptions[to_string(Rotation270)] = "90 deg anti-clockwise";

		std::map<string, string> typeOptions;
		typeOptions[to_string(SwitchTypeLeft)] = "Left";
		typeOptions[to_string(SwitchTypeRight)] = "Right";

		std::map<string, string> timeoutOptions;
		timeoutOptions["0000"] = "0";
		timeoutOptions["0100"] = "100";
		timeoutOptions["0250"] = "250";
		timeoutOptions["1000"] = "1000";

		content.AddContent(HtmlTag("h1").AddContent("Edit switch &quot;" + name + "&quot;"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "switchsave"))
			.AddContent(HtmlTagInputHidden("switch", to_string(switchID)))
			.AddContent(HtmlTagInputTextWithLabel("name", "Switch Name:", name))
			.AddContent(HtmlTagLabel("Control:", "control"))
			.AddContent(HtmlTagSelect("control", controlOptions, to_string(controlID)))
			.AddContent(HtmlTagLabel("Protocol:", "protocol"))
			.AddContent(HtmlTagSelect("protocol", protocolOptions, to_string(protocol)))
			.AddContent(HtmlTagInputTextWithLabel("address", "Address:", to_string(address)))
			.AddContent(HtmlTagLabel("Pos X:", "posx"))
			.AddContent(HtmlTagSelect("posx", positionOptions, toStringWithLeadingZeros(posx, 2)))
			.AddContent(HtmlTagLabel("Pos Y:", "posy"))
			.AddContent(HtmlTagSelect("posy", positionOptions, toStringWithLeadingZeros(posy, 2)))
			/* FIXME: layers not supported
			.AddContent(HtmlTagLabel("Pos Z:", "posz"))
			.AddContent(HtmlTagSelect("posz", positionOptions, toStringWithLeadingZeros(posz, 2)))
			*/
			.AddContent(HtmlTagLabel("Rotation:", "rotation"))
			.AddContent(HtmlTagSelect("rotation", rotationOptions, to_string(rotation)))
			.AddContent(HtmlTagLabel("Type:", "type"))
			.AddContent(HtmlTagSelect("type", typeOptions, to_string(type)))
			.AddContent(HtmlTagLabel("Timeout:", "timeout"))
			.AddContent(HtmlTagSelect("timeout", timeoutOptions, toStringWithLeadingZeros(timeout, 4)))
			.AddContent(HtmlTagLabel("Inverted:", "inverted"))
			.AddContent(HtmlTagInputCheckbox("inverted", "true", inverted))
		);
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleSwitchSave(const map<string, string>& arguments)
	{
		stringstream ss;
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);
		string name = GetStringMapEntry(arguments, "name");
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		protocol_t protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address_t address = GetIntegerMapEntry(arguments, "address", AddressNone);
		layoutPosition_t posX = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posY = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posZ = GetIntegerMapEntry(arguments, "posz", 0);
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		switchType_t type = GetIntegerMapEntry(arguments, "type", SwitchTypeLeft);
		accessoryTimeout_t timeout = GetIntegerMapEntry(arguments, "timeout", 100);
		bool inverted = GetBoolMapEntry(arguments, "inverted");
		string result;
		if (!manager.switchSave(switchID, name, posX, posY, posZ, rotation, controlId, protocol, address, type, timeout, inverted, result))
		{
			ss << result;
		}
		else
		{
			ss << "Switch &quot;" << name << "&quot; saved.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleSwitchState(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);
		switchState_t switchState = (GetStringMapEntry(arguments, "state", "turnout").compare("turnout") == 0 ? SwitchStateTurnout : SwitchStateStraight);

		manager.handleSwitch(ControlTypeWebserver, switchID, switchState);

		stringstream ss;
		ss << "Switch &quot;" << manager.getSwitchName(switchID) << "&quot; is now set to " << switchState;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleSwitchAskDelete(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);

		if (switchID == SwitchNone)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unknown switch"));
			return;
		}

		const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unknown switch"));
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete switch &quot;" + mySwitch->name + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the switch &quot;" + mySwitch->name + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "switchdelete"))
			.AddContent(HtmlTagInputHidden("switch", to_string(switchID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleSwitchDelete(const map<string, string>& arguments)
	{
		stringstream ss;
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);
		const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unable to delete switch"));
			return;
		}

		string name = mySwitch->name;

		if (!manager.switchDelete(switchID))
		{
			ss << "Unable to delete switch";
		}
		else
		{
			ss << "switch &quot;" << name << "&quot; deleted.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleSwitchGet(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch");
		const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		HtmlReplyWithHeader(HtmlTagSwitch(mySwitch));
	}

	void WebClient::handleTrackEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		string name("New Track");
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		// FIXME: layers not supported yet: layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		layoutItemSize_t height = GetIntegerMapEntry(arguments, "length", 1);
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		trackType_t type = TrackTypeStraight;
		if (trackID > TrackNone)
		{
			const datamodel::Track* track = manager.getTrack(trackID);
			name = track->name;
			posx = track->posX;
			posy = track->posY;
			// FIXME: layers not supported yet: posz = track->posZ;
			height = track->height;
			rotation = track->rotation;
			type = track->Type();
		}

		std::map<string, string> positionOptions;
		for(int i = 0; i < 50; ++i)
		{
			positionOptions[toStringWithLeadingZeros(i, 2)] = to_string(i);
		}

		std::map<string, string> heightOptions;
		for(int i = 0; i < 5; ++i)
		{
			heightOptions[toStringWithLeadingZeros(i, 1)] = to_string(i);
		}

		std::map<string, string> rotationOptions;
		rotationOptions[to_string(Rotation0)] = "none";
		rotationOptions[to_string(Rotation90)] = "90 deg clockwise";
		rotationOptions[to_string(Rotation180)] = "180 deg";
		rotationOptions[to_string(Rotation270)] = "90 deg anti-clockwise";

		std::map<string, string> typeOptions;
		typeOptions[to_string(TrackTypeStraight)] = "Straight";
		typeOptions[to_string(TrackTypeLeft)] = "Left";
		typeOptions[to_string(TrackTypeRight)] = "Right";

		content.AddContent(HtmlTag("h1").AddContent("Edit track &quot;" + name + "&quot;"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "tracksave"))
			.AddContent(HtmlTagInputHidden("track", to_string(trackID)))
			.AddContent(HtmlTagInputTextWithLabel("name", "Track Name:", name))
			.AddContent(HtmlTagLabel("Pos X:", "posx"))
			.AddContent(HtmlTagSelect("posx", positionOptions, toStringWithLeadingZeros(posx, 2)))
			.AddContent(HtmlTagLabel("Pos Y:", "posy"))
			.AddContent(HtmlTagSelect("posy", positionOptions, toStringWithLeadingZeros(posy, 2)))
			/* FIXME: layers not supported
			.AddContent(HtmlTagLabel("Pos Z:", "posz"))
			.AddContent(HtmlTagSelect("posz", positionOptions, toStringWithLeadingZeros(posz, 2)))
			*/
			.AddContent(HtmlTagLabel("Length:", "length"))
			.AddContent(HtmlTagSelect("length", heightOptions, to_string(height)))
			.AddContent(HtmlTagLabel("Rotation:", "rotation"))
			.AddContent(HtmlTagSelect("rotation", rotationOptions, to_string(rotation)))
			.AddContent(HtmlTagLabel("Type:", "type"))
			.AddContent(HtmlTagSelect("type", typeOptions, to_string(type)))
		);
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleTrackSave(const map<string, string>& arguments)
	{
		stringstream ss;
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		string name = GetStringMapEntry(arguments, "name");
		layoutPosition_t posX = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posY = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posZ = GetIntegerMapEntry(arguments, "posz", 0);
		layoutItemSize_t height = GetIntegerMapEntry(arguments, "length", 1);
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		trackType_t type = GetIntegerMapEntry(arguments, "type", TrackTypeStraight);
		string result;
		if (!manager.trackSave(trackID, name, posX, posY, posZ, height, rotation, type, result))
		{
			ss << result;
		}
		else
		{
			ss << "Track &quot;" << name << "&quot; saved.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleTrackAskDelete(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);

		if (trackID == TrackNone)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unknown track"));
			return;
		}

		const datamodel::Track* track = manager.getTrack(trackID);
		if (track == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unknown track"));
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete track &quot;" + track->name + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the track &quot;" + track->name + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "trackdelete"))
			.AddContent(HtmlTagInputHidden("track", to_string(trackID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleTrackDelete(const map<string, string>& arguments)
	{
		stringstream ss;
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		const datamodel::Track* track = manager.getTrack(trackID);
		if (track == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Unable to delete track"));
			return;
		}

		string name = track->name;

		if (!manager.trackDelete(trackID))
		{
			ss << "Unable to delete track";
		}
		else
		{
			ss << "track &quot;" << name << "&quot; deleted.";
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent(ss.str()));
	}

	void WebClient::handleTrackGet(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track");
		const datamodel::Track* track = manager.getTrack(trackID);
		HtmlReplyWithHeader(HtmlTagTrack(track));
	}

	void WebClient::handleUpdater(const map<string, string>& headers)
	{
		Response response(Response::OK);
		response.AddHeader("Cache-Control", "no-cache, must-revalidate");
		response.AddHeader("Pragma", "no-cache");
		response.AddHeader("Expires", "Sun, 12 Feb 2016 00:00:00 GMT");
		response.AddHeader("Content-Type", "text/event-stream; charset=utf-8");
		int ret = connection->Send(response);
		if (ret <= 0)
		{
			return;
		}

		unsigned int updateID = GetIntegerMapEntry(headers, "Last-Event-ID", 1);
		while(run)
		{
			string s;
			bool ok = server.nextUpdate(updateID, s);
			if (ok == false)
			{
				// FIXME: use signaling instead of sleep
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			string reply("id: ");
			reply += to_string(updateID);
			reply += "\r\n";
			reply += s;
			reply += "\r\n\r\n";

			++updateID;

			ret = connection->Send(reply);
			if (ret < 0)
			{
				return;
			}
		}
	}

	void WebClient::HtmlReplyWithHeader(const HtmlTag& tag)
	{
		connection->Send(HtmlResponse(tag));
	}

	HtmlTag WebClient::selectLoco()
	{
		const map<locoID_t, Loco*>& locos = manager.locoList();
		map<string,string> options;
		for (auto locoTMP : locos) {
			Loco* loco = locoTMP.second;
			options[to_string(loco->objectID)] = loco->name;
		}
		return HtmlTag("form").AddAttribute("method", "get").AddAttribute("action", "/").AddAttribute("id", "selectLoco_form")
		.AddContent(HtmlTagSelect("loco", options).AddAttribute("onchange", "loadDivFromForm('selectLoco_form', 'loco')"))
		.AddContent(HtmlTagInputHidden("cmd", "loco"));
	}

	void WebClient::printLoco(const map<string, string>& arguments)
	{
		string content;
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		if (locoID > LocoNone)
		{
			stringstream ss;
			Loco* loco = manager.getLoco(locoID);
			ss << HtmlTag("p").AddContent(loco->name);
			unsigned int speed = loco->Speed();
			map<string,string> buttonArguments;
			buttonArguments["loco"] = to_string(locoID);

			string id = "locospeed_" + to_string(locoID);
			ss << HtmlTagInputSliderLocoSpeed("speed", "locospeed", MinSpeed, MaxSpeed, speed, locoID);
			buttonArguments["speed"] = "0";
			ss << HtmlTagButtonCommand("0%", id + "_0", buttonArguments);
			buttonArguments["speed"] = "255";
			ss << HtmlTagButtonCommand("25%", id + "_1", buttonArguments);
			buttonArguments["speed"] = "511";
			ss << HtmlTagButtonCommand("50%", id + "_2", buttonArguments);
			buttonArguments["speed"] = "767";
			ss << HtmlTagButtonCommand("75%", id + "_3", buttonArguments);
			buttonArguments["speed"] = "1023";
			ss << HtmlTagButtonCommand("100%", id + "_4", buttonArguments);
			buttonArguments.erase("speed");

			id = "locoedit_" + to_string(locoID);
			ss << HtmlTagButtonPopup(HtmlTag("span").AddClass("symbola").AddContent("&#x270D;"), id, buttonArguments);

			id = "locodirection_" + to_string(locoID);
			ss << HtmlTagButtonCommandToggle(HtmlTag("span").AddClass("symbola").AddContent("&#9193;"), id, loco->GetDirection(), buttonArguments).AddClass("button_direction");

			id = "locofunction_" + to_string(locoID);
			function_t nrOfFunctions = loco->GetNrOfFunctions();
			for (function_t nr = 0; nr <= nrOfFunctions; ++nr)
			{
				string nrText(to_string(nr));
				buttonArguments["function"] = nrText;
				ss << HtmlTagButtonCommandToggle("f" + nrText, id + "_" + nrText, loco->GetFunction(nr), buttonArguments);
			}
			buttonArguments.erase("function");
			content = ss.str();
		}
		else
		{
			content = "No locoID provided";
		}
		HtmlReplyWithHeader(HtmlTag().AddContent(content));
	}

	void WebClient::printMainHTML() {
		// handle base request
		HtmlTag body("body");
		body.AddAttribute("onload","startUp();");
		body.AddAttribute("id", "body");

		map<string,string> buttonArguments;

		HtmlTag menu("div");
		menu.AddClass("menu");
		menu.AddContent(HtmlTagButtonCommand("<svg width=\"35\" height=\"35\"><polygon points=\"15,0.5 30,0.5 30,24.5 15,24.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"20,10.5 30,0.5 30,24.5 20,34.5\" fill=\"black\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"0,10 7.5,10 7.5,5 15,12.5 7.5,20 7.5,15 0,15\"/></svg>", "quit"));
		menu.AddContent(HtmlTagButtonCommandToggle("<svg width=\"35\" height=\"35\"><polyline points=\"12.5,8.8 11.1,9.8 9.8,11.1 8.8,12.5 8.1,14.1 7.7,15.8 7.5,17.5 7.7,19.2 8.1,20.9 8.8,22.5 9.8,23.9 11.1,25.2 12.5,26.2 14.1,26.9 15.8,27.3 17.5,27.5 19.2,27.3 20.9,26.9 22.5,26.2 23.9,25.2 25.2,23.9 26.2,22.5 26.9,20.9 27.3,19.2 27.5,17.5 27.3,15.8 26.9,14.1 26.2,12.5 25.2,11.1 23.9,9.8 22.5,8.8\" stroke=\"black\" stroke-width=\"3\" fill=\"none\"/><polyline points=\"17.5,2.5 17.5,15\" stroke=\"black\" stroke-width=\"3\" fill=\"none\"/></svg>", "booster", false, buttonArguments).AddClass("button_booster"));
		menu.AddContent(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"10,0.5 25,0.5 25,34.5 10,34.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"13,3.5 22,3.5 22,7.5 13,7.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><circle cx=\"14.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"27.5\" r=\"5\" fill=\"black\"/></svg>", "controllist"));
		menu.AddContent(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"0,10 5,10 5,0 10,0 10,10 25,10 25,0 35,0 35,5 30,5 30,10 35,10 35,25 0,25\" fill=\"black\"/><circle cx=\"5\" cy=\"30\" r=\"5\" fill=\"black\"/><circle cx=\"17.5\" cy=\"30\" r=\"5\" fill=\"black\"/><circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"black\"/</svg>", "locolist"));
		body.AddChildTag(menu);

		body.AddChildTag(HtmlTag("div").AddClass("loco_selector").AddChildTag(selectLoco()));
		body.AddChildTag(HtmlTag("div").AddClass("layout_selector").AddChildTag(selectLayout()));
		body.AddChildTag(HtmlTag("div").AddClass("loco").AddAttribute("id", "loco"));
		body.AddChildTag(HtmlTag("div").AddClass("layout").AddAttribute("id", "layout"));
		body.AddChildTag(HtmlTag("div").AddClass("popup").AddAttribute("id", "popup"));
		body.AddChildTag(HtmlTag("div").AddClass("status").AddAttribute("id", "status"));

		body.AddChildTag(HtmlTagJavascript(
			"var updater = new EventSource('/?cmd=updater');"
			"updater.onmessage = function(e) {"
			" dataUpdate(e);"
			"};"));

		body.AddChildTag(HtmlTag("div").AddClass( "contextmenu").AddAttribute("id", "layout_context")
			.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add track").AddAttribute("onClick", "loadPopup('/?cmd=trackedit&track=0');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add switch").AddAttribute("onClick", "loadPopup('/?cmd=switchedit&switch=0');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add accessory").AddAttribute("onClick", "loadPopup('/?cmd=accessoryedit&accessory=0');"))
			));

		std::stringstream javascript;
		javascript << "$(function() {"
			" $('#layout').on('contextmenu', function(event) {"
			"  if (event.shiftKey) {"
			"   return true;"
			"  }"
			"  event.preventDefault();"
			"  hideAllContextMenus();"
			"  menu = document.querySelector('#layout_context');"
			"  if (menu) {"
			"  	menu.style.display = 'block';"
			"   menu.style.left = event.pageX + 'px';"
			"   menu.style.top = event.pageY + 'px';"
			"   window.layoutPosX = Math.floor((event.pageX - 254) / 35);"
			"   window.layoutPosY = Math.floor((event.pageY - 92) / 35);"
			"  }"
			"  return true;"
			" });"
			"});"
			;
		body.AddChildTag(HtmlTagJavascript(javascript.str()));

		connection->Send(HtmlFullResponse("Railcontrol", body));
	}

} ; // namespace webserver
