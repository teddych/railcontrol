#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>
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
#include "webserver/HtmlTagInputHidden.h"
#include "webserver/HtmlTagInputSliderLocoSpeed.h"
#include "webserver/HtmlTagInputTextWithLabel.h"
#include "webserver/HtmlTagSelect.h"

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
	WebClient::WebClient(const unsigned int id, Network::TcpConnection* connection, WebServer& webserver, Manager& m)
	:	id(id),
		connection(connection),
		run(false),
		server(webserver),
		clientThread(thread([this] {worker();})),
		manager(m),
		buttonID(0)
	{}

	WebClient::~WebClient()
	{
		run = false;
		clientThread.join();
		connection->Terminate();
	}

	// worker is the thread that handles client requests
	void WebClient::worker()
	{
		WorkerImpl();
		connection->Terminate();
	}

	void WebClient::WorkerImpl()
	{
		xlog("Executing webclient");
		run = true;

		char buffer_in[1024];
		memset(buffer_in, 0, sizeof(buffer_in));

		size_t pos = 0;
		string s;
		while(pos < sizeof(buffer_in) - 1 && s.find("\n\n") == string::npos && run)
		{
			pos += connection->Receive(buffer_in + pos, sizeof(buffer_in) - 1 - pos, 0);
			s = string(buffer_in);
			str_replace(s, string("\r\n"), string("\n"));
			str_replace(s, string("\r"), string("\n"));
		}

		vector<string> lines;
		str_split(s, string("\n"), lines);

		if (lines.size() <= 1)
		{
			xlog("Ignoring invalid request");
			return;
		}

		string method;
		string uri;
		string protocol;
		map<string, string> arguments;
		map<string, string> headers;
		interpretClientRequest(lines, method, uri, protocol, arguments, headers);
		xlog("%s %s", method.c_str(), uri.c_str());

		// if method is not implemented
		if ((method.compare("GET") != 0) && (method.compare("HEAD") != 0))
		{
			xlog("HTTP method %s not implemented", method.c_str());
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

		xlog("Terminating webclient");
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
		xlog(realFile);
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

	void WebClient::handleLocoSpeed(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		speed_t speed = GetIntegerMapEntry(arguments, "speed", MinSpeed);

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
		controlID_t controlID = ControlNone;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		string name("New Loco");
		if (locoID > LocoNone)
		{
			const datamodel::Loco* loco = manager.getLoco(locoID);
			controlID = loco->controlID;
			protocol = loco->protocol;
			address = loco->address;
			name = loco->name;
		}

		std::map<controlID_t,string> controls = manager.controlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlNone)
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

		content.AddContent(HtmlTag("h1").AddContent("Edit loco &quot;" + name + "&quot;"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "locosave"))
			.AddContent(HtmlTagInputHidden("loco", to_string(locoID)))
			.AddContent(HtmlTagInputTextWithLabel("name", "Loco Name:", name))
			.AddContent(HtmlTagLabel("Control:", "control"))
			.AddContent(HtmlTagSelect("control", controlOptions, to_string(controlID)))
			.AddContent(HtmlTagLabel("Protocol:", "protocol"))
			.AddContent(HtmlTagSelect("protocol", protocolOptions, to_string(protocol)))
			.AddContent(HtmlTagInputTextWithLabel("address", "Address:", to_string(address))));
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
		string result;

		if (!manager.locoSave(locoID, name, controlId, protocol, address, result))
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
			layoutPosition_t posX;
			layoutPosition_t posY;
			layoutPosition_t posZ;
			layoutItemSize_t w;
			layoutItemSize_t h;
			layoutRotation_t r;
			accessory.second->position(posX, posY, posZ, w, h, r);
			if (posZ != layer)
			{
				continue;
			}
			content.AddChildTag(HtmlTagAccessory(accessory.first, accessory.second->name, posX, posY, posZ, accessory.second->state, accessory.second->address));
		}
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleAccessoryEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		controlID_t controlID = ControlNone;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		string name("New Accessory");
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		accessoryTimeout_t timeout = 100;
		if (accessoryID > AccessoryNone)
		{
			const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
			controlID = accessory->controlID;
			protocol = accessory->protocol;
			address = accessory->address;
			name = accessory->name;
			posx = accessory->posX;
			posy = accessory->posY;
			posz = accessory->posZ;
		}

		std::map<controlID_t,string> controls = manager.controlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlNone)
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
		for(int i = 0; i < 20; ++i)
		{
			string is(to_string(i));
			positionOptions[is] = is;
		}

		std::map<string, string> timeoutOptions;
		timeoutOptions["0"] = "0";
		timeoutOptions["100"] = "100";
		timeoutOptions["250"] = "250";
		timeoutOptions["1000"] = "1000";

		content.AddContent(HtmlTag("h1").AddContent("Edit acessory &quot;" + name + "&quot;"));
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
			.AddContent(HtmlTagSelect("posx", positionOptions, to_string(posx)))
			.AddContent(HtmlTagLabel("Pos Y:", "posy"))
			.AddContent(HtmlTagSelect("posy", positionOptions, to_string(posy)))
			.AddContent(HtmlTagLabel("Pos Z:", "posz"))
			.AddContent(HtmlTagSelect("posz", positionOptions, to_string(posz)))
			.AddContent(HtmlTagLabel("Timeout:", "timeout"))
			.AddContent(HtmlTagSelect("timeout", timeoutOptions, to_string(timeout)))
		);
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
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
		string result;
		if (!manager.accessorySave(accessoryID, name, posX, posY, posZ, controlId, protocol, address, AccessoryTypeDefault, AccessoryStateOff, timeout, result))
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
				usleep(100000);
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

			id = "locofunction_" + to_string(locoID);
			buttonArguments["function"] = "0";
			ss << HtmlTagButtonCommandToggle("f0", id + "_0", loco->GetFunction(0), buttonArguments);
			buttonArguments["function"] = "1";
			ss << HtmlTagButtonCommandToggle("f1", id + "_1", loco->GetFunction(1), buttonArguments);
			buttonArguments["function"] = "2";
			ss << HtmlTagButtonCommandToggle("f2", id + "_2", loco->GetFunction(2), buttonArguments);
			buttonArguments.erase("function");

			id = "locodirection_" + to_string(locoID);
			ss << HtmlTagButtonCommandToggle("dir", id, true, buttonArguments);

			id = "locoedit_" + to_string(locoID);
			ss << HtmlTagButtonPopup("Edit", id, buttonArguments);
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
		body.AddAttribute("onload","loadDivFromForm('selectLoco_form', 'loco');loadDivFromForm('selectLayout_form', 'layout');");

		body.AddChildTag(HtmlTag("h1").AddContent("Railcontrol"));

		map<string,string> buttonArguments;

		body.AddChildTag(HtmlTag("div").AddAttribute("class", "menu")
			.AddContent(HtmlTagButtonCommand("&times;", "quit"))
			.AddContent(HtmlTagButtonCommandToggle(HtmlTag("span").AddAttribute("class", "symbola").AddContent("&#9211;"), "booster", false, buttonArguments))
			.AddContent(HtmlTagButtonPopup("NewLoco", "locoedit_0")));

		body.AddChildTag(HtmlTag("div").AddAttribute("class", "locolist").AddChildTag(selectLoco()));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "layoutlist").AddChildTag(selectLayout()));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "loco").AddAttribute("id", "loco"));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "layout").AddAttribute("id", "layout"));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "popup").AddAttribute("id", "popup"));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "status").AddAttribute("id", "status"));

		body.AddChildTag(HtmlTagJavascript(
			"var updater = new EventSource('/?cmd=updater');"
			"updater.onmessage = function(e) {"
			" dataUpdate(e);"
			"};"));

		body.AddChildTag(HtmlTag("div").AddAttribute("class", "contextmenu").AddAttribute("id", "layout_context")
			.AddChildTag(HtmlTag("ul").AddAttribute("class", "contextentries")
			.AddChildTag(HtmlTag("li").AddAttribute("class", "contextentry").AddContent("New accessory").AddAttribute("onClick", "loadPopup('/?cmd=accessoryedit&accessory=0');"))
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
			"   window.layoutPosY = Math.floor((event.pageY - 189) / 35);"
			"  }"
			"  return true;"
			" });"
			"});"
			;
		body.AddChildTag(HtmlTagJavascript(javascript.str()));

		connection->Send(HtmlFullResponse("Railcontrol", body));
	}

} ; // namespace webserver
