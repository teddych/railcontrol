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
#include "webserver/HtmlTagButtonCancel.h"
#include "webserver/HtmlTagButtonCommand.h"
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
		else if (arguments["cmd"].compare("on") == 0)
		{
			HtmlReplyWithHeader(string("Turning booster on"));
			manager.booster(ControlTypeWebserver, BoosterGo);
		}
		else if (arguments["cmd"].compare("off") == 0)
		{
			HtmlReplyWithHeader(string("Turning booster off"));
			manager.booster(ControlTypeWebserver, BoosterStop);
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
		const char* contentType = NULL;
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
		}
		else if (length > 3 && virtualFile[length - 3] == '.' && virtualFile[length - 2] == 'j' && virtualFile[length - 1] == 's')
		{
			contentType = "application/javascript";
		}

		Response response(Response::OK, HtmlTag());
		response.AddHeader("Cache-Control", "max-age=3600");
		response.AddHeader("Content-Length", to_string(s.st_size));
		response.AddHeader("Content-Type", contentType);
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
		string directionText = GetStringMapEntry(arguments, "direction", "forward");
		direction_t direction = (directionText.compare("forward") == 0 ? DirectionRight : DirectionLeft);

		manager.locoDirection(ControlTypeWebserver, locoID, direction);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; is now set to " << direction;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoFunction(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		function_t function = GetIntegerMapEntry(arguments, "function", 0);
		bool on = GetIntegerMapEntry(arguments, "on", false);

		manager.locoFunction(ControlTypeWebserver, locoID, function, on);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; has now set f";
		ss << function << " to " << (on ? "on" : "off");
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
		if (locoID > LocoNone)
		{
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
				ss << "Loco &quot;" << locoID << "&quot; saved.";
			}
		}
		else
		{
			ss << "Unable to save loco.";
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
			HtmlTag div("div");
			string id("a_" + to_string(accessory.first));
			div.AddAttribute("id", id);
			string classes("layout_item accessory_item");
			if (accessory.second->state == AccessoryStateOn)
			{
				classes += " accessory_on";
			}
			div.AddAttribute("class", classes);
			div.AddAttribute("style", "left:" + to_string(posX * 35) + "px;top:" + to_string(posY * 35) + "px;");
			div.AddContent("A");
			div.AddChildTag(HtmlTag("span").AddAttribute("class", "tooltip").AddContent(accessory.second->name));
			stringstream javascript;
			javascript << "$(function() {"
				" $('#" << id << "').on('click', function() {"
				"  var element = document.getElementById('" << id << "');"
				"  var url = '/?cmd=accessorystate';"
				"  url += '&state=' + (element.classList.contains('accessory_on') ? 'off' : 'on');"
				"  url += '&accessory=" << accessory.first << "';"
				"  var xmlHttp = new XMLHttpRequest();"
				"  xmlHttp.open('GET', url, true);"
				"  xmlHttp.send(null);"
				"  return false;"
				" });"
				"});";
			div.AddChildTag(HtmlTagJavascript(javascript.str()));
			content.AddContent(div);
		}
		HtmlReplyWithHeader(content);
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
		connection->Send(HtmlResponse("Railcontrol", tag));
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
			ss << HtmlTagInputSliderLocoSpeed("speed", "locospeed", MinSpeed, MaxSpeed, speed, locoID);
			buttonArguments["speed"] = "0";
			ss << HtmlTagButtonCommand("0%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "255";
			ss << HtmlTagButtonCommand("25%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "511";
			ss << HtmlTagButtonCommand("50%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "767";
			ss << HtmlTagButtonCommand("75%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "1023";
			ss << HtmlTagButtonCommand("100%", "locospeed", buttonArguments);
			buttonArguments.erase("speed");

			buttonArguments["function"] = "0";
			buttonArguments["on"] = "1";
			ss << HtmlTagButtonCommand("f0 on", "locofunction", buttonArguments);
			buttonArguments["on"] = "0";
			ss << HtmlTagButtonCommand("f0 off", "locofunction", buttonArguments);
			buttonArguments.erase("function");
			buttonArguments.erase("on");

			buttonArguments["direction"] = "forward";
			ss << HtmlTagButtonCommand("forward", "locodirection", buttonArguments);
			buttonArguments["direction"] = "reverse";
			ss << HtmlTagButtonCommand("reverse", "locodirection", buttonArguments);
			buttonArguments.erase("direction");

			ss << HtmlTagButtonPopup("Edit", "locoedit", buttonArguments);
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
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "menu")
			.AddContent(HtmlTagButtonCommand("X", "quit"))
			.AddContent(HtmlTagButtonCommand("On", "on"))
			.AddContent(HtmlTagButtonCommand("Off", "off"))
			.AddContent(HtmlTagButtonPopup("NewLoco", "locoedit")));

		body.AddChildTag(HtmlTag("div").AddAttribute("class", "locolist").AddChildTag(selectLoco()));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "layoutlist").AddChildTag(selectLayout()));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "loco").AddAttribute("id", "loco"));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "layout").AddAttribute("id", "layout"));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "popup").AddAttribute("id", "popup"));
		body.AddChildTag(HtmlTag("div").AddAttribute("class", "status").AddAttribute("id", "status"));
		body.AddChildTag(HtmlTagJavascript(
			"var updater = new EventSource('/?cmd=updater');"
			"updater.onmessage = function(e) {"
			" var status = document.getElementById('status');"
			" var arguments = e.data.split(';');"
			" var argumentMap = new Map();"
			" arguments.forEach(function(argument) {"
			"  var parts = argument.split('=');"
			"  if (parts[0] == 'status') {"
			"   status.innerHTML += parts[1] + '<br>';"
			"   status.scrollTop = status.scrollHeight - status.clientHeight;"
			"  }"
			"  else {"
			"   argumentMap.set(parts[0], parts[1]);"
			"  }"
			" });"
			" if (argumentMap.get('command') == 'locospeed') {"
			"  var elementName = 'locospeed_' + argumentMap.get('loco');"
			"  var element = document.getElementById(elementName);"
			"  if (element) element.value = argumentMap.get('speed');"
			" }"
			" else if (argumentMap.get('command') == 'accessory') {"
			"  var elementName = 'a_' + argumentMap.get('accessory');"
			"  var element = document.getElementById(elementName);"
			"  if (element) {"
			"   var state = argumentMap.get('state');"
			"   if (state == 'green') {"
			"    element.classList.add('accessory_on');"
			"   } else {"
			"    element.classList.remove('accessory_on');"
			"   }"
			"  }"
			" }"
			"};"));

		connection->Send(HtmlFullResponse("Railcontrol", body));
	}

} ; // namespace webserver
