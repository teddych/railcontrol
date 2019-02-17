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
#include "webserver/HtmlTagInputCheckboxWithLabel.h"
#include "webserver/HtmlTagInputHidden.h"
#include "webserver/HtmlTagInputIntegerWithLabel.h"
#include "webserver/HtmlTagInputSliderLocoSpeed.h"
#include "webserver/HtmlTagInputTextWithLabel.h"
#include "webserver/HtmlTagSelectWithLabel.h"
#include "webserver/HtmlTagStreet.h"
#include "webserver/HtmlTagSwitch.h"
#include "webserver/HtmlTagTrack.h"

using datamodel::Accessory;
using datamodel::Layer;
using datamodel::Loco;
using datamodel::Relation;
using datamodel::Street;
using datamodel::Switch;
using datamodel::Track;
using std::map;
using std::string;
using std::stringstream;
using std::thread;
using std::to_string;
using std::vector;

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
			else if (arguments["cmd"].compare("layeredit") == 0)
			{
				handleLayerEdit(arguments);
			}
			else if (arguments["cmd"].compare("layersave") == 0)
			{
				handleLayerSave(arguments);
			}
			else if (arguments["cmd"].compare("layerlist") == 0)
			{
				handleLayerList(arguments);
			}
			else if (arguments["cmd"].compare("layeraskdelete") == 0)
			{
				handleLayerAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("layerdelete") == 0)
			{
				handleLayerDelete(arguments);
			}
			else if (arguments["cmd"].compare("controledit") == 0)
			{
				handleControlEdit(arguments);
			}
			else if (arguments["cmd"].compare("controlsave") == 0)
			{
				handleControlSave(arguments);
			}
			else if (arguments["cmd"].compare("controllist") == 0)
			{
				handleControlList(arguments);
			}
			else if (arguments["cmd"].compare("controlaskdelete") == 0)
			{
				handleControlAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("controldelete") == 0)
			{
				handleControlDelete(arguments);
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
			else if (arguments["cmd"].compare("locolist") == 0)
			{
				handleLocoList(arguments);
			}
			else if (arguments["cmd"].compare("locoaskdelete") == 0)
			{
				handleLocoAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("locodelete") == 0)
			{
				handleLocoDelete(arguments);
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
			else if (arguments["cmd"].compare("accessorylist") == 0)
			{
				handleAccessoryList(arguments);
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
			else if (arguments["cmd"].compare("switchlist") == 0)
			{
				handleSwitchList(arguments);
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
			else if (arguments["cmd"].compare("streetedit") == 0)
			{
				handleStreetEdit(arguments);
			}
			else if (arguments["cmd"].compare("streetsave") == 0)
			{
				handleStreetSave(arguments);
			}
			else if (arguments["cmd"].compare("streetlist") == 0)
			{
				handleStreetList(arguments);
			}
			else if (arguments["cmd"].compare("streetaskdelete") == 0)
			{
				handleStreetAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("streetdelete") == 0)
			{
				handleStreetDelete(arguments);
			}
			else if (arguments["cmd"].compare("streetget") == 0)
			{
				handleStreetGet(arguments);
			}
			else if (arguments["cmd"].compare("streetexecute") == 0)
			{
				handleStreetExecute(arguments);
			}
			else if (arguments["cmd"].compare("trackedit") == 0)
			{
				handleTrackEdit(arguments);
			}
			else if (arguments["cmd"].compare("tracksave") == 0)
			{
				handleTrackSave(arguments);
			}
			else if (arguments["cmd"].compare("tracklist") == 0)
			{
				handleTrackList(arguments);
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
			else if (arguments["cmd"].compare("protocolloco") == 0)
			{
				handleProtocolLoco(arguments);
			}
			else if (arguments["cmd"].compare("protocolaccessory") == 0)
			{
				handleProtocolAccessory(arguments);
			}
			else if (arguments["cmd"].compare("protocolswitch") == 0)
			{
				handleProtocolSwitch(arguments);
			}
			else if (arguments["cmd"].compare("relationadd") == 0)
			{
				handleRelationAdd(arguments);
			}
			else if (arguments["cmd"].compare("layout") == 0)
			{
				handleLayout(arguments);
			}
			else if (arguments["cmd"].compare("locoselector") == 0)
			{
				handleLocoSelector(arguments);
			}
			else if (arguments["cmd"].compare("stopall") == 0)
			{
				manager.StopAllLocosImmediately(ControlTypeWebserver);
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
			logger->Info("HTTP connection {0}: 404 Not found: {1}", id, virtualFile);
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

	HtmlTag WebClient::HtmlTagControlArgument(const unsigned char argNr, const argumentType_t type, const string& value)
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
				const int valueInteger = Util::StringToInteger(value, 0, 62);
				return HtmlTagInputIntegerWithLabel("arg" + to_string(argNr), argumentName, valueInteger, 0, 62);
		}
		return HtmlTagInputTextWithLabel("arg" + to_string(argNr), argumentName, value);
	}

	void WebClient::handleLayerEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		layerID_t layerID = GetIntegerMapEntry(arguments, "layer", LayerNone);
		string name("New Layer");

		if (layerID != LayerNone)
		{
			Layer* layer = manager.GetLayer(layerID);
			if (layer != nullptr)
			{
				name = layer->name;
			}
		}

		content.AddChildTag(HtmlTag("h1").AddContent("Edit layer &quot;" + name + "&quot;"));
		HtmlTag form("form");
		form.AddAttribute("id", "editform");
		form.AddChildTag(HtmlTagInputHidden("cmd", "layersave"));
		form.AddChildTag(HtmlTagInputHidden("layer", to_string(layerID)));
		form.AddChildTag(HtmlTagInputTextWithLabel("name", "Layer Name:", name));
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(form));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLayerSave(const map<string, string>& arguments)
	{
		layerID_t layerID = GetIntegerMapEntry(arguments, "layer", LayerNone);
		string name = GetStringMapEntry(arguments, "name");
		string result;

		if (!manager.LayerSave(layerID, name, result))
		{
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Layer &quot;" + name + "&quot; saved.");
	}

	void WebClient::handleLayerAskDelete(const map<string, string>& arguments)
	{
		layerID_t layerID = GetIntegerMapEntry(arguments, "layer", LayerNone);

		if (layerID == ControlNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		const Layer* layer = manager.GetLayer(layerID);
		if (layer == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete layer &quot;" + layer->Name() + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the layer &quot;" + layer->Name() + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "layerdelete"))
			.AddContent(HtmlTagInputHidden("layer", to_string(layerID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLayerDelete(const map<string, string>& arguments)
	{
		layerID_t layerID = GetIntegerMapEntry(arguments, "layer", LayerNone);
		const Layer* layer = manager.GetLayer(layerID);
		if (layer == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		if (!manager.LayerDelete(layerID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete layer");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Layer &quot;" + layer->Name() + "&quot; deleted.");
	}

	void WebClient::handleLayerList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Layers"));
		HtmlTag table("table");
		const map<string,layerID_t> layerList = manager.LayerListByName();
		map<string,string> layerArgument;
		for (auto layer : layerList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(layer.first));
			string layerIdString = to_string(layer.second);
			layerArgument["layer"] = layerIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "layeredit_list_" + layerIdString, layerArgument)));
			if (layer.second != 1)
			{
				row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "layeraskdelete_" + layerIdString, layerArgument)));
			}
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "layeredit_0"));
		HtmlReplyWithHeader(content);
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

		content.AddChildTag(HtmlTag("h1").AddContent("Edit control &quot;" + name + "&quot;"));
		HtmlTag form("form");
		form.AddAttribute("id", "editform");
		form.AddChildTag(HtmlTagInputHidden("cmd", "controlsave"));
		form.AddChildTag(HtmlTagInputHidden("control", to_string(controlID)));
		form.AddChildTag(HtmlTagInputTextWithLabel("name", "Control Name:", name));
		form.AddChildTag(HtmlTagSelectWithLabel("hardwaretype", "Hardware type:", hardwareOptions, to_string(hardwareType)));
		std::map<unsigned char,argumentType_t> argumentTypes = manager.ArgumentTypesOfControl(controlID);
		if (argumentTypes.count(1) == 1)
		{
			form.AddChildTag(HtmlTagControlArgument(1, argumentTypes.at(1), arg1));
		}
		if (argumentTypes.count(2) == 1)
		{
			form.AddChildTag(HtmlTagControlArgument(2, argumentTypes.at(2), arg2));
		}
		if (argumentTypes.count(3) == 1)
		{
			form.AddChildTag(HtmlTagControlArgument(3, argumentTypes.at(3), arg3));
		}
		if (argumentTypes.count(4) == 1)
		{
			form.AddChildTag(HtmlTagControlArgument(4, argumentTypes.at(4), arg4));
		}
		if (argumentTypes.count(5) == 1)
		{
			form.AddChildTag(HtmlTagControlArgument(5, argumentTypes.at(5), arg5));
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(form));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleControlSave(const map<string, string>& arguments)
	{
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
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Control &quot;" + name + "&quot; saved.");
	}

	void WebClient::handleControlAskDelete(const map<string, string>& arguments)
	{
		controlID_t controlID = GetIntegerMapEntry(arguments, "control", ControlNone);

		if (controlID == ControlNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown control");
			return;
		}

		const hardware::HardwareParams* control = manager.getHardware(controlID);
		if (control == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown control");
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete control &quot;" + control->name + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the control &quot;" + control->name + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "controldelete"))
			.AddContent(HtmlTagInputHidden("control", to_string(controlID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleControlDelete(const map<string, string>& arguments)
	{
		controlID_t controlID = GetIntegerMapEntry(arguments, "control", ControlNone);
		const hardware::HardwareParams* control = manager.getHardware(controlID);
		if (control == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete control");
			return;
		}

		if (!manager.controlDelete(controlID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete control");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Control &quot;" + control->name + "&quot; deleted.");
	}

	void WebClient::handleControlList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Controls"));
		HtmlTag table("table");
		const map<string,hardware::HardwareParams*> hardwareList = manager.controlListByName();
		map<string,string> hardwareArgument;
		for (auto hardware : hardwareList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(hardware.first));
			string controlIdString = to_string(hardware.second->controlID);
			hardwareArgument["control"] = controlIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "controledit_list_" + controlIdString, hardwareArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "controlaskdelete_" + controlIdString, hardwareArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "controledit_0"));
		HtmlReplyWithHeader(content);
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

	HtmlTag WebClient::HtmlTagProtocolLoco(const controlID_t controlID, const protocol_t selectedProtocol)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTagLabel("Protocol:", "protocol"));
		map<string,protocol_t> protocolMap = manager.LocoProtocolsOfControl(controlID);
		content.AddChildTag(HtmlTagSelect("protocol", protocolMap, selectedProtocol));
		return content;
	}

	void WebClient::handleProtocolLoco(const map<string, string>& arguments)
	{
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		if (controlId == ControlIdNone)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown control"));
			return;
		}
		locoID_t locoId = GetIntegerMapEntry(arguments, "loco", LocoNone);
		Loco* loco = manager.getLoco(locoId);
		if (loco == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown loco"));
			return;
		}
		HtmlReplyWithHeader(HtmlTagProtocolLoco(controlId, loco->protocol));
	}

	HtmlTag WebClient::HtmlTagProtocolAccessory(const controlID_t controlID, const protocol_t selectedProtocol)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTagLabel("Protocol:", "protocol"));
		map<string,protocol_t> protocolMap = manager.AccessoryProtocolsOfControl(controlID);
		content.AddChildTag(HtmlTagSelect("protocol", protocolMap, selectedProtocol));
		return content;
	}

	HtmlTag WebClient::HtmlTagTimeout(const accessoryTimeout_t timeout) const
	{
		std::map<string,string> timeoutOptions;
		timeoutOptions["0000"] = "0";
		timeoutOptions["0100"] = "100";
		timeoutOptions["0250"] = "250";
		timeoutOptions["1000"] = "1000";
		return HtmlTagSelectWithLabel("timeout", "Timeout:", timeoutOptions, toStringWithLeadingZeros(timeout, 4));
	}

	HtmlTag WebClient::HtmlTagRelation(const string& priority, const switchID_t switchId, const switchState_t state)
	{
		HtmlTag content("div");
		content.AddAttribute("id", "priority_" + priority);
		HtmlTagButton deleteButton("Del", "delete_relation_" + priority);
		deleteButton.AddAttribute("onclick", "deleteElement('priority_" + priority + "');return false;");
		content.AddChildTag(deleteButton);

		content.AddChildTag(HtmlTagInputHidden("relation_type_" + priority, to_string(ObjectTypeSwitch)));
		std::map<string,Switch*> switches = manager.switchListByName();
		map<string,switchID_t> switchOptions;
		for (auto mySwitch : switches)
		{
			switchOptions[mySwitch.first] = mySwitch.second->objectID;
		}
		content.AddChildTag(HtmlTagSelect("relation_id_" + priority, switchOptions, switchId).AddClass("select_relation_id"));

		map<string,switchState_t> stateOptions;
		stateOptions["Straight"] = SwitchStateStraight;
		stateOptions["Turnout"] = SwitchStateTurnout;
		content.AddChildTag(HtmlTagSelect("relation_state_" + priority, stateOptions, state).AddClass("select_relation_state"));
		return content;
	}

	HtmlTag WebClient::HtmlTagRotation(const layoutRotation_t rotation) const
	{
		std::map<string, string> rotationOptions;
		rotationOptions[to_string(Rotation0)] = "none";
		rotationOptions[to_string(Rotation90)] = "90 deg clockwise";
		rotationOptions[to_string(Rotation180)] = "180 deg";
		rotationOptions[to_string(Rotation270)] = "90 deg anti-clockwise";
		return HtmlTagSelectWithLabel("rotation", "Rotation:", rotationOptions, to_string(rotation));
	}

	HtmlTag WebClient::HtmlTagSelectTrack(const std::string& name, const std::string& label, const trackID_t trackId, const direction_t direction) const
	{
		HtmlTag tag;
		map<string,trackID_t> tracks = manager.trackListIdByName();
		tag.AddChildTag(HtmlTagSelectWithLabel(name + "track", label, tracks, trackId).AddClass("select_track"));
		map<string,direction_t> directions;
		directions["Left"] = DirectionLeft;
		directions["Right"] = DirectionRight;
		tag.AddChildTag(HtmlTagSelect(name + "direction", directions, direction).AddClass("select_direction"));
		return tag;
	}

	void WebClient::handleProtocolAccessory(const map<string, string>& arguments)
	{
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		if (controlId == ControlIdNone)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown control"));
			return;
		}
		accessoryID_t accessoryId = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		Accessory* accessory = manager.getAccessory(accessoryId);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown accessory"));
			return;
		}
		HtmlReplyWithHeader(HtmlTagProtocolAccessory(controlId, accessory->protocol));
	}

	void WebClient::handleRelationAdd(const map<string, string>& arguments)
	{
		string priorityString = GetStringMapEntry(arguments, "priority", "1");
		priority_t priority = Util::StringToInteger(priorityString, 1);
		HtmlTag container;
		container.AddChildTag(HtmlTagRelation(priorityString));
		container.AddChildTag(HtmlTag("div").AddAttribute("id", "new_priority_" + to_string(priority + 1)));
		HtmlReplyWithHeader(container);
	}

	void WebClient::handleProtocolSwitch(const map<string, string>& arguments)
	{
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		if (controlId == ControlIdNone)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown control"));
			return;
		}
		switchID_t switchId = GetIntegerMapEntry(arguments, "switch", SwitchNone);
		Switch* mySwitch = manager.getSwitch(switchId);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown switch"));
			return;
		}
		HtmlReplyWithHeader(HtmlTagProtocolAccessory(controlId, mySwitch->protocol));
	}

	void WebClient::handleLocoEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		controlID_t controlID = ControlIdNone;
		protocol_t protocol = ProtocolNone;
		address_t address = 1;
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

		std::map<controlID_t,string> controls = manager.LocoControlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlIdNone)
			{
				controlID = control.first;
			}
		}

		content.AddChildTag(HtmlTag("h1").AddContent("Edit loco &quot;" + name + "&quot;"));
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(HtmlTag("form").AddAttribute("id", "editform")
			.AddChildTag(HtmlTagInputHidden("cmd", "locosave"))
			.AddChildTag(HtmlTagInputHidden("loco", to_string(locoID)))
			.AddChildTag(HtmlTagInputTextWithLabel("name", "Loco Name:", name))
			.AddChildTag(HtmlTagSelectWithLabel("control", "Control:", controlOptions, to_string(controlID))
				.AddAttribute("onchange", "loadProtocol('loco', " + to_string(locoID) + ")")
				)
			.AddChildTag(HtmlTag("div").AddAttribute("id", "select_protocol").AddChildTag(HtmlTagProtocolLoco(controlID, protocol)))
			.AddChildTag(HtmlTagInputIntegerWithLabel("address", "Address:", address, 1, 9999))
			.AddChildTag(HtmlTagInputIntegerWithLabel("function", "# of functions:", nrOfFunctions, 0, datamodel::LocoFunctions::maxFunctions))
			));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLocoSave(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		string name = GetStringMapEntry(arguments, "name");
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		protocol_t protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		address_t address = GetIntegerMapEntry(arguments, "address", AddressNone);
		function_t nrOfFunctions = GetIntegerMapEntry(arguments, "function", 0);
		string result;

		if (!manager.locoSave(locoID, name, controlId, protocol, address, nrOfFunctions, result))
		{
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}

		HtmlTag p("p");
		p.AddContent("Loco &quot;" + name + "&quot; saved.");
		HtmlTagJavascript script("loadLocoSelector();");
		HtmlReplyWithHeader(HtmlTag().AddChildTag(p).AddChildTag(script));
	}

	void WebClient::handleLocoList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Locos"));
		HtmlTag table("table");
		const map<string,datamodel::Loco*> locoList = manager.locoListByName();
		map<string,string> locoArgument;
		for (auto loco : locoList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(loco.first));
			row.AddChildTag(HtmlTag("td").AddContent(to_string(loco.second->address)));
			string locoIdString = to_string(loco.second->objectID);
			locoArgument["loco"] = locoIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "locoedit_list_" + locoIdString, locoArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "locoaskdelete_" + locoIdString, locoArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "locoedit_0"));
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLocoAskDelete(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);

		if (locoID == LocoNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown loco");
			return;
		}

		const datamodel::Loco* loco = manager.getLoco(locoID);
		if (loco == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown loco");
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete loco &quot;" + loco->name + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the loco &quot;" + loco->name + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "locodelete"))
			.AddContent(HtmlTagInputHidden("loco", to_string(locoID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLocoDelete(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		const datamodel::Loco* loco = manager.getLoco(locoID);
		if (loco == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete loco");
			return;
		}

		if (!manager.locoDelete(locoID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete loco");
			return;
		}

		HtmlTag p("p");
		p.AddContent("Loco &quot;" + loco->name + "&quot; deleted.");
		HtmlTagJavascript script("loadLocoSelector();");
		HtmlReplyWithHeader(HtmlTag().AddChildTag(p).AddChildTag(script));
	}

	HtmlTag WebClient::HtmlTagSelectLayout() const
	{
		map<string,layerID_t> options = manager.LayerListByNameWithFeedback();
		// FIXME: select layers with content
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

		const map<streetID_t,datamodel::Street*>& streets = manager.streetList();
		for (auto street : streets)
		{
			if (street.second->posZ != layer || street.second->visible == VisibleNo)
			{
				continue;
			}
			content.AddChildTag(HtmlTagStreet(street.second));
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

		std::map<controlID_t,string> controls = manager.AccessoryControlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlIdNone)
			{
				controlID = control.first;
			}
		}

		content.AddChildTag(HtmlTag("h1").AddContent("Edit accessory &quot;" + name + "&quot;"));
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(HtmlTag("form").AddAttribute("id", "editform")
			.AddChildTag(HtmlTagInputHidden("cmd", "accessorysave"))
			.AddChildTag(HtmlTagInputHidden("accessory", to_string(accessoryID)))
			.AddChildTag(HtmlTagInputTextWithLabel("name", "Accessory Name:", name))
			.AddChildTag(HtmlTagSelectWithLabel("control", "Control:", controlOptions, to_string(controlID))
				.AddAttribute("onchange", "loadProtocol('accessory', " + to_string(accessoryID) + ")")
				)
			.AddChildTag(HtmlTag("div").AddAttribute("id", "select_protocol").AddChildTag(HtmlTagProtocolAccessory(controlID, protocol)))
			.AddChildTag(HtmlTagInputIntegerWithLabel("address", "Address:", address, 1, 2044))
			.AddChildTag(HtmlTagInputIntegerWithLabel("posx", "Pos X:", posx, 0, 255))
			.AddChildTag(HtmlTagInputIntegerWithLabel("posy", "Pos Y:", posy, 0, 255))
			/* FIXME: layers not supported
			.AddChildTag(HtmlTagInputIntegerWithLabel("posz", "Pos Z:", posz, 0, 20))
			*/
			.AddChildTag(HtmlTagTimeout(timeout))
			.AddChildTag(HtmlTagInputCheckboxWithLabel("inverted", "Inverted:", "true", inverted))
		));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
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
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Accessory &quot;" + name + "&quot; saved.");
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

	void WebClient::handleAccessoryList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Accessories"));
		HtmlTag table("table");
		const map<string,datamodel::Accessory*> accessoryList = manager.accessoryListByName();
		map<string,string> locoArgument;
		for (auto accessory : accessoryList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(accessory.first));
			string accessoryIdString = to_string(accessory.second->objectID);
			locoArgument["accessory"] = accessoryIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "accessoryedit_list_" + accessoryIdString, locoArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "accessoryaskdelete_" + accessoryIdString, locoArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "accessoryedit_0"));
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleAccessoryAskDelete(const map<string, string>& arguments)
	{
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);

		if (accessoryID == AccessoryNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown accessory");
			return;
		}

		const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown accessory");
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
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory", AccessoryNone);
		const datamodel::Accessory* accessory = manager.getAccessory(accessoryID);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete accessory");
			return;
		}

		if (!manager.accessoryDelete(accessoryID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete accessory");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Accessory &quot;" + accessory->name + "&quot; deleted.");
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

		std::map<controlID_t,string> controls = manager.AccessoryControlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlID == ControlIdNone)
			{
				controlID = control.first;
			}
		}

		std::map<string, string> typeOptions;
		typeOptions[to_string(SwitchTypeLeft)] = "Left";
		typeOptions[to_string(SwitchTypeRight)] = "Right";

		content.AddChildTag(HtmlTag("h1").AddContent("Edit switch &quot;" + name + "&quot;"));
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(HtmlTag("form").AddAttribute("id", "editform")
			.AddChildTag(HtmlTagInputHidden("cmd", "switchsave"))
			.AddChildTag(HtmlTagInputHidden("switch", to_string(switchID)))
			.AddChildTag(HtmlTagInputTextWithLabel("name", "Switch Name:", name))
			.AddChildTag(HtmlTagSelectWithLabel("control", "Control:", controlOptions, to_string(controlID))
				.AddAttribute("onchange", "loadProtocol('switch', " + to_string(switchID) + ")")
				)
			.AddChildTag(HtmlTag("div").AddAttribute("id", "select_protocol").AddChildTag(HtmlTagProtocolAccessory(controlID, protocol)))
			.AddChildTag(HtmlTagInputIntegerWithLabel("address", "Address:", address, 1, 2044))
			.AddChildTag(HtmlTagInputIntegerWithLabel("posx", "Pos X:", posx, 0, 255))
			.AddChildTag(HtmlTagInputIntegerWithLabel("posy", "Pos Y:", posy, 0, 255))
			/* FIXME: layers not supported
			.AddChildTag(HtmlTagInputIntegerWithLabel("posz", "Pos Z:", posz, 0, 20))
			*/
			.AddChildTag(HtmlTagRotation(rotation))
			.AddChildTag(HtmlTagSelectWithLabel("type", "Type:", typeOptions, to_string(type)))
			.AddChildTag(HtmlTagTimeout(timeout))
			.AddChildTag(HtmlTagInputCheckboxWithLabel("inverted", "Inverted:", "true", inverted))
		));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleSwitchSave(const map<string, string>& arguments)
	{
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
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}
		HtmlReplyWithHeaderAndParagraph("Switch &quot;" + name + "&quot; saved.");
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

	void WebClient::handleSwitchList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Switches"));
		HtmlTag table("table");
		const map<string,datamodel::Switch*> switchList = manager.switchListByName();
		map<string,string> locoArgument;
		for (auto mySwitch : switchList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(mySwitch.first));
			string switchIdString = to_string(mySwitch.second->objectID);
			locoArgument["switch"] = switchIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "switchedit_list_" + switchIdString, locoArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "switchaskdelete_" + switchIdString, locoArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "switchedit_0"));
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleSwitchAskDelete(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);

		if (switchID == SwitchNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown switch");
			return;
		}

		const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown switch");
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
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch", SwitchNone);
		const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete switch");
			return;
		}

		if (!manager.switchDelete(switchID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete switch");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Switch &quot;" + mySwitch->name + "&quot; deleted.");
	}

	void WebClient::handleSwitchGet(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch");
		const datamodel::Switch* mySwitch = manager.getSwitch(switchID);
		HtmlReplyWithHeader(HtmlTagSwitch(mySwitch));
	}

	void WebClient::handleStreetGet(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street");
		const datamodel::Street* street = manager.getStreet(streetID);
		HtmlReplyWithHeader(HtmlTagStreet(street));
	}

	void WebClient::handleStreetEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);
		string name("New Street");
		vector<Relation*> relations;
		visible_t visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible", VisibleYes));
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		// FIXME: layers not supported yet: layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		automode_t automode = static_cast<automode_t>(GetBoolMapEntry(arguments, "automode", AutomodeNo));
		trackID_t fromTrack = GetIntegerMapEntry(arguments, "fromtrack", TrackNone);
		direction_t fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromdirection", DirectionRight));
		trackID_t toTrack = GetIntegerMapEntry(arguments, "totrack", TrackNone);
		direction_t toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "todirection", DirectionLeft));
		if (streetID > StreetNone)
		{
			const datamodel::Street* street = manager.getStreet(streetID);
			name = street->name;
			relations = street->GetRelations();
			visible = street->visible;
			posx = street->posX;
			posy = street->posY;
			// FIXME: layers not supported yet: posz = mySwitch->posZ;
			automode = street->automode;
			fromTrack = street->fromTrack;
			fromDirection = street->fromDirection;
			fromTrack = street->fromTrack;
			fromDirection = street->fromDirection;
		}

		content.AddChildTag(HtmlTag("h1").AddContent("Edit street &quot;" + name + "&quot;"));
		HtmlTag form("form");
		form.AddAttribute("id", "editform");
		form.AddChildTag(HtmlTagInputHidden("cmd", "streetsave"));
		form.AddChildTag(HtmlTagInputHidden("street", to_string(streetID)));
		form.AddChildTag(HtmlTagInputTextWithLabel("name", "Street Name:", name));

		HtmlTag relationDiv("div");
		relationDiv.AddChildTag(HtmlTagInputHidden("relationcounter", to_string(relations.size())));
		relationDiv.AddAttribute("id", "relation");
		priority_t priority = 1;
		for (auto relation : relations)
		{
			relationDiv.AddChildTag(HtmlTagRelation(to_string(relation->Priority()), relation->ObjectID2(), relation->AccessoryState()));
			priority = relation->Priority() + 1;
		}
		relationDiv.AddChildTag(HtmlTag("div").AddAttribute("id", "new_priority_" + to_string(priority)));
		HtmlTag relationDivOuter("div");
		relationDivOuter.AddChildTag(relationDiv);
		HtmlTagButton newButton("New", "newrelation");
		newButton.AddAttribute("onclick", "addRelation();return false;");
		relationDivOuter.AddChildTag(newButton);
		relationDivOuter.AddChildTag(HtmlTag("br"));
		form.AddChildTag(relationDivOuter);

		HtmlTagInputCheckboxWithLabel checkboxVisible("visible", "Visible:", "visible", static_cast<bool>(visible));
		checkboxVisible.AddAttribute("id", "visible");
		checkboxVisible.AddAttribute("onchange", "onChangeCheckboxShowHide('visible', 'position');");
		form.AddChildTag(checkboxVisible);

		HtmlTag posDiv("div");
		posDiv.AddAttribute("id", "position");
		if (visible == VisibleNo)
		{
			posDiv.AddAttribute("hidden");
		}
		posDiv.AddChildTag(HtmlTagInputIntegerWithLabel("posx", "Pos X:", posx, 0, 255));
		posDiv.AddChildTag(HtmlTagInputIntegerWithLabel("posy", "Pos Y:", posy, 0, 255));
		/* FIXME: layers not supported
		posDiv.AddChildTag(HtmlTagInputIntegerWithLabel("posz", "Pos Z:", posz, 0, 20)):
		*/
		form.AddChildTag(posDiv);

		HtmlTagInputCheckboxWithLabel checkboxAutomode("automode", "Auto-mode:", "automode", static_cast<bool>(automode));
		checkboxAutomode.AddAttribute("id", "automode");
		checkboxAutomode.AddAttribute("onchange", "onChangeCheckboxShowHide('automode', 'tracks');");
		form.AddChildTag(checkboxAutomode);

		HtmlTag tracksDiv("div");
		tracksDiv.AddAttribute("id", "tracks");
		if (automode == AutomodeNo)
		{
			tracksDiv.AddAttribute("hidden");
		}
		tracksDiv.AddChildTag(HtmlTagSelectTrack("from", "From track:", fromTrack, fromDirection));
		tracksDiv.AddChildTag(HtmlTagSelectTrack("to", "To track:", toTrack, toDirection));
		form.AddChildTag(tracksDiv);

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(form));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleStreetSave(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);
		string name = GetStringMapEntry(arguments, "name");
		visible_t visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible"));
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		automode_t automode = static_cast<automode_t>(GetBoolMapEntry(arguments, "automode"));
		trackID_t fromTrack = GetIntegerMapEntry(arguments, "fromtrack", TrackNone);
		direction_t fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromdirection", DirectionRight));
		trackID_t toTrack = GetIntegerMapEntry(arguments, "totrack", TrackNone);
		direction_t toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "todirection", DirectionLeft));

		vector<Relation*> relations;
		priority_t relationCount = GetIntegerMapEntry(arguments, "relationcounter", 0);
		priority_t priority = 1;
		for (priority_t relationId = 1; relationId <= relationCount; ++relationId)
		{
			string priorityString = to_string(relationId);
			objectType_t objectType = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "relation_type_" + priorityString));
			switchID_t switchId = GetIntegerMapEntry(arguments, "relation_id_" + priorityString, SwitchNone);
			switchState_t state = GetIntegerMapEntry(arguments, "relation_state_" + priorityString);
			if (switchId == SwitchNone)
			{
				continue;
			}
			Relation* relation = new Relation(ObjectTypeStreet, streetID, objectType, switchId, priority, state, LockStateFree);
			relations.push_back(relation);
			++priority;
		}

		string result;
		if (!manager.streetSave(streetID, name, relations, visible, posx, posy, posz, automode, fromTrack, fromDirection, toTrack, toDirection, FeedbackNone, result))
		{
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}
		HtmlReplyWithHeaderAndParagraph("Street &quot;" + name + "&quot; saved.");
	}

	void WebClient::handleStreetAskDelete(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);

		if (streetID == StreetNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown street");
			return;
		}

		const datamodel::Street* street = manager.getStreet(streetID);
		if (street == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown street");
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete street &quot;" + street->name + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the street &quot;" + street->name + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "streetdelete"))
			.AddContent(HtmlTagInputHidden("street", to_string(streetID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleStreetDelete(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);
		const datamodel::Street* street = manager.getStreet(streetID);
		if (street == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete street");
			return;
		}

		if (!manager.streetDelete(streetID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete street");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Street &quot;" + street->name + "&quot; deleted.");
	}

	void WebClient::handleStreetList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Streets"));
		HtmlTag table("table");
		const map<string,datamodel::Street*> streetList = manager.streetListByName();
		map<string,string> streetArgument;
		for (auto street : streetList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(street.first));
			string streetIdString = to_string(street.second->objectID);
			streetArgument["street"] = streetIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "streetedit_list_" + streetIdString, streetArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "streetaskdelete_" + streetIdString, streetArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "streetedit_0"));
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleStreetExecute(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);
		manager.executeStreetInParallel(streetID);
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

		std::map<string, string> typeOptions;
		typeOptions[to_string(TrackTypeStraight)] = "Straight";
		typeOptions[to_string(TrackTypeLeft)] = "Left";
		typeOptions[to_string(TrackTypeRight)] = "Right";

		content.AddChildTag(HtmlTag("h1").AddContent("Edit track &quot;" + name + "&quot;"));
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(HtmlTag("form").AddAttribute("id", "editform")
			.AddChildTag(HtmlTagInputHidden("cmd", "tracksave"))
			.AddChildTag(HtmlTagInputHidden("track", to_string(trackID)))
			.AddChildTag(HtmlTagInputTextWithLabel("name", "Track Name:", name))
			.AddChildTag(HtmlTagInputIntegerWithLabel("posx", "Pos X:", posx, 0, 255))
			.AddChildTag(HtmlTagInputIntegerWithLabel("posy", "Pos Y:", posy, 0, 255))
			/* FIXME: layers not supported
			.AddChildTag(HtmlTagInputIntegerWithLabel("posz", "Pos Z:", posz, 0, 20))
			*/
			.AddChildTag(HtmlTagInputIntegerWithLabel("length", "Length:", height, 1, 100))
			.AddChildTag(HtmlTagRotation(rotation))
			.AddChildTag(HtmlTagSelectWithLabel("type", "Type:", typeOptions, to_string(type)))
		));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleTrackSave(const map<string, string>& arguments)
	{
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
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Track &quot;" + name + "&quot; saved.");
	}

	void WebClient::handleTrackAskDelete(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);

		if (trackID == TrackNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown track");
			return;
		}

		const datamodel::Track* track = manager.getTrack(trackID);
		if (track == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown track");
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

	void WebClient::handleTrackList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Tracks"));
		HtmlTag table("table");
		const map<string,datamodel::Track*> trackList = manager.trackListByName();
		map<string,string> locoArgument;
		for (auto track : trackList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(track.first));
			string locoIdString = to_string(track.second->objectID);
			locoArgument["track"] = locoIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "trackedit_list_" + locoIdString, locoArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "trackaskdelete_" + locoIdString, locoArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "trackedit_0"));
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleTrackDelete(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		const datamodel::Track* track = manager.getTrack(trackID);
		if (track == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete track");
			return;
		}

		if (!manager.trackDelete(trackID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete track");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Track &quot;" + track->name + "&quot; deleted.");
	}

	void WebClient::handleTrackGet(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track");
		const datamodel::Track* track = manager.getTrack(trackID);
		HtmlReplyWithHeader(HtmlTagTrack(track));
	}

	void WebClient::handleLocoSelector(const map<string, string>& arguments)
	{
		HtmlReplyWithHeader(HtmlTagLocoSelector());
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

	HtmlTag WebClient::HtmlTagLocoSelector() const
	{
		const map<locoID_t, Loco*>& locos = manager.locoList();
		map<string,locoID_t> options;
		for (auto locoTMP : locos) {
			Loco* loco = locoTMP.second;
			options[loco->name] = loco->objectID;
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
			buttonArguments["speed"] = "102";
			ss << HtmlTagButtonCommand("10%", id + "_1", buttonArguments);
			buttonArguments["speed"] = "408";
			ss << HtmlTagButtonCommand("40%", id + "_2", buttonArguments);
			buttonArguments["speed"] = "714";
			ss << HtmlTagButtonCommand("70%", id + "_3", buttonArguments);
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
		menu.AddChildTag(HtmlTagButtonCommand("<svg width=\"35\" height=\"35\"><polygon points=\"15,0.5 30,0.5 30,24.5 15,24.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"20,10.5 30,0.5 30,24.5 20,34.5\" fill=\"black\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"0,10 7.5,10 7.5,5 15,12.5 7.5,20 7.5,15 0,15\"/></svg>", "quit"));
		menu.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menu.AddChildTag(HtmlTagButtonCommandToggle("<svg width=\"35\" height=\"35\"><polyline points=\"12.5,8.8 11.1,9.8 9.8,11.1 8.8,12.5 8.1,14.1 7.7,15.8 7.5,17.5 7.7,19.2 8.1,20.9 8.8,22.5 9.8,23.9 11.1,25.2 12.5,26.2 14.1,26.9 15.8,27.3 17.5,27.5 19.2,27.3 20.9,26.9 22.5,26.2 23.9,25.2 25.2,23.9 26.2,22.5 26.9,20.9 27.3,19.2 27.5,17.5 27.3,15.8 26.9,14.1 26.2,12.5 25.2,11.1 23.9,9.8 22.5,8.8\" stroke=\"black\" stroke-width=\"3\" fill=\"none\"/><polyline points=\"17.5,2.5 17.5,15\" stroke=\"black\" stroke-width=\"3\" fill=\"none\"/></svg>", "booster", false, buttonArguments).AddClass("button_booster"));
		menu.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menu.AddChildTag(HtmlTagButtonCommand("<svg width=\"35\" height=\"35\"><polyline points=\"1,11 1,10 10,1 25,1 34,10 34,25 25,34 10,34 1,25 1,11\" stroke=\"black\" stroke-width=\"1\" fill=\"red\"/><text x=\"3\" y=\"21\" fill=\"white\" font-size=\"11\">STOP</text></svg>", "stopall"));
		menu.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\" id=\"_img\"><polygon points=\"1,30 25,30 34,20 10,20\" fill=\"white\" stroke=\"black\"/><polygon points=\"1,25 25,25 34,15 10,15\" fill=\"white\" stroke=\"black\"/><polygon points=\"1,20 25,20 34,10 10,10\" fill=\"white\" stroke=\"black\"/><polygon points=\"1,15 25,15 34,5 10,5\" fill=\"white\" stroke=\"black\"/></svg>", "layerlist"));
		menu.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"10,0.5 25,0.5 25,34.5 10,34.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"13,3.5 22,3.5 22,7.5 13,7.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><circle cx=\"14.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"27.5\" r=\"5\" fill=\"black\"/></svg>", "controllist"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"0,10 5,10 5,0 10,0 10,10 25,10 25,0 35,0 35,5 30,5 30,10 35,10 35,25 0,25\" fill=\"black\"/><circle cx=\"5\" cy=\"30\" r=\"5\" fill=\"black\"/><circle cx=\"17.5\" cy=\"30\" r=\"5\" fill=\"black\"/><circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"black\"/</svg>", "locolist"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"1,12 34,12\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"1,23 34,23\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"3,10 3,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"6,10 6,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"9,10 9,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"12,10 12,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"15,10 15,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"18,10 18,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"21,10 21,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"24,10 24,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"27,10 27,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"30,10 30,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"33,10 33,25\" stroke=\"black\" stroke-width=\"1\"/></svg>", "tracklist"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"1,20 7.1,19.5 13,17.9 18.5,15.3 23.5,11.8 27.8,7.5\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"1,28 8.5,27.3 15.7,25.4 22.5,22.2 28.6,17.9 33.9,12.6\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"1,20 34,20\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"1,28 34,28\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"3,18 3,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"6,18 6,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"9,17 9,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"12,16 12,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"15,15 15,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"18,13 18,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"21,12 21,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"24,9 24,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"27,17 27,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"30,18 30,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"33,18 33,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"24,9 32,17\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"26,7 34,15\" stroke=\"black\" stroke-width=\"1\"/></svg>", "switchlist"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"1,20 10,20 30,15\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"28,17 28,20 34,20\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/></svg>", "accessorylist"));
		menu.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"5,34 15,1\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"30,34 20,1\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,34 17.5,30\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,24 17.5,20\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,14 17.5,10\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,4 17.5,1\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/></svg>", "streetlist"));
		body.AddChildTag(menu);

		body.AddChildTag(HtmlTag("div").AddClass("loco_selector").AddAttribute("id", "loco_selector").AddChildTag(HtmlTagLocoSelector()));
		body.AddChildTag(HtmlTag("div").AddClass("layout_selector").AddChildTag(HtmlTagSelectLayout()));
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
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add street").AddAttribute("onClick", "loadPopup('/?cmd=streetedit&street=0');"))
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
