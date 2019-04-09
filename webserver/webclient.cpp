#include <algorithm>
#include <cstring>		//memset
#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "datamodel/datamodel.h"
#include "railcontrol.h"
#include "timestamp.h"
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
#include "webserver/HtmlTagFeedback.h"
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
using datamodel::Feedback;
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
					if (errno != ETIMEDOUT)
					{
						return;
					}
					if (run == false)
					{
						return;
					}
					continue;
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
				stopRailControlWebserver();
			}
			else if (arguments["cmd"].compare("booster") == 0)
			{
				bool on = GetBoolMapEntry(arguments, "on");
				if (on)
				{
					HtmlReplyWithHeader(string("Turning booster on"));
					manager.Booster(ControlTypeWebserver, BoosterGo);
				}
				else
				{
					HtmlReplyWithHeader(string("Turning booster off"));
					manager.Booster(ControlTypeWebserver, BoosterStop);
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
			else if (arguments["cmd"].compare("locorelease") == 0)
			{
				handleLocoRelease(arguments);
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
			else if (arguments["cmd"].compare("accessoryrelease") == 0)
			{
				handleAccessoryRelease(arguments);
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
			else if (arguments["cmd"].compare("streetrelease") == 0)
			{
				handleStreetRelease(arguments);
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
			else if (arguments["cmd"].compare("tracksetloco") == 0)
			{
				handleTrackSetLoco(arguments);
			}
			else if (arguments["cmd"].compare("trackrelease") == 0)
			{
				handleTrackRelease(arguments);
			}
			else if (arguments["cmd"].compare("trackstartloco") == 0)
			{
				handleTrackStartLoco(arguments);
			}
			else if (arguments["cmd"].compare("trackstoploco") == 0)
			{
				handleTrackStopLoco(arguments);
			}
			else if (arguments["cmd"].compare("feedbackedit") == 0)
			{
				handleFeedbackEdit(arguments);
			}
			else if (arguments["cmd"].compare("feedbacksave") == 0)
			{
				handleFeedbackSave(arguments);
			}
			else if (arguments["cmd"].compare("feedbackstate") == 0)
			{
				handleFeedbackState(arguments);
			}
			else if (arguments["cmd"].compare("feedbacklist") == 0)
			{
				handleFeedbackList(arguments);
			}
			else if (arguments["cmd"].compare("feedbackaskdelete") == 0)
			{
				handleFeedbackAskDelete(arguments);
			}
			else if (arguments["cmd"].compare("feedbackdelete") == 0)
			{
				handleFeedbackDelete(arguments);
			}
			else if (arguments["cmd"].compare("feedbackget") == 0)
			{
				handleFeedbackGet(arguments);
			}
			else if (arguments["cmd"].compare("feedbacksoftrack") == 0)
			{
				handleFeedbacksOfTrack(arguments);
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
			else if (arguments["cmd"].compare("feedbackadd") == 0)
			{
				handleFeedbackAdd(arguments);
			}
			else if (arguments["cmd"].compare("relationadd") == 0)
			{
				handleRelationAdd(arguments);
			}
			else if (arguments["cmd"].compare("relationobject") == 0)
			{
				handleRelationObject(arguments);
			}
			else if (arguments["cmd"].compare("layout") == 0)
			{
				handleLayout(arguments);
			}
			else if (arguments["cmd"].compare("locoselector") == 0)
			{
				handleLocoSelector(arguments);
			}
			else if (arguments["cmd"].compare("layerselector") == 0)
			{
				handleLayerSelector(arguments);
			}
			else if (arguments["cmd"].compare("stopall") == 0)
			{
				manager.StopAllLocosImmediately(ControlTypeWebserver);
			}
			else if (arguments["cmd"].compare("settingsedit") == 0)
			{
				handleSettingsEdit(arguments);
			}
			else if (arguments["cmd"].compare("settingssave") == 0)
			{
				handleSettingsSave(arguments);
			}
			else if (arguments["cmd"].compare("timestamp") == 0)
			{
				handleTimestamp(arguments);
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
				if (argument.length() == 0)
				{
					continue;
				}
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
				name = layer->GetName();
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

		HtmlReplyWithHeader(HtmlTag("p").AddContent("Layer &quot;" + name + "&quot; saved."));
	}

	void WebClient::handleLayerAskDelete(const map<string, string>& arguments)
	{
		layerID_t layerID = GetIntegerMapEntry(arguments, "layer", LayerNone);

		if (layerID == LayerNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		if (layerID == LayerUndeletable)
		{
			HtmlReplyWithHeaderAndParagraph("Not allowed to delete this layer");
			return;
		}

		const Layer* layer = manager.GetLayer(layerID);
		if (layer == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		HtmlTag content;
		content.AddContent(HtmlTag("h1").AddContent("Delete layer &quot;" + layer->GetName() + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the layer &quot;" + layer->GetName() + "&quot;?"));
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

		if (layerID == LayerNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		if (layerID == LayerUndeletable)
		{
			HtmlReplyWithHeaderAndParagraph("Not allowed to delete this layer");
			return;
		}

		const Layer* layer = manager.GetLayer(layerID);
		if (layer == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown layer");
			return;
		}

		string name = layer->GetName();

		if (!manager.LayerDelete(layerID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete layer");
			return;
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent("Layer &quot;" + name + "&quot; deleted."));
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
			if (layer.second != LayerUndeletable)
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
			hardware::HardwareParams* params = manager.GetHardware(controlID);
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

		const std::map<hardwareType_t,string> hardwares = manager.HardwareListNames();
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

		if (!manager.ControlSave(controlID, hardwareType, name, arg1, arg2, arg3, arg4, arg5, result))
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

		const hardware::HardwareParams* control = manager.GetHardware(controlID);
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
		const hardware::HardwareParams* control = manager.GetHardware(controlID);
		if (control == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete control");
			return;
		}

		string name = control->name;

		if (!manager.ControlDelete(controlID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete control");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Control &quot;" + name + "&quot; deleted.");
	}

	void WebClient::handleControlList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Controls"));
		HtmlTag table("table");
		const map<string,hardware::HardwareParams*> hardwareList = manager.ControlListByName();
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
		locoSpeed_t speed = GetIntegerMapEntry(arguments, "speed", MinSpeed);

		manager.LocoSpeed(ControlTypeWebserver, locoID, speed);

		stringstream ss;
		ss << "Loco &quot;" << manager.GetLocoName(locoID) << "&quot; is now set to speed " << speed;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoDirection(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		direction_t direction = (GetBoolMapEntry(arguments, "on") ? DirectionRight : DirectionLeft);

		manager.LocoDirection(ControlTypeWebserver, locoID, direction);

		stringstream ss;
		ss << "Loco &quot;" << manager.GetLocoName(locoID) << "&quot; is now set to " << direction;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoFunction(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		function_t function = GetIntegerMapEntry(arguments, "function", 0);
		bool on = GetBoolMapEntry(arguments, "on");

		manager.LocoFunction(ControlTypeWebserver, locoID, function, on);

		stringstream ss;
		ss << "Loco &quot;" << manager.GetLocoName(locoID) << "&quot; has now f";
		ss << function << " set to " << (on ? "on" : "off");
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleLocoRelease(const map<string, string>& arguments)
	{
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco");
		bool ret = manager.LocoRelease(locoID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Loco released" : "Loco not released"));
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
		Loco* loco = manager.GetLoco(locoId);
		if (loco == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown loco"));
			return;
		}
		HtmlReplyWithHeader(HtmlTagProtocolLoco(controlId, loco->GetProtocol()));
	}

	HtmlTag WebClient::HtmlTagProtocolAccessory(const controlID_t controlID, const protocol_t selectedProtocol)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTagLabel("Protocol:", "protocol"));
		map<string,protocol_t> protocolMap = manager.AccessoryProtocolsOfControl(controlID);
		content.AddChildTag(HtmlTagSelect("protocol", protocolMap, selectedProtocol));
		return content;
	}

	HtmlTag WebClient::HtmlTagDuration(const accessoryDuration_t duration, const string& label) const
	{
		std::map<string,string> durationOptions;
		durationOptions["0000"] = "0";
		durationOptions["0100"] = "100";
		durationOptions["0250"] = "250";
		durationOptions["1000"] = "1000";
		return HtmlTagSelectWithLabel("duration", label, durationOptions, toStringWithLeadingZeros(duration, 4));
	}

	HtmlTag WebClient::HtmlTagPosition(const layoutPosition_t posx, const layoutPosition_t posy, const layoutPosition_t posz)
	{
		HtmlTag content("div");
		content.AddAttribute("id", "position");;
		content.AddChildTag(HtmlTagInputIntegerWithLabel("posx", "Pos X:", posx, 0, 255));
		content.AddChildTag(HtmlTagInputIntegerWithLabel("posy", "Pos Y:", posy, 0, 255));
		map<string,layerID_t> layerList = manager.LayerListByName();
		content.AddChildTag(HtmlTagSelectWithLabel("posz", "Pos Z:", layerList, posz));
		return content;
	}

	HtmlTag WebClient::HtmlTagPosition(const layoutPosition_t posx, const layoutPosition_t posy, const layoutPosition_t posz, const visible_t visible)
	{
		HtmlTag content;
		HtmlTagInputCheckboxWithLabel checkboxVisible("visible", "Visible:", "visible", static_cast<bool>(visible));
		checkboxVisible.AddAttribute("id", "visible");
		checkboxVisible.AddAttribute("onchange", "onChangeCheckboxShowHide('visible', 'position');");
		content.AddChildTag(checkboxVisible);
		HtmlTag posDiv = HtmlTagPosition(posx, posy, posz);
		if (visible == VisibleNo)
		{
			posDiv.AddAttribute("hidden");
		}
		content.AddChildTag(posDiv);
		return content;
	}

	HtmlTag WebClient::HtmlTagRelationObject(const string& priority, const objectType_t objectType, const objectID_t objectId, const accessoryState_t state)
	{
		HtmlTag content;
		switch (objectType)
		{
			case ObjectTypeSwitch:
			{
				std::map<string, Switch*> switches = manager.SwitchListByName();
				map<string, switchID_t> switchOptions;
				for (auto mySwitch : switches)
				{
					switchOptions[mySwitch.first] = mySwitch.second->GetID();
				}
				content.AddChildTag(HtmlTagSelect("relation_id_" + priority, switchOptions, objectId).AddClass("select_relation_id"));

				map<string, switchState_t> stateOptions;
				stateOptions["Straight"] = SwitchStateStraight;
				stateOptions["Turnout"] = SwitchStateTurnout;
				content.AddChildTag(HtmlTagSelect("relation_state_" + priority, stateOptions, state).AddClass("select_relation_state"));
				return content;
			}

			case ObjectTypeAccessory:
			{
				std::map<string, Accessory*> accessories = manager.AccessoryListByName();
				map<string, accessoryID_t> accessoryOptions;
				for (auto accessory : accessories)
				{
					accessoryOptions[accessory.first] = accessory.second->GetID();
				}
				content.AddChildTag(HtmlTagSelect("relation_id_" + priority, accessoryOptions, objectId).AddClass("select_relation_id"));

				map<string, accessoryState_t> stateOptions;
				stateOptions["on"] = AccessoryStateOn;
				stateOptions["off"] = AccessoryStateOff;
				content.AddChildTag(HtmlTagSelect("relation_state_" + priority, stateOptions, state).AddClass("select_relation_state"));
				return content;
			}

			case ObjectTypeTrack:
			{
				std::map<string, Track*> tracks = manager.TrackListByName();
				map<string, trackID_t> trackOptions;
				for (auto track : tracks)
				{
					trackOptions[track.first] = track.second->GetID();
				}
				content.AddChildTag(HtmlTagSelect("relation_id_" + priority, trackOptions, objectId).AddClass("select_relation_id"));
				return content;
			}

			default:
			{
				content.AddContent("Unknown objecttype");
				return content;
			}
		}
	}

	HtmlTag WebClient::HtmlTagRelation(const string& priority, const objectType_t objectType, const objectID_t objectId, const accessoryState_t state)
	{
		HtmlTag content("div");
		content.AddAttribute("id", "priority_" + priority);
		HtmlTagButton deleteButton("Del", "delete_relation_" + priority);
		deleteButton.AddAttribute("onclick", "deleteElement('priority_" + priority + "');return false;");
		content.AddChildTag(deleteButton);

		map<string,objectType_t> objectTypeOptions;
		objectTypeOptions["Accessory"] = ObjectTypeAccessory;
		objectTypeOptions["Switch"] = ObjectTypeSwitch;
		objectTypeOptions["Track"] = ObjectTypeTrack;
		HtmlTagSelect select("relation_type_" + priority, objectTypeOptions, objectType);
		select.AddClass("select_relation_objecttype");
		select.AddAttribute("onchange", "loadRelationObject(" + priority + ");return false;");
		content.AddChildTag(select);
		HtmlTag contentObject("div");
		contentObject.AddAttribute("id", "relation_object_" + priority);
		contentObject.AddClass("inline-block");
		contentObject.AddChildTag(HtmlTagRelationObject(priority, objectType, objectId, state));
		content.AddChildTag(contentObject);
		return content;
	}

	HtmlTag WebClient::HtmlTagSelectFeedbackForTrack(const unsigned int counter, const trackID_t trackID, const feedbackID_t feedbackID)
	{
		string counterString = to_string(counter);
		HtmlTag content("div");
		content.AddAttribute("id", "feedback_container_" + counterString);
		HtmlTagButton deleteButton("Del", "delete_feedback_" + counterString);
		deleteButton.AddAttribute("onclick", "deleteElement('feedback_container_" + counterString + "');return false;");
		content.AddChildTag(deleteButton);

		map<string, Feedback*> feedbacks = manager.FeedbackListByName();
		map<string, feedbackID_t> feedbackOptions;
		for (auto feedback : feedbacks)
		{
			const trackID_t trackIDOfFeedback = feedback.second->GetTrack();
			if (trackIDOfFeedback == TrackNone || trackIDOfFeedback == trackID)
			{
				feedbackOptions[feedback.first] = feedback.second->GetID();
			}
		}
		content.AddChildTag(HtmlTagSelect("feedback_" + counterString, feedbackOptions, feedbackID));
		content.AddChildTag(HtmlTag("div").AddAttribute("id", "div_feedback_" + to_string(counter + 1)));
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

	HtmlTag WebClient::HtmlTagSelectTrack(const std::string& name, const std::string& label, const trackID_t trackId, const direction_t direction, const string& onchange) const
	{
		HtmlTag tag;
		map<string,trackID_t> tracks = manager.TrackListIdByName();
		HtmlTagSelectWithLabel selectTrack(name + "track", label, tracks, trackId);
		selectTrack.AddClass("select_track");
		if (onchange.size() > 0)
		{
			selectTrack.AddAttribute("onchange", onchange);
		}
		tag.AddChildTag(selectTrack);
		map<string,direction_t> directions;
		directions["Left"] = DirectionLeft;
		directions["Right"] = DirectionRight;
		tag.AddChildTag(HtmlTagSelect(name + "direction", directions, direction).AddClass("select_direction"));
		return tag;
	}

	HtmlTag WebClient::HtmlTagSelectFeedbacksOfTrack(const trackID_t trackId, const feedbackID_t feedbackIdReduced, const feedbackID_t feedbackIdCreep, const feedbackID_t feedbackIdStop, const feedbackID_t feedbackIdOver) const
	{
		HtmlTag tag;
		map<string,feedbackID_t> feedbacks = manager.FeedbacksOfTrack(trackId);
		map<string,feedbackID_t> feedbacksWithNone = feedbacks;
		feedbacksWithNone["-"] = FeedbackNone;
		tag.AddChildTag(HtmlTagSelectWithLabel("feedbackreduced", "Reduce speed at:", feedbacksWithNone, feedbackIdReduced).AddClass("select_feedback"));
		tag.AddChildTag(HtmlTagSelectWithLabel("feedbackcreep", "Creep at:", feedbacksWithNone, feedbackIdCreep).AddClass("select_feedback"));
		tag.AddChildTag(HtmlTagSelectWithLabel("feedbackstop", "Stop at:", feedbacks, feedbackIdStop).AddClass("select_feedback"));
		tag.AddChildTag(HtmlTagSelectWithLabel("feedbackover", "Overrun at:", feedbacksWithNone, feedbackIdOver).AddClass("select_feedback"));
		return tag;
	}

	HtmlTag WebClient::HtmlTagTabMenuItem(const std::string& tabName, const std::string& buttonValue, const bool selected) const
	{
		HtmlTag button("button");
		button.AddClass("tab_button");
		button.AddAttribute("id", "tab_button_" + tabName);
		button.AddAttribute("onclick", "ShowTab('" + tabName + "');");
		button.AddContent(buttonValue);
		if (selected)
		{
			button.AddClass("tab_button_selected");
		}
		return button;
	}

	HtmlTag WebClient::HtmlTagSelectSelectStreetApproach(const datamodel::Track::selectStreetApproach_t selectStreetApproach, const bool addDefault)
	{
		map<string,string> options;
		if (addDefault)
		{
			options[to_string(datamodel::Track::SelectStreetSystemDefault)] = "Use system default";
		}
		options[to_string(datamodel::Track::SelectStreetDoNotCare)] = "Do not care";
		options[to_string(datamodel::Track::SelectStreetRandom)] = "Random";
		options[to_string(datamodel::Track::SelectStreetMinTrackLength)] = "Minimal destination track length";
		options[to_string(datamodel::Track::SelectStreetLongestUnused)] = "Longest unused";
		return HtmlTagSelectWithLabel("selectstreetapproach", "Select street by:", options, to_string(static_cast<int>(selectStreetApproach)));
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
		Accessory* accessory = manager.GetAccessory(accessoryId);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown accessory"));
			return;
		}
		HtmlReplyWithHeader(HtmlTagProtocolAccessory(controlId, accessory->GetProtocol()));
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

	void WebClient::handleFeedbackAdd(const map<string, string>& arguments)
	{
		unsigned int counter = GetIntegerMapEntry(arguments, "counter", 1);
		trackID_t trackID = static_cast<trackID_t>(GetIntegerMapEntry(arguments, "track", TrackNone));
		HtmlReplyWithHeader(HtmlTagSelectFeedbackForTrack(counter, trackID));
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
		Switch* mySwitch = manager.GetSwitch(switchId);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeader(HtmlTag().AddContent("Unknown switch"));
			return;
		}
		HtmlReplyWithHeader(HtmlTagProtocolAccessory(controlId, mySwitch->GetProtocol()));
	}

	void WebClient::handleRelationObject(const map<string, string>& arguments)
	{
		const string priority = GetStringMapEntry(arguments, "priority");
		const objectType_t objectType = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objecttype"));
		HtmlReplyWithHeader(HtmlTagRelationObject(priority, objectType));
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
		bool commuter = false;
		length_t length = 0;
		locoSpeed_t maxSpeed = MaxSpeed;
		locoSpeed_t travelSpeed = DefaultTravelSpeed;
		locoSpeed_t reducedSpeed = DefaultReducedSpeed;
		locoSpeed_t creepSpeed = DefaultCreepSpeed;

		if (locoID > LocoNone)
		{
			const datamodel::Loco* loco = manager.GetLoco(locoID);
			controlID = loco->GetControlID();
			protocol = loco->GetProtocol();
			address = loco->GetAddress();
			name = loco->GetName();
			nrOfFunctions = loco->GetNrOfFunctions();
			commuter = loco->GetCommuter();
			length = loco->GetLength();
			maxSpeed = loco->GetMaxSpeed();
			travelSpeed = loco->GetTravelSpeed();
			reducedSpeed = loco->GetReducedSpeed();
			creepSpeed = loco->GetCreepSpeed();
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
		HtmlTag tabMenu("div");
		tabMenu.AddChildTag(HtmlTagTabMenuItem("basic", "Basic", true));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("automode", "Automode"));
		content.AddChildTag(tabMenu);

		HtmlTag formContent;
		formContent.AddChildTag(HtmlTagInputHidden("cmd", "locosave"));
		formContent.AddChildTag(HtmlTagInputHidden("loco", to_string(locoID)));

		HtmlTag basicContent("div");
		basicContent.AddAttribute("id", "tab_basic");
		basicContent.AddClass("tab_content");
		basicContent.AddChildTag(HtmlTagInputTextWithLabel("name", "Loco Name:", name));
		basicContent.AddChildTag(HtmlTagSelectWithLabel("control", "Control:", controlOptions, to_string(controlID)).AddAttribute("onchange", "loadProtocol('loco', " + to_string(locoID) + ")"));
		basicContent.AddChildTag(HtmlTag("div").AddAttribute("id", "select_protocol").AddChildTag(HtmlTagProtocolLoco(controlID, protocol)));
		basicContent.AddChildTag(HtmlTagInputIntegerWithLabel("address", "Address:", address, 1, 9999));
		basicContent.AddChildTag(HtmlTagInputIntegerWithLabel("function", "# of functions:", nrOfFunctions, 0, datamodel::LocoFunctions::maxFunctions));
		basicContent.AddChildTag(HtmlTagInputIntegerWithLabel("length", "Train length:", length, 0, 99999));
		formContent.AddChildTag(basicContent);

		HtmlTag automodeContent("div");
		automodeContent.AddAttribute("id", "tab_automode");
		automodeContent.AddClass("tab_content");
		automodeContent.AddClass("hidden");
		automodeContent.AddChildTag(HtmlTagInputCheckboxWithLabel("commuter", "Commuter:", "commuter", commuter));
		automodeContent.AddChildTag(HtmlTagInputIntegerWithLabel("maxspeed", "Maximum speed:", maxSpeed, 0, MaxSpeed));
		automodeContent.AddChildTag(HtmlTagInputIntegerWithLabel("travelspeed", "Travel speed:", travelSpeed, 0, MaxSpeed));
		automodeContent.AddChildTag(HtmlTagInputIntegerWithLabel("reducedspeed", "Reduced speed:", reducedSpeed, 0, MaxSpeed));
		automodeContent.AddChildTag(HtmlTagInputIntegerWithLabel("creepspeed", "Creep speed:", creepSpeed, 0, MaxSpeed));
		formContent.AddChildTag(automodeContent);

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(HtmlTag("form").AddAttribute("id", "editform").AddChildTag(formContent)));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleLocoSave(const map<string, string>& arguments)
	{
		const locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		const string name = GetStringMapEntry(arguments, "name");
		const controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		const protocol_t protocol = static_cast<protocol_t>(GetIntegerMapEntry(arguments, "protocol", ProtocolNone));
		const address_t address = GetIntegerMapEntry(arguments, "address", AddressNone);
		const function_t nrOfFunctions = GetIntegerMapEntry(arguments, "function", 0);
		const length_t length = GetIntegerMapEntry(arguments, "length", 0);
		const bool commuter = GetBoolMapEntry(arguments, "commuter", false);
		const locoSpeed_t maxSpeed = GetIntegerMapEntry(arguments, "maxspeed", MaxSpeed);
		locoSpeed_t travelSpeed = GetIntegerMapEntry(arguments, "travelspeed", DefaultTravelSpeed);
		if (travelSpeed > maxSpeed)
		{
			travelSpeed = maxSpeed;
		}
		locoSpeed_t reducedSpeed = GetIntegerMapEntry(arguments, "reducedspeed", DefaultReducedSpeed);
		if (reducedSpeed > travelSpeed)
		{
			reducedSpeed = travelSpeed;
		}
		locoSpeed_t creepSpeed = GetIntegerMapEntry(arguments, "creepspeed", DefaultCreepSpeed);
		if (creepSpeed > reducedSpeed)
		{
			creepSpeed = reducedSpeed;
		}
		string result;

		if (!manager.LocoSave(locoID,
			name,
			controlId,
			protocol,
			address,
			nrOfFunctions,
			length,
			commuter,
			maxSpeed,
			travelSpeed,
			reducedSpeed,
			creepSpeed,
			result))
		{
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent("Loco &quot;" + name + "&quot; saved."));
	}

	void WebClient::handleLocoList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Locos"));
		HtmlTag table("table");
		const map<string,datamodel::Loco*> locoList = manager.LocoListByName();
		map<string,string> locoArgument;
		for (auto loco : locoList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(loco.first));
			row.AddChildTag(HtmlTag("td").AddContent(to_string(loco.second->GetAddress())));
			string locoIdString = to_string(loco.second->GetID());
			locoArgument["loco"] = locoIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "locoedit_list_" + locoIdString, locoArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "locoaskdelete_" + locoIdString, locoArgument)));
			if (loco.second->GetTrack() != TrackNone || loco.second->GetStreet() != StreetNone)
			{
				row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonCommand("Release", "locorelease_" + locoIdString, locoArgument)));
			}
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

		const datamodel::Loco* loco = manager.GetLoco(locoID);
		if (loco == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown loco");
			return;
		}

		HtmlTag content;
		const string& locoName = loco->GetName();
		content.AddContent(HtmlTag("h1").AddContent("Delete loco &quot;" + locoName + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the loco &quot;" + locoName + "&quot;?"));
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
		const datamodel::Loco* loco = manager.GetLoco(locoID);
		if (loco == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete loco");
			return;
		}

		string name = loco->GetName();

		if (!manager.LocoDelete(locoID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete loco");
			return;
		}

		HtmlReplyWithHeader(HtmlTag("p").AddContent("Loco &quot;" + name + "&quot; deleted."));
	}

	HtmlTag WebClient::HtmlTagLayerSelector() const
	{
		map<string,layerID_t> options = manager.LayerListByNameWithFeedback();
		return HtmlTagSelect("layer", options).AddAttribute("onchange", "loadLayout();");
	}

	void WebClient::handleLayout(const map<string, string>& arguments)
	{
		layerID_t layer = static_cast<layerID_t>(GetIntegerMapEntry(arguments, "layer", CHAR_MIN));
		HtmlTag content;

		if (layer < LayerUndeletable)
		{
			const map<feedbackID_t,Feedback*>& feedbacks = manager.FeedbackList();
			for (auto feedback : feedbacks)
			{
				if (feedback.second->GetControlID() != -layer)
				{
					continue;
				}
				feedbackPin_t pin = feedback.second->GetPin() - 1;
				layoutPosition_t x = pin & 0x0F; // => % 16;
				layoutPosition_t y = pin >> 4;   // => / 16;
				if (x >= 8)
				{
					++x;
				}
				content.AddChildTag(HtmlTagFeedback(feedback.second, x, y));
			}
			HtmlReplyWithHeader(content);
			return;
		}

		const map<accessoryID_t,datamodel::Accessory*>& accessories = manager.AccessoryList();
		for (auto accessory : accessories)
		{
			if (accessory.second->IsVisibleOnLayer(layer) == false)
			{
				continue;
			}
			content.AddChildTag(HtmlTagAccessory(accessory.second));
		}

		const map<switchID_t,datamodel::Switch*>& switches = manager.SwitchList();
		for (auto mySwitch : switches)
		{
			if (mySwitch.second->IsVisibleOnLayer(layer) == false)
			{
				continue;
			}
			content.AddChildTag(HtmlTagSwitch(mySwitch.second));
		}

		const map<switchID_t,datamodel::Track*>& tracks = manager.TrackList();
		for (auto track : tracks)
		{
			if (track.second->IsVisibleOnLayer(layer) == false)
			{
				continue;
			}
			content.AddChildTag(HtmlTagTrack(manager, track.second));
		}

		const map<streetID_t,datamodel::Street*>& streets = manager.StreetList();
		for (auto street : streets)
		{
			if (street.second->IsVisibleOnLayer(layer) == false)
			{
				continue;
			}
			content.AddChildTag(HtmlTagStreet(street.second));
		}

		const map<feedbackID_t,Feedback*>& feedbacks = manager.FeedbackList();
		for (auto feedback : feedbacks)
		{
			if (feedback.second->IsVisibleOnLayer(layer) == false)
			{
				continue;
			}
			content.AddChildTag(HtmlTagFeedback(feedback.second));
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
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", LayerUndeletable);
		accessoryDuration_t duration = manager.GetDefaultAccessoryDuration();
		bool inverted = false;
		if (accessoryID > AccessoryNone)
		{
			const datamodel::Accessory* accessory = manager.GetAccessory(accessoryID);
			controlID = accessory->GetControlID();
			protocol = accessory->GetProtocol();
			address = accessory->GetAddress();
			name = accessory->GetName();
			posx = accessory->GetPosX();
			posy = accessory->GetPosY();
			posz = accessory->GetPosZ();
			inverted = accessory->GetInverted();
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
			.AddChildTag(HtmlTagDuration(duration))
			.AddChildTag(HtmlTagInputCheckboxWithLabel("inverted", "Inverted:", "true", inverted))
			.AddChildTag(HtmlTagPosition(posx, posy, posz))
		));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleAccessoryGet(const map<string, string>& arguments)
	{
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory");
		const datamodel::Accessory* accessory = manager.GetAccessory(accessoryID);
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
		accessoryDuration_t duration = GetIntegerMapEntry(arguments, "duration", manager.GetDefaultAccessoryDuration());
		bool inverted = GetBoolMapEntry(arguments, "inverted");
		string result;
		if (!manager.AccessorySave(accessoryID, name, posX, posY, posZ, controlId, protocol, address, AccessoryTypeDefault, duration, inverted, result))
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

		manager.AccessoryState(ControlTypeWebserver, accessoryID, accessoryState, false);

		stringstream ss;
		ss << "Accessory &quot;" << manager.GetAccessoryName(accessoryID) << "&quot; is now set to " << accessoryState;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleAccessoryList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Accessories"));
		HtmlTag table("table");
		const map<string,datamodel::Accessory*> accessoryList = manager.AccessoryListByName();
		map<string,string> accessoryArgument;
		for (auto accessory : accessoryList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(accessory.first));
			string accessoryIdString = to_string(accessory.second->GetID());
			accessoryArgument["accessory"] = accessoryIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "accessoryedit_list_" + accessoryIdString, accessoryArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "accessoryaskdelete_" + accessoryIdString, accessoryArgument)));
			if (accessory.second->IsInUse())
			{
				row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonCommand("Release", "accessoryrelease_" + accessoryIdString, accessoryArgument)));
			}
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

		const datamodel::Accessory* accessory = manager.GetAccessory(accessoryID);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown accessory");
			return;
		}

		HtmlTag content;
		const string& accessoryName = accessory->GetName();
		content.AddContent(HtmlTag("h1").AddContent("Delete accessory &quot;" + accessoryName + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the accessory &quot;" + accessoryName + "&quot;?"));
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
		const datamodel::Accessory* accessory = manager.GetAccessory(accessoryID);
		if (accessory == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete accessory");
			return;
		}

		string name = accessory->GetName();

		if (!manager.AccessoryDelete(accessoryID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete accessory");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Accessory &quot;" + name + "&quot; deleted.");
	}

	void WebClient::handleAccessoryRelease(const map<string, string>& arguments)
	{
		accessoryID_t accessoryID = GetIntegerMapEntry(arguments, "accessory");
		bool ret = manager.AccessoryRelease(accessoryID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Accessory released" : "Accessory not released"));
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
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", LayerUndeletable);
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		switchType_t type = SwitchTypeLeft;
		accessoryDuration_t duration = manager.GetDefaultAccessoryDuration();
		bool inverted = false;
		if (switchID > SwitchNone)
		{
			const datamodel::Switch* mySwitch = manager.GetSwitch(switchID);
			controlID = mySwitch->GetControlID();
			protocol = mySwitch->GetProtocol();
			address = mySwitch->GetAddress();
			name = mySwitch->GetName();
			posx = mySwitch->GetPosX();
			posy = mySwitch->GetPosY();
			posz = mySwitch->GetPosZ();
			rotation = mySwitch->GetRotation();
			type = mySwitch->GetType();
			duration = mySwitch->GetDuration();
			inverted = mySwitch->GetInverted();
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
		HtmlTag tabMenu("div");
		tabMenu.AddChildTag(HtmlTagTabMenuItem("main", "Main", true));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("position", "Position"));
		content.AddChildTag(tabMenu);

		HtmlTag formContent;
		formContent.AddChildTag(HtmlTagInputHidden("cmd", "switchsave"));
		formContent.AddChildTag(HtmlTagInputHidden("switch", to_string(switchID)));

		HtmlTag mainContent("div");
		mainContent.AddAttribute("id", "tab_main");
		mainContent.AddClass("tab_content");
		mainContent.AddChildTag(HtmlTagInputTextWithLabel("name", "Switch Name:", name));
		mainContent.AddChildTag(HtmlTagSelectWithLabel("type", "Type:", typeOptions, to_string(type)));
		mainContent.AddChildTag(HtmlTagSelectWithLabel("control", "Control:", controlOptions, to_string(controlID)).AddAttribute("onchange", "loadProtocol('switch', " + to_string(switchID) + ")"));
		mainContent.AddChildTag(HtmlTag("div").AddAttribute("id", "select_protocol").AddChildTag(HtmlTagProtocolAccessory(controlID, protocol)));
		mainContent.AddChildTag(HtmlTagInputIntegerWithLabel("address", "Address:", address, 1, 2044));
		mainContent.AddChildTag(HtmlTagDuration(duration));
		mainContent.AddChildTag(HtmlTagInputCheckboxWithLabel("inverted", "Inverted:", "true", inverted));
		formContent.AddChildTag(mainContent);

		HtmlTag positionContent("div");
		positionContent.AddAttribute("id", "tab_position");
		positionContent.AddClass("tab_content");
		positionContent.AddClass("hidden");
		positionContent.AddChildTag(HtmlTagPosition(posx, posy, posz));
		positionContent.AddChildTag(HtmlTagRotation(rotation));
		formContent.AddChildTag(positionContent);

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(HtmlTag("form").AddAttribute("id", "editform").AddChildTag(formContent)));
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
		accessoryDuration_t duration = GetIntegerMapEntry(arguments, "duration", manager.GetDefaultAccessoryDuration());
		bool inverted = GetBoolMapEntry(arguments, "inverted");
		string result;
		if (!manager.SwitchSave(switchID, name, posX, posY, posZ, rotation, controlId, protocol, address, type, duration, inverted, result))
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

		manager.SwitchState(ControlTypeWebserver, switchID, switchState, false);

		stringstream ss;
		ss << "Switch &quot;" << manager.GetSwitchName(switchID) << "&quot; is now set to " << switchState;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleSwitchList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Switches"));
		HtmlTag table("table");
		const map<string,datamodel::Switch*> switchList = manager.SwitchListByName();
		map<string,string> switchArgument;
		for (auto mySwitch : switchList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(mySwitch.first));
			string switchIdString = to_string(mySwitch.second->GetID());
			switchArgument["switch"] = switchIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "switchedit_list_" + switchIdString, switchArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "switchaskdelete_" + switchIdString, switchArgument)));
			if (mySwitch.second->IsInUse())
			{
				row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonCommand("Release", "switchrelease_" + switchIdString, switchArgument)));
			}
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

		const datamodel::Switch* mySwitch = manager.GetSwitch(switchID);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown switch");
			return;
		}

		HtmlTag content;
		const string& switchName = mySwitch->GetName();
		content.AddContent(HtmlTag("h1").AddContent("Delete switch &quot;" + switchName + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the switch &quot;" + switchName + "&quot;?"));
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
		const datamodel::Switch* mySwitch = manager.GetSwitch(switchID);
		if (mySwitch == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete switch");
			return;
		}

		string name = mySwitch->GetName();

		if (!manager.SwitchDelete(switchID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete switch");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Switch &quot;" + name + "&quot; deleted.");
	}

	void WebClient::handleSwitchGet(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch");
		const datamodel::Switch* mySwitch = manager.GetSwitch(switchID);
		HtmlReplyWithHeader(HtmlTagSwitch(mySwitch));
	}

	void WebClient::handleSwitchRelease(const map<string, string>& arguments)
	{
		switchID_t switchID = GetIntegerMapEntry(arguments, "switch");
		bool ret = manager.SwitchRelease(switchID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Switch released" : "Switch not released"));
	}

	void WebClient::handleStreetGet(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street");
		const datamodel::Street* street = manager.GetStreet(streetID);
		if (street->GetVisible() == VisibleNo)
		{
			HtmlReplyWithHeader(HtmlTag());
			return;
		}
		HtmlReplyWithHeader(HtmlTagStreet(street));
	}

	void WebClient::handleStreetEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);
		string name("New Street");
		delay_t delay = Street::DefaultDelay;
		Street::commuterType_t commuter = Street::CommuterTypeBoth;
		length_t minTrainLength = 0;
		length_t maxTrainLength = 0;
		vector<Relation*> relations;
		visible_t visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible", VisibleYes));
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", LayerUndeletable);
		automode_t automode = static_cast<automode_t>(GetBoolMapEntry(arguments, "automode", AutomodeNo));
		trackID_t fromTrack = GetIntegerMapEntry(arguments, "fromtrack", TrackNone);
		direction_t fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromdirection", DirectionRight));
		trackID_t toTrack = GetIntegerMapEntry(arguments, "totrack", TrackNone);
		direction_t toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "todirection", DirectionLeft));
		feedbackID_t feedbackIdReduced = GetIntegerMapEntry(arguments, "feedbackreduced", FeedbackNone);
		feedbackID_t feedbackIdCreep = GetIntegerMapEntry(arguments, "feedbackcreep", FeedbackNone);
		feedbackID_t feedbackIdStop = GetIntegerMapEntry(arguments, "feedbackstop", FeedbackNone);
		feedbackID_t feedbackIdOver = GetIntegerMapEntry(arguments, "feedbackover", FeedbackNone);
		if (streetID > StreetNone)
		{
			const datamodel::Street* street = manager.GetStreet(streetID);
			name = street->GetName();
			delay = street->GetDelay();
			commuter = street->GetCommuter();
			minTrainLength = street->GetMinTrainLength();
			maxTrainLength = street->GetMaxTrainLength();
			relations = street->GetRelations();
			visible = street->GetVisible();
			posx = street->GetPosX();
			posy = street->GetPosY();
			posz = street->GetPosZ();
			automode = street->GetAutomode();
			fromTrack = street->GetFromTrack();
			fromDirection = street->GetFromDirection();
			toTrack = street->GetToTrack();
			toDirection = street->GetToDirection();
			feedbackIdReduced = street->GetFeedbackIdReduced();
			feedbackIdCreep = street->GetFeedbackIdCreep();
			feedbackIdStop = street->GetFeedbackIdStop();
			feedbackIdOver = street->GetFeedbackIdOver();
		}

		content.AddChildTag(HtmlTag("h1").AddContent("Edit street &quot;" + name + "&quot;"));
		HtmlTag tabMenu("div");
		tabMenu.AddChildTag(HtmlTagTabMenuItem("basic", "Basic", true));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("relation", "Relations"));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("position", "Position"));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("automode", "Auto-mode"));
		content.AddChildTag(tabMenu);

		HtmlTag formContent("form");
		formContent.AddAttribute("id", "editform");
		formContent.AddChildTag(HtmlTagInputHidden("cmd", "streetsave"));
		formContent.AddChildTag(HtmlTagInputHidden("street", to_string(streetID)));

		HtmlTag basicContent("div");
		basicContent.AddAttribute("id", "tab_basic");
		basicContent.AddClass("tab_content");
		basicContent.AddChildTag(HtmlTagInputTextWithLabel("name", "Street Name:", name));
		basicContent.AddChildTag(HtmlTagInputIntegerWithLabel("delay", "Delay in ms:", delay, 1, USHRT_MAX));
		formContent.AddChildTag(basicContent);

		HtmlTag relationDiv("div");
		relationDiv.AddChildTag(HtmlTagInputHidden("relationcounter", to_string(relations.size())));
		relationDiv.AddAttribute("id", "relation");
		priority_t priority = 1;
		for (auto relation : relations)
		{
			relationDiv.AddChildTag(HtmlTagRelation(to_string(relation->Priority()), relation->ObjectType2(), relation->ObjectID2(), relation->AccessoryState()));
			priority = relation->Priority() + 1;
		}
		relationDiv.AddChildTag(HtmlTag("div").AddAttribute("id", "new_priority_" + to_string(priority)));
		HtmlTag relationContent("div");
		relationContent.AddAttribute("id", "tab_relation");
		relationContent.AddClass("tab_content");
		relationContent.AddClass("hidden");
		relationContent.AddChildTag(relationDiv);
		HtmlTagButton newButton("New", "newrelation");
		newButton.AddAttribute("onclick", "addRelation();return false;");
		relationContent.AddChildTag(newButton);
		relationContent.AddChildTag(HtmlTag("br"));
		formContent.AddChildTag(relationContent);

		HtmlTag positionContent("div");
		positionContent.AddAttribute("id", "tab_position");
		positionContent.AddClass("tab_content");
		positionContent.AddClass("hidden");
		positionContent.AddChildTag(HtmlTagPosition(posx, posy, posz, visible));
		formContent.AddChildTag(positionContent);

		HtmlTag automodeContent("div");
		automodeContent.AddAttribute("id", "tab_automode");
		automodeContent.AddClass("tab_content");
		automodeContent.AddClass("hidden");

		HtmlTagInputCheckboxWithLabel checkboxAutomode("automode", "Auto-mode:", "automode", static_cast<bool>(automode));
		checkboxAutomode.AddAttribute("id", "automode");
		checkboxAutomode.AddAttribute("onchange", "onChangeCheckboxShowHide('automode', 'tracks');");
		automodeContent.AddChildTag(checkboxAutomode);

		HtmlTag tracksDiv("div");
		tracksDiv.AddAttribute("id", "tracks");
		if (automode == AutomodeNo)
		{
			tracksDiv.AddAttribute("hidden");
		}
		tracksDiv.AddChildTag(HtmlTagSelectTrack("from", "From track:", fromTrack, fromDirection));
		tracksDiv.AddChildTag(HtmlTagSelectTrack("to", "To track:", toTrack, toDirection, "updateFeedbacksOfTrack(); return false;"));
		HtmlTag feedbackDiv("div");
		feedbackDiv.AddAttribute("id", "feedbacks");
		feedbackDiv.AddChildTag(HtmlTagSelectFeedbacksOfTrack(toTrack, feedbackIdReduced, feedbackIdCreep, feedbackIdStop, feedbackIdOver));
		tracksDiv.AddChildTag(feedbackDiv);
		map<string,string> commuterOptions;
		commuterOptions[to_string(Street::CommuterTypeNo)] = "Only non-commuter";
		commuterOptions[to_string(Street::CommuterTypeBoth)] = "Commuter and non-commuter";
		commuterOptions[to_string(Street::CommuterTypeOnly)] = "Only commuter";
		tracksDiv.AddChildTag(HtmlTagSelectWithLabel("commuter", "Allow trains:", commuterOptions, to_string(commuter)));
		tracksDiv.AddChildTag(HtmlTagInputIntegerWithLabel("mintrainlength", "Min. train length:", minTrainLength, 0, 99999));
		tracksDiv.AddChildTag(HtmlTagInputIntegerWithLabel("maxtrainlength", "Max. train length:", maxTrainLength, 0, 99999));
		automodeContent.AddChildTag(tracksDiv);
		formContent.AddChildTag(automodeContent);

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(formContent));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleFeedbacksOfTrack(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		HtmlReplyWithHeader(HtmlTagSelectFeedbacksOfTrack(trackID, FeedbackNone, FeedbackNone, FeedbackNone, FeedbackNone));
	}

	void WebClient::handleStreetSave(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street", StreetNone);
		string name = GetStringMapEntry(arguments, "name");
		delay_t delay = static_cast<delay_t>(GetIntegerMapEntry(arguments, "delay"));
		Street::commuterType_t commuter = static_cast<Street::commuterType_t>(GetIntegerMapEntry(arguments, "commuter", Street::CommuterTypeBoth));
		length_t mintrainlength = static_cast<length_t>(GetIntegerMapEntry(arguments, "mintrainlength", 0));
		length_t maxtrainlength = static_cast<length_t>(GetIntegerMapEntry(arguments, "maxtrainlength", 0));
		visible_t visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible"));
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		automode_t automode = static_cast<automode_t>(GetBoolMapEntry(arguments, "automode"));
		trackID_t fromTrack = GetIntegerMapEntry(arguments, "fromtrack", TrackNone);
		direction_t fromDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "fromdirection", DirectionRight));
		trackID_t toTrack = GetIntegerMapEntry(arguments, "totrack", TrackNone);
		direction_t toDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "todirection", DirectionLeft));
		feedbackID_t feedbackIdReduced = GetIntegerMapEntry(arguments, "feedbackreduced", FeedbackNone);
		feedbackID_t feedbackIdCreep = GetIntegerMapEntry(arguments, "feedbackcreep", FeedbackNone);
		feedbackID_t feedbackIdStop = GetIntegerMapEntry(arguments, "feedbackstop", FeedbackNone);
		feedbackID_t feedbackIdOver = GetIntegerMapEntry(arguments, "feedbackover", FeedbackNone);

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
			relations.push_back(new Relation(&manager, ObjectTypeStreet, streetID, objectType, switchId, priority, state, LockStateFree));
			++priority;
		}

		string result;
		if (!manager.StreetSave(streetID,
			name,
			delay,
			commuter,
			mintrainlength,
			maxtrainlength,
			relations,
			visible,
			posx,
			posy,
			posz,
			automode,
			fromTrack,
			fromDirection,
			toTrack,
			toDirection,
			feedbackIdReduced,
			feedbackIdCreep,
			feedbackIdStop,
			feedbackIdOver,
			result))
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

		const datamodel::Street* street = manager.GetStreet(streetID);
		if (street == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown street");
			return;
		}

		HtmlTag content;
		const string& streetName = street->GetName();
		content.AddContent(HtmlTag("h1").AddContent("Delete street &quot;" + streetName + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the street &quot;" + streetName + "&quot;?"));
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
		const datamodel::Street* street = manager.GetStreet(streetID);
		if (street == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete street");
			return;
		}

		string name = street->GetName();

		if (!manager.StreetDelete(streetID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete street");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Street &quot;" + name + "&quot; deleted.");
	}

	void WebClient::handleStreetList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Streets"));
		HtmlTag table("table");
		const map<string,datamodel::Street*> streetList = manager.StreetListByName();
		map<string,string> streetArgument;
		for (auto street : streetList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(street.first));
			string streetIdString = to_string(street.second->GetID());
			streetArgument["street"] = streetIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "streetedit_list_" + streetIdString, streetArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "streetaskdelete_" + streetIdString, streetArgument)));
			if (street.second->IsInUse())
			{
				row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonCommand("Release", "streetrelease_" + streetIdString, streetArgument)));
			}
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
		manager.ExecuteStreetAsync(streetID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent("Street executed"));
	}

	void WebClient::handleStreetRelease(const map<string, string>& arguments)
	{
		streetID_t streetID = GetIntegerMapEntry(arguments, "street");
		bool ret = manager.StreetRelease(streetID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Street released" : "Street not released"));
	}

	void WebClient::handleTrackEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		string name("New Track");
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", 0);
		layoutItemSize_t height = GetIntegerMapEntry(arguments, "length", 1);
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		trackType_t type = TrackTypeStraight;
		std::vector<feedbackID_t> feedbacks;
		datamodel::Track::selectStreetApproach_t selectStreetApproach = static_cast<datamodel::Track::selectStreetApproach_t>(GetIntegerMapEntry(arguments, "selectstreetapproach", datamodel::Track::SelectStreetSystemDefault));
		if (trackID > TrackNone)
		{
			const datamodel::Track* track = manager.GetTrack(trackID);
			name = track->GetName();
			posx = track->GetPosX();
			posy = track->GetPosY();
			posz = track->GetPosZ();
			height = track->GetHeight();
			rotation = track->GetRotation();
			type = track->GetType();
			feedbacks = track->GetFeedbacks();
			selectStreetApproach = track->GetSelectStreetApproach();
		}
		if (type == TrackTypeTurn)
		{
			height = 1;
		}

		std::map<string, string> typeOptions;
		typeOptions[to_string(static_cast<int>(TrackTypeStraight))] = "Straight";
		typeOptions[to_string(static_cast<int>(TrackTypeTurn))] = "Turn";

		content.AddChildTag(HtmlTag("h1").AddContent("Edit track &quot;" + name + "&quot;"));
		HtmlTag tabMenu("div");
		tabMenu.AddChildTag(HtmlTagTabMenuItem("main", "Main", true));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("position", "Position"));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("feedback", "Feedbacks"));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("automode", "Automode"));
		content.AddChildTag(tabMenu);

		HtmlTag formContent("form");
		formContent.AddAttribute("id", "editform");
		formContent.AddChildTag(HtmlTagInputHidden("cmd", "tracksave"));
		formContent.AddChildTag(HtmlTagInputHidden("track", to_string(trackID)));

		HtmlTag mainContent("div");
		mainContent.AddAttribute("id", "tab_main");
		mainContent.AddClass("tab_content");
		mainContent.AddChildTag(HtmlTagInputTextWithLabel("name", "Track Name:", name));
		mainContent.AddChildTag(HtmlTagSelectWithLabel("type", "Type:", typeOptions, to_string(type)).AddAttribute("onchange", "onChangeTrackType();return false;"));
		HtmlTag i_length("div");
		i_length.AddAttribute("id", "i_length");
		i_length.AddChildTag(HtmlTagInputIntegerWithLabel("length", "Length:", height, 1, 100));
		if (type == TrackTypeTurn)
		{
			i_length.AddAttribute("hidden");
		}
		mainContent.AddChildTag(i_length);
		formContent.AddChildTag(mainContent);

		HtmlTag positionContent("div");
		positionContent.AddAttribute("id", "tab_position");
		positionContent.AddClass("tab_content");
		positionContent.AddClass("hidden");
		positionContent.AddChildTag(HtmlTagPosition(posx, posy, posz));
		positionContent.AddChildTag(HtmlTagRotation(rotation));
		formContent.AddChildTag(positionContent);

		HtmlTag automodeContent("div");
		automodeContent.AddAttribute("id", "tab_automode");
		automodeContent.AddClass("tab_content");
		automodeContent.AddClass("hidden");
		automodeContent.AddChildTag(HtmlTagSelectSelectStreetApproach(selectStreetApproach, true));
		formContent.AddChildTag(automodeContent);

		unsigned int feedbackCounter = 0;
		HtmlTag existingFeedbacks("div");
		existingFeedbacks.AddAttribute("id", "feedbackcontent");
		for (auto feedbackID : feedbacks)
		{
			existingFeedbacks.AddChildTag(HtmlTagSelectFeedbackForTrack(++feedbackCounter, trackID, feedbackID));
		}
		existingFeedbacks.AddChildTag(HtmlTag("div").AddAttribute("id", "div_feedback_" + to_string(feedbackCounter + 1)));

		HtmlTag feedbackContent("div");
		feedbackContent.AddAttribute("id", "tab_feedback");
		feedbackContent.AddClass("tab_content");
		feedbackContent.AddClass("hidden");
		feedbackContent.AddChildTag(HtmlTagInputHidden("feedbackcounter", to_string(feedbackCounter)));
		feedbackContent.AddChildTag(existingFeedbacks);
		HtmlTagButton newButton("New", "newfeedback");
		newButton.AddAttribute("onclick", "addFeedback();return false;");
		feedbackContent.AddChildTag(newButton);
		feedbackContent.AddChildTag(HtmlTag("br"));
		formContent.AddChildTag(feedbackContent);

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(formContent));
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
		layoutItemSize_t height = 1;
		layoutRotation_t rotation = static_cast<layoutRotation_t>(GetIntegerMapEntry(arguments, "rotation", Rotation0));
		trackType_t type = static_cast<trackType_t>(GetBoolMapEntry(arguments, "type", TrackTypeStraight));
		if (type == TrackTypeStraight)
		{
			height = GetIntegerMapEntry(arguments, "length", 1);
		}
		vector<feedbackID_t> feedbacks;
		unsigned int feedbackCounter = GetIntegerMapEntry(arguments, "feedbackcounter", 1);
		for (unsigned int feedback = 1; feedback <= feedbackCounter; ++feedback)
		{
			feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback_" + to_string(feedback), FeedbackNone);
			if (feedbackID != FeedbackNone)
			{
				feedbacks.push_back(feedbackID);
			}
		}
		datamodel::Track::selectStreetApproach_t selectStreetApproach = static_cast<datamodel::Track::selectStreetApproach_t>(GetIntegerMapEntry(arguments, "selectstreetapproach", datamodel::Track::SelectStreetSystemDefault));
		string result;
		if (manager.TrackSave(trackID, name, posX, posY, posZ, height, rotation, type, feedbacks, selectStreetApproach, result) == TrackNone)
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

		const datamodel::Track* track = manager.GetTrack(trackID);
		if (track == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown track");
			return;
		}

		HtmlTag content;
		const string& trackName = track->GetName();
		content.AddContent(HtmlTag("h1").AddContent("Delete track &quot;" + trackName + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the track &quot;" + trackName + "&quot;?"));
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
		const map<string,datamodel::Track*> trackList = manager.TrackListByName();
		map<string,string> trackArgument;
		for (auto track : trackList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(track.first));
			string locoIdString = to_string(track.second->GetID());
			trackArgument["track"] = locoIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "trackedit_list_" + locoIdString, trackArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "trackaskdelete_" + locoIdString, trackArgument)));
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
		const datamodel::Track* track = manager.GetTrack(trackID);
		if (track == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete track");
			return;
		}

		string name = track->GetName();

		if (!manager.TrackDelete(trackID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete track");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Track &quot;" + name + "&quot; deleted.");
	}

	void WebClient::handleTrackGet(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track");
		const datamodel::Track* track = manager.GetTrack(trackID);
		HtmlReplyWithHeader(HtmlTagTrack(manager, track));
	}

	void WebClient::handleTrackSetLoco(const map<string, string>& arguments)
	{
		HtmlTag content;
		trackID_t trackID = GetIntegerMapEntry(arguments, "track", TrackNone);
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		if (locoID != LocoNone)
		{
			bool ok = manager.LocoIntoTrack(locoID, trackID);
			HtmlReplyWithHeaderAndParagraph(ok ? "Loco added to track." : "Unable to add loco to track.");
			return;
		}
		const datamodel::Track* track = manager.GetTrack(trackID);
		if (track->IsInUse())
		{
			HtmlReplyErrorWithHeader("Track " + track->GetName() + " is in use.");
			return;
		}
		map<string,locoID_t> locos = manager.LocoListFree();
		content.AddChildTag(HtmlTag("h1").AddContent("Select loco for track " + track->GetName()));
		content.AddChildTag(HtmlTagInputHidden("cmd", "tracksetloco"));
		content.AddChildTag(HtmlTagInputHidden("track", to_string(trackID)));
		content.AddChildTag(HtmlTagSelectWithLabel("loco", "Loco:", locos));
		content.AddChildTag(HtmlTag("br"));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(HtmlTag("form").AddAttribute("id", "editform").AddChildTag(content));
	}

	void WebClient::handleTrackRelease(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track");
		bool ret = manager.TrackReleaseWithLoco(trackID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Track released" : "Track not released"));
	}

	void WebClient::handleTrackStartLoco(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track");
		bool ret = manager.TrackStartLoco(trackID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Loco started" : "Loco not started"));
	}

	void WebClient::handleTrackStopLoco(const map<string, string>& arguments)
	{
		trackID_t trackID = GetIntegerMapEntry(arguments, "track");
		bool ret = manager.TrackStopLoco(trackID);
		HtmlReplyWithHeader(HtmlTag("p").AddContent(ret ? "Loco stopped" : "Loco not stopped"));
	}

	void WebClient::handleFeedbackEdit(const map<string, string>& arguments)
	{
		HtmlTag content;
		feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback", FeedbackNone);
		string name("New feedback");
		controlID_t controlId = ControlNone;
		feedbackPin_t pin = FeedbackPinNone;
		layoutPosition_t posx = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posy = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posz = GetIntegerMapEntry(arguments, "posz", LayerUndeletable);
		visible_t visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible", feedbackID == FeedbackNone && (posx || posy) ? VisibleYes : VisibleNo));
		bool inverted = false;
		if (feedbackID > FeedbackNone)
		{
			const datamodel::Feedback* feedback = manager.GetFeedback(feedbackID);
			name = feedback->GetName();
			controlId = feedback->GetControlID();
			pin = feedback->GetPin();
			inverted = feedback->IsInverted();
			visible = feedback->GetVisible();
			posx = feedback->GetPosX();
			posy = feedback->GetPosY();
			posz = feedback->GetPosZ();
		}

		std::map<controlID_t,string> controls = manager.FeedbackControlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls)
		{
			controlOptions[to_string(control.first)] = control.second;
			if (controlId == ControlIdNone)
			{
				controlId = control.first;
			}
		}

		content.AddChildTag(HtmlTag("h1").AddContent("Edit feedback &quot;" + name + "&quot;"));

		HtmlTag tabMenu("div");
		tabMenu.AddChildTag(HtmlTagTabMenuItem("main", "Main", true));
		tabMenu.AddChildTag(HtmlTagTabMenuItem("position", "Position"));
		content.AddChildTag(tabMenu);

		HtmlTag formContent("form");
		formContent.AddAttribute("id", "editform");
		formContent.AddChildTag(HtmlTagInputHidden("cmd", "feedbacksave"));
		formContent.AddChildTag(HtmlTagInputHidden("feedback", to_string(feedbackID)));

		HtmlTag mainContent("div");
		mainContent.AddAttribute("id", "tab_main");
		mainContent.AddClass("tab_content");
		mainContent.AddChildTag(HtmlTagInputTextWithLabel("name", "Feedback Name:", name));
		mainContent.AddChildTag(HtmlTagSelectWithLabel("control", "Control:", controlOptions, to_string(controlId)).AddAttribute("onchange", "loadProtocol('feedback', " + to_string(feedbackID) + ")"));
		mainContent.AddChildTag(HtmlTagInputIntegerWithLabel("pin", "Pin:", pin, 1, 4096));
		mainContent.AddChildTag(HtmlTagInputCheckboxWithLabel("inverted", "Inverted:", "true", inverted));
		formContent.AddChildTag(mainContent);

		HtmlTag positionContent("div");
		positionContent.AddAttribute("id", "tab_position");
		positionContent.AddClass("tab_content");
		positionContent.AddClass("hidden");
		positionContent.AddChildTag(HtmlTagPosition(posx, posy, posz, visible));
		formContent.AddChildTag(positionContent);

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(formContent));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleFeedbackSave(const map<string, string>& arguments)
	{
		feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback", FeedbackNone);
		string name = GetStringMapEntry(arguments, "name");
		controlID_t controlId = GetIntegerMapEntry(arguments, "control", ControlIdNone);
		feedbackPin_t pin = static_cast<feedbackPin_t>(GetIntegerMapEntry(arguments, "pin", FeedbackPinNone));
		bool inverted = GetBoolMapEntry(arguments, "inverted");
		visible_t visible = static_cast<visible_t>(GetBoolMapEntry(arguments, "visible", VisibleNo));
		layoutPosition_t posX = GetIntegerMapEntry(arguments, "posx", 0);
		layoutPosition_t posY = GetIntegerMapEntry(arguments, "posy", 0);
		layoutPosition_t posZ = GetIntegerMapEntry(arguments, "posz", 0);
		string result;
		if (manager.FeedbackSave(feedbackID, name, visible, posX, posY, posZ, controlId, pin, inverted, result) == FeedbackNone)
		{
			HtmlReplyWithHeaderAndParagraph(result);
			return;
		}
		HtmlReplyWithHeaderAndParagraph("Feedback &quot;" + name + "&quot; saved.");
	}

	void WebClient::handleFeedbackState(const map<string, string>& arguments)
	{
		feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback", FeedbackNone);
		feedbackState_t state = (GetStringMapEntry(arguments, "state", "occupied").compare("occupied") == 0 ? FeedbackStateOccupied : FeedbackStateFree);

		manager.FeedbackState(feedbackID, state);

		stringstream ss;
		ss << "Feedback &quot;" << manager.GetFeedbackName(feedbackID) << "&quot; is now set to " << state;
		HtmlReplyWithHeader(HtmlTag().AddContent(ss.str()));
	}

	void WebClient::handleFeedbackList(const map<string, string>& arguments)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Feedback"));
		HtmlTag table("table");
		const map<string,datamodel::Feedback*> feedbackList = manager.FeedbackListByName();
		map<string,string> feedbackArgument;
		for (auto feedback : feedbackList)
		{
			HtmlTag row("tr");
			row.AddChildTag(HtmlTag("td").AddContent(feedback.first));
			string feedbackIdString = to_string(feedback.second->GetID());
			feedbackArgument["feedback"] = feedbackIdString;
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Edit", "feedbackedit_list_" + feedbackIdString, feedbackArgument)));
			row.AddChildTag(HtmlTag("td").AddChildTag(HtmlTagButtonPopup("Delete", "feedbackaskdelete_" + feedbackIdString, feedbackArgument)));
			table.AddChildTag(row);
		}
		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(table));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonPopup("New", "feedbackedit_0"));
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleFeedbackAskDelete(const map<string, string>& arguments)
	{
		feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback", FeedbackNone);

		if (feedbackID == FeedbackNone)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown feedback");
			return;
		}

		const datamodel::Feedback* feedback = manager.GetFeedback(feedbackID);
		if (feedback == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unknown feedback");
			return;
		}

		HtmlTag content;
		const string& feedbackName = feedback->GetName();
		content.AddContent(HtmlTag("h1").AddContent("Delete feedback &quot;" + feedbackName + "&quot;?"));
		content.AddContent(HtmlTag("p").AddContent("Are you sure to delete the feedback &quot;" + feedbackName + "&quot;?"));
		content.AddContent(HtmlTag("form").AddAttribute("id", "editform")
			.AddContent(HtmlTagInputHidden("cmd", "feedbackdelete"))
			.AddContent(HtmlTagInputHidden("feedback", to_string(feedbackID))
			));
		content.AddContent(HtmlTagButtonCancel());
		content.AddContent(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleFeedbackDelete(const map<string, string>& arguments)
	{
		feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback", FeedbackNone);
		const datamodel::Feedback* feedback = manager.GetFeedback(feedbackID);
		if (feedback == nullptr)
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete feedback");
			return;
		}

		string name = feedback->GetName();

		if (!manager.FeedbackDelete(feedbackID))
		{
			HtmlReplyWithHeaderAndParagraph("Unable to delete feedback");
			return;
		}

		HtmlReplyWithHeaderAndParagraph("Feedback &quot;" + name + "&quot; deleted.");
	}

	void WebClient::handleFeedbackGet(const map<string, string>& arguments)
	{
		feedbackID_t feedbackID = GetIntegerMapEntry(arguments, "feedback", FeedbackNone);
		const datamodel::Feedback* feedback = manager.GetFeedback(feedbackID);
		if (feedback->GetVisible() == VisibleNo)
		{
			HtmlReplyWithHeader(HtmlTag());
			return;
		}
		HtmlReplyWithHeader(HtmlTagFeedback(feedback));
	}

	void WebClient::handleLocoSelector(const map<string, string>& arguments)
	{
		HtmlReplyWithHeader(HtmlTagLocoSelector());
	}

	void WebClient::handleLayerSelector(const map<string, string>& arguments)
	{
		HtmlReplyWithHeader(HtmlTagLayerSelector());
	}

	void WebClient::handleSettingsEdit(const map<string, string>& arguments)
	{
		const accessoryDuration_t defaultAccessoryDuration = manager.GetDefaultAccessoryDuration();
		const bool autoAddFeedback = manager.GetAutoAddFeedback();
		const datamodel::Track::selectStreetApproach_t selectStreetApproach = manager.GetSelectStreetApproach();

		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Edit settings"));

		HtmlTag formContent("form");
		formContent.AddAttribute("id", "editform");
		formContent.AddChildTag(HtmlTagInputHidden("cmd", "settingssave"));
		formContent.AddChildTag(HtmlTagDuration(defaultAccessoryDuration, "Default duration for accessory/switch (ms):"));
		formContent.AddChildTag(HtmlTagInputCheckboxWithLabel("autoaddfeedback", "Automatically add unknown feedbacks", "autoaddfeedback", autoAddFeedback));
		formContent.AddChildTag(HtmlTagSelectSelectStreetApproach(selectStreetApproach, false));

		content.AddChildTag(HtmlTag("div").AddClass("popup_content").AddChildTag(formContent));
		content.AddChildTag(HtmlTagButtonCancel());
		content.AddChildTag(HtmlTagButtonOK());
		HtmlReplyWithHeader(content);
	}

	void WebClient::handleSettingsSave(const map<string, string>& arguments)
	{
		const accessoryDuration_t defaultAccessoryDuration = GetIntegerMapEntry(arguments, "duration", manager.GetDefaultAccessoryDuration());
		const bool autoAddFeedback = GetBoolMapEntry(arguments, "autoaddfeedback", manager.GetAutoAddFeedback());
		const datamodel::Track::selectStreetApproach_t selectStreetApproach = static_cast<datamodel::Track::selectStreetApproach_t>(GetIntegerMapEntry(arguments, "selectstreetapproach", datamodel::Track::SelectStreetRandom));
		manager.SaveSettings(defaultAccessoryDuration, autoAddFeedback, selectStreetApproach);
		HtmlReplyWithHeaderAndParagraph("Settings saved.");
	}

	void WebClient::handleTimestamp(const map<string, string>& arguments)
	{
		const time_t timestamp = GetIntegerMapEntry(arguments, "timestamp", 0);
		if (timestamp == 0)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Timestamp not set"));
			return;
		}
		struct timeval tv;
		int ret = gettimeofday(&tv, nullptr);
		if (ret != 0 || tv.tv_sec > __UNIX_TIMESTAMP__)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Timestamp already set"));
			return;
		}

		tv.tv_sec = timestamp;
		ret = settimeofday(&tv, nullptr);
		if (ret != 0)
		{
			HtmlReplyWithHeader(HtmlTag("p").AddContent("Timestamp not set"));
			return;
		}
		HtmlReplyWithHeader(HtmlTag("p").AddContent("Timestamp set"));
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
			bool ok = server.NextUpdate(updateID, s);
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

	void WebClient::HtmlReplyErrorWithHeader(const string& errorText)
	{
		HtmlTag content;
		content.AddChildTag(HtmlTag("h1").AddContent("Error"));
		content.AddChildTag(HtmlTag("p").AddContent(errorText));
		content.AddChildTag(HtmlTagButtonCancel());
		HtmlReplyWithHeader(content);
	}

	void WebClient::HtmlReplyWithHeader(const HtmlTag& tag)
	{
		connection->Send(HtmlResponse(tag));
	}

	HtmlTag WebClient::HtmlTagLocoSelector() const
	{
		const map<locoID_t, Loco*>& locos = manager.locoList();
		map<string,locoID_t> options;
		for (auto locoTMP : locos)
		{
			Loco* loco = locoTMP.second;
			options[loco->GetName()] = loco->GetID();
		}
		return HtmlTagSelect("loco", options).AddAttribute("onchange", "loadLoco();");
	}

	void WebClient::printLoco(const map<string, string>& arguments)
	{
		string content;
		locoID_t locoID = GetIntegerMapEntry(arguments, "loco", LocoNone);
		if (locoID > LocoNone)
		{
			stringstream ss;
			Loco* loco = manager.GetLoco(locoID);
			ss << HtmlTag("p").AddContent(loco->GetName());
			unsigned int speed = loco->Speed();
			map<string,string> buttonArguments;
			buttonArguments["loco"] = to_string(locoID);

			string id = "locospeed_" + to_string(locoID);
			ss << HtmlTagInputSliderLocoSpeed("speed", "locospeed", MinSpeed, loco->GetMaxSpeed(), speed, locoID);
			buttonArguments["speed"] = to_string(MinSpeed);
			ss << HtmlTagButtonCommand("0", id + "_0", buttonArguments);
			buttonArguments["speed"] = to_string(loco->GetCreepSpeed());
			ss << HtmlTagButtonCommand("I", id + "_1", buttonArguments);
			buttonArguments["speed"] = to_string(loco->GetReducedSpeed());
			ss << HtmlTagButtonCommand("II", id + "_2", buttonArguments);
			buttonArguments["speed"] = to_string(loco->GetTravelSpeed());
			ss << HtmlTagButtonCommand("III", id + "_3", buttonArguments);
			buttonArguments["speed"] = to_string(loco->GetMaxSpeed());
			ss << HtmlTagButtonCommand("IV", id + "_4", buttonArguments);
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
		HtmlTag menuMain("div");
		menuMain.AddClass("menu_main");
		menuMain.AddChildTag(HtmlTagButtonCommand("<svg width=\"35\" height=\"35\"><polygon points=\"15,0.5 30,0.5 30,24.5 15,24.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"20,10.5 30,0.5 30,24.5 20,34.5\" fill=\"black\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"0,10 7.5,10 7.5,5 15,12.5 7.5,20 7.5,15 0,15\"/></svg>", "quit"));
		menuMain.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuMain.AddChildTag(HtmlTagButtonCommandToggle("<svg width=\"35\" height=\"35\"><polyline points=\"12.5,8.8 11.1,9.8 9.8,11.1 8.8,12.5 8.1,14.1 7.7,15.8 7.5,17.5 7.7,19.2 8.1,20.9 8.8,22.5 9.8,23.9 11.1,25.2 12.5,26.2 14.1,26.9 15.8,27.3 17.5,27.5 19.2,27.3 20.9,26.9 22.5,26.2 23.9,25.2 25.2,23.9 26.2,22.5 26.9,20.9 27.3,19.2 27.5,17.5 27.3,15.8 26.9,14.1 26.2,12.5 25.2,11.1 23.9,9.8 22.5,8.8\" stroke=\"black\" stroke-width=\"3\" fill=\"none\"/><polyline points=\"17.5,2.5 17.5,15\" stroke=\"black\" stroke-width=\"3\" fill=\"none\"/></svg>", "booster", manager.Booster(), buttonArguments).AddClass("button_booster"));
		menuMain.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuMain.AddChildTag(HtmlTagButtonCommand("<svg width=\"35\" height=\"35\"><polyline points=\"1,11 1,10 10,1 25,1 34,10 34,25 25,34 10,34 1,25 1,11\" stroke=\"black\" stroke-width=\"1\" fill=\"red\"/><text x=\"3\" y=\"21\" fill=\"white\" font-size=\"11\">STOP</text></svg>", "stopall"));
		menu.AddChildTag(menuMain);

		HtmlTag menuAdd("div");
		menuAdd.AddClass("menu_add");
		menuAdd.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"36\" height=\"36\"><circle r=\"7\" cx=\"14\" cy=\"14\" fill=\"black\" /><line x1=\"14\" y1=\"5\" x2=\"14\" y2=\"23\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"9.5\" y1=\"6.2\" x2=\"18.5\" y2=\"21.8\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"6.2\" y1=\"9.5\" x2=\"21.8\" y2=\"18.5\" stroke-width=\"2\" stroke=\"black\" /><line y1=\"14\" x1=\"5\" y2=\"14\" x2=\"23\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"9.5\" y1=\"21.8\" x2=\"18.5\" y2=\"6.2\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"6.2\" y1=\"18.5\" x2=\"21.8\" y2=\"9.5\" stroke-width=\"2\" stroke=\"black\" /><circle r=\"5\" cx=\"14\" cy=\"14\" fill=\"white\" /><circle r=\"4\" cx=\"24\" cy=\"24\" fill=\"black\" /><line x1=\"18\" y1=\"24\" x2=\"30\" y2=\"24\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"28.2\" y1=\"28.2\" x2=\"19.8\" y2=\"19.8\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"24\" y1=\"18\" x2=\"24\" y2=\"30\" stroke-width=\"2\" stroke=\"black\" /><line x1=\"19.8\" y1=\"28.2\" x2=\"28.2\" y2=\"19.8\" stroke-width=\"2\" stroke=\"black\" /><circle r=\"2\" cx=\"24\" cy=\"24\" fill=\"white\" /></svg>", "settingsedit"));
		menuAdd.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"10,0.5 25,0.5 25,34.5 10,34.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><polygon points=\"13,3.5 22,3.5 22,7.5 13,7.5\" fill=\"white\" style=\"stroke:black;stroke-width:1;\"/><circle cx=\"14.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"11\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"14\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"17\" r=\"1\" fill=\"black\"/><circle cx=\"14.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"20.5\" cy=\"20\" r=\"1\" fill=\"black\"/><circle cx=\"17.5\" cy=\"27.5\" r=\"5\" fill=\"black\"/></svg>", "controllist"));
		menuAdd.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"0,10 5,10 5,0 10,0 10,10 25,10 25,0 35,0 35,5 30,5 30,10 35,10 35,25 0,25\" fill=\"black\"/><circle cx=\"5\" cy=\"30\" r=\"5\" fill=\"black\"/><circle cx=\"17.5\" cy=\"30\" r=\"5\" fill=\"black\"/><circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"black\"/</svg>", "locolist"));
		menuAdd.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polygon points=\"1,30 25,30 34,20 10,20\" fill=\"white\" stroke=\"black\"/><polygon points=\"1,25 25,25 34,15 10,15\" fill=\"white\" stroke=\"black\"/><polygon points=\"1,20 25,20 34,10 10,10\" fill=\"white\" stroke=\"black\"/><polygon points=\"1,15 25,15 34,5 10,5\" fill=\"white\" stroke=\"black\"/></svg>", "layerlist"));
		menuAdd.AddChildTag(HtmlTag().AddContent("&nbsp;&nbsp;&nbsp;"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"1,12 34,12\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"1,23 34,23\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"3,10 3,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"6,10 6,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"9,10 9,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"12,10 12,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"15,10 15,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"18,10 18,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"21,10 21,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"24,10 24,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"27,10 27,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"30,10 30,25\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"33,10 33,25\" stroke=\"black\" stroke-width=\"1\"/></svg>", "tracklist"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"1,20 7.1,19.5 13,17.9 18.5,15.3 23.5,11.8 27.8,7.5\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"1,28 8.5,27.3 15.7,25.4 22.5,22.2 28.6,17.9 33.9,12.6\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"1,20 34,20\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"1,28 34,28\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"3,18 3,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"6,18 6,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"9,17 9,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"12,16 12,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"15,15 15,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"18,13 18,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"21,12 21,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"24,9 24,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"27,17 27,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"30,18 30,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"33,18 33,30\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"24,9 32,17\" stroke=\"black\" stroke-width=\"1\"/><polyline points=\"26,7 34,15\" stroke=\"black\" stroke-width=\"1\"/></svg>", "switchlist"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"1,20 10,20 30,15\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"28,17 28,20 34,20\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/></svg>", "accessorylist"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"35\" height=\"35\"><polyline points=\"5,34 15,1\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"30,34 20,1\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,34 17.5,30\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,24 17.5,20\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,14 17.5,10\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/><polyline points=\"17.5,4 17.5,1\" stroke=\"black\" stroke-width=\"1\" fill=\"none\"/></svg>", "streetlist"));
		menuAdd.AddChildTag(HtmlTagButtonPopup("<svg width=\"36\" height=\"36\"><polyline points=\"1,25 35,25\" fill=\"none\" stroke=\"black\"/><polygon points=\"4,25 4,23 8,23 8,25\" fill=\"black\" stroke=\"black\"/><polygon points=\"35,22 16,22 15,19 18,10 35,10\" stroke=\"black\" fill=\"black\"/><polygon points=\"20,12 25,12 25,15 19,15\" fill=\"white\"/><polyline points=\"26,10 30,8 26,6\" stroke=\"black\" fill=\"none\"/><circle cx=\"22\" cy=\"22\" r=\"3\"/><circle cx=\"30\" cy=\"22\" r=\"3\"/></svg>", "feedbacklist"));
		menu.AddChildTag(menuAdd);
		body.AddChildTag(menu);

		body.AddChildTag(HtmlTag("div").AddClass("loco_selector").AddAttribute("id", "loco_selector").AddChildTag(HtmlTagLocoSelector()));
		body.AddChildTag(HtmlTag("div").AddClass("layer_selector").AddAttribute("id", "layer_selector").AddChildTag(HtmlTagLayerSelector()));
		body.AddChildTag(HtmlTag("div").AddClass("loco").AddAttribute("id", "loco"));
		body.AddChildTag(HtmlTag("div").AddClass("layout").AddAttribute("id", "layout").AddAttribute("oncontextmenu", "return loadLayoutContext(event);"));
		body.AddChildTag(HtmlTag("div").AddClass("popup").AddAttribute("id", "popup"));
		body.AddChildTag(HtmlTag("div").AddClass("status").AddAttribute("id", "status"));

		body.AddChildTag(HtmlTag("div").AddClass("contextmenu").AddAttribute("id", "layout_context")
			.AddChildTag(HtmlTag("ul").AddClass("contextentries")
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add track").AddAttribute("onClick", "loadPopup('/?cmd=trackedit&track=0');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add switch").AddAttribute("onClick", "loadPopup('/?cmd=switchedit&switch=0');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add accessory").AddAttribute("onClick", "loadPopup('/?cmd=accessoryedit&accessory=0');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add street").AddAttribute("onClick", "loadPopup('/?cmd=streetedit&street=0');"))
			.AddChildTag(HtmlTag("li").AddClass("contextentry").AddContent("Add feedback").AddAttribute("onClick", "loadPopup('/?cmd=feedbackedit&feedback=0');"))
			));

		connection->Send(HtmlFullResponse("Railcontrol", body));
	}
}; // namespace webserver
