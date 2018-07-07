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
#include "webclient.h"
#include "webserver.h"

using std::map;
using std::stoi;
using std::string;
using std::stringstream;
using std::thread;
using std::to_string;
using std::vector;
using datamodel::Loco;

namespace webserver {

	WebClient::WebClient(const unsigned int id, int socket, WebServer& webserver, Manager& m) :
		id(id),
		clientSocket(socket),
		run(false),
		server(webserver),
		clientThread(thread([this] {worker();})),
		manager(m),
		buttonID(0) {
	}

	WebClient::~WebClient() {
		// inform thread to stop
		run = false;
		// join thread
		clientThread.join();
	}

	// worker is the thread that handles client requests
	void WebClient::worker() {
		xlog("Executing webclient");
		run = true;

		char buffer_in[1024];
		memset(buffer_in, 0, sizeof(buffer_in));

		size_t pos = 0;
		string s;
		while(pos < sizeof(buffer_in) - 1 && s.find("\n\n") == string::npos) {
			pos += recv_timeout(clientSocket, buffer_in + pos, sizeof(buffer_in) - 1 - pos, 0);
			s = string(buffer_in);
			str_replace(s, string("\r\n"), string("\n"));
			str_replace(s, string("\r"), string("\n"));
		}
		vector<string> lines;
		str_split(s, string("\n"), lines);

		if (lines.size() < 1) {
			xlog("Invalid request");
			close(clientSocket);
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
		if ((method.compare("GET") != 0) && (method.compare("HEAD") != 0)) {
			xlog("Method %s not implemented", method.c_str());
			const char* reply =
				"HTTP/1.0 501 Not implemented\r\n\r\n"
				"<!DOCTYPE html><html><head><title>501 Not implemented</title></head><body><p>Method not implemented</p></body></html>";
			send_timeout(clientSocket, reply, strlen(reply), 0);
			close(clientSocket);
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
		if (arguments["cmd"].compare("quit") == 0) {
			simpleReply("Stopping Railcontrol");
			manager.booster(ControlTypeWebserver, BoosterStop);
			stopRailControlWebserver();
		}
		else if (arguments["cmd"].compare("on") == 0) {
			simpleReply("Turning booster on");
			manager.booster(ControlTypeWebserver, BoosterGo);
		}
		else if (arguments["cmd"].compare("off") == 0) {
			simpleReply("Turning booster off");
			manager.booster(ControlTypeWebserver, BoosterStop);
		}
		else if (arguments["cmd"].compare("loco") == 0) {
			printLoco(arguments);
		}
		else if (arguments["cmd"].compare("locospeed") == 0) {
			handleLocoSpeed(arguments);
		}
		else if (arguments["cmd"].compare("locodirection") == 0) {
			handleLocoDirection(arguments);
		}
		else if (arguments["cmd"].compare("locofunction") == 0) {
			handleLocoFunction(arguments);
		}
		else if (arguments["cmd"].compare("locoedit") == 0) {
			handleLocoEdit(arguments);
		}
		else if (arguments["cmd"].compare("locosave") == 0) {
			handleLocoSave(arguments);
		}
		else if (arguments["cmd"].compare("protocol") == 0) {
			handleProtocol(arguments);
		}
		else if (arguments["cmd"].compare("updater") == 0) {
			handleUpdater(headers);
		}
		else if (uri.compare("/") == 0) {
			printMainHTML();
		}
		else {
			deliverFile(uri);
		}

		xlog("Terminating webclient");
		close(clientSocket);
	}

	int WebClient::stop() {
		// inform thread to stop
		run = false;
		return 0;
	}

	void WebClient::interpretClientRequest(const vector<string>& lines, string& method, string& uri, string& protocol, map<string,string>& arguments, map<string,string>& headers) {
		if (lines.size()) {
			for (auto line : lines) {
				if (line.find("HTTP/1.") != string::npos) {
					vector<string> list;
					str_split(line, string(" "), list);
					if (list.size() == 3) {
						method = list[0];
						// transform method to uppercase
						std::transform(method.begin(), method.end(), method.begin(), ::toupper);
						// if method == HEAD set membervariable
						headOnly = false;
						if (method.compare("HEAD") == 0) {
							headOnly = true;
						}
						// set uri and protocol
						uri = list[1];
						protocol = list[2];
						// read GET-arguments from uri
						vector<string> uri_parts;
						str_split(uri, "?", uri_parts);
						if (uri_parts.size() == 2) {
							vector<string> argumentStrings;
							str_split(uri_parts[1], "&", argumentStrings);
							for (auto argument : argumentStrings) {
								vector<string> argumentParts;
								str_split(argument, "=", argumentParts);
								string argumentValue = argumentParts[1];
								// decode %20 and similar
								while (true) {
									size_t pos = argumentValue.find('%');
									if (pos == string::npos || pos + 3 > argumentValue.length()) {
										break;
									}
									char c1 = argumentValue[pos + 1];
									char c2 = argumentValue[pos + 2];
									if (c1 >= 'a') {
										c1 -= 'a' - 10;
									}
									else if (c1 >= 'A') {
										c1 -= 'A' - 10;
									}
									else if (c1 >= '0') {
										c1 -= '0';
									}
									if (c2 >= 'a') {
										c2 -= 'a' - 10;
									}
									else if (c2 >= 'A') {
										c2 -= 'A' - 10;
									}
									else if (c2 >= '0') {
										c2 -= '0';
									}
									char c = c1 * 16 + c2;
									argumentValue.replace(pos, 3, 1, c);
								}
								arguments[argumentParts[0]] = argumentValue;
							}
						}
					}
				}
				else {
					vector<string> list;
					str_split(line, string(": "), list);
					if (list.size() == 2) {
						headers[list[0]] = list[1];
					}
				}
			}
		}
	}

	void WebClient::deliverFile(const string& virtualFile) {
		stringstream ss;
		char workingDir[128];
		int rc;
		if (getcwd(workingDir, sizeof(workingDir))) {
			ss << workingDir << "/html" << virtualFile;
		}
		string sFile = ss.str();
		const char* realFile = sFile.c_str();
		xlog(realFile);
		FILE* f = fopen(realFile, "r");
		if (f) {
			struct stat s;
			rc = stat(realFile, &s);
			if (rc == 0) {
				size_t length = virtualFile.length();
				const char* contentType = NULL;
				if (length > 3 && virtualFile[length - 3] == '.' && virtualFile[length - 2] == 'j' && virtualFile[length - 1] == 's') {
					contentType = "application/javascript";
				}
				else if (length > 4 && virtualFile[length - 4] == '.') {
					if (virtualFile[length - 3] == 'i' && virtualFile[length - 2] == 'c' && virtualFile[length - 1] == 'o') {
						contentType = "image/x-icon";
					}
					else if (virtualFile[length - 3] == 'c' && virtualFile[length - 2] == 's' && virtualFile[length - 1] == 's') {
						contentType = "text/css";
					}
				}
				char header[1024];
				snprintf(header, sizeof(header),
					"HTTP/1.0 200 OK\r\n"
					"Cache-Control: max-age=3600"
					"Content-Lenth: %lu\r\n"
					"Content-Type: %s\r\n\r\n",
					s.st_size, contentType);
				send_timeout(clientSocket, header, strlen(header), 0);
				if (headOnly == false) {
					char* buffer = static_cast<char*>(malloc(s.st_size));
					if (buffer) {
						size_t r = fread(buffer, 1, s.st_size, f);
						send_timeout(clientSocket, buffer, r, 0);
						free(buffer);
						fclose(f);
						return;
					}
				}
			}
			fclose(f);
		}
		char reply[1024];
		snprintf(reply, sizeof(reply),
			"HTTP/1.0 404 Not found\r\n\r\n"
			"<!DOCTYPE html><html><head><title>404 Not found</title></head><body><p>File %s not found</p></body></html>",
			virtualFile.c_str());
		send_timeout(clientSocket, reply, strlen(reply), 0);
	}

	void WebClient::handleLocoSpeed(const map<string, string>& arguments) {
		locoID_t locoID = 0;
		speed_t speed = 0;
		if (arguments.count("loco")) locoID = stoi(arguments.at("loco"));
		if (arguments.count("speed")) speed = stoi(arguments.at("speed"));

		manager.locoSpeed(ControlTypeWebserver, locoID, speed);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; is now set to speed " << speed;
		string sOut = ss.str();
		simpleReply(sOut);
	}

	void WebClient::handleLocoDirection(const map<string, string>& arguments) {
		locoID_t locoID = 0;
		direction_t direction = 0;
		if (arguments.count("loco")) locoID = stoi(arguments.at("loco"));
		if (arguments.count("direction")) direction = (arguments.at("direction").compare("forward") == 0 ? 1 : 0);

		manager.locoDirection(ControlTypeWebserver, locoID, direction);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; is now set to " << direction;
		string sOut = ss.str();
		simpleReply(sOut);
	}

	void WebClient::handleLocoFunction(const map<string, string>& arguments) {
		locoID_t locoID = 0;
		function_t function = 0;
		bool on = false;
		if (arguments.count("loco")) locoID = stoi(arguments.at("loco"));
		if (arguments.count("function")) function = stoi(arguments.at("function"));
		if (arguments.count("on")) on = stoi(arguments.at("on"));

		manager.locoFunction(ControlTypeWebserver, locoID, function, on);

		stringstream ss;
		ss << "Loco &quot;" << manager.getLocoName(locoID) << "&quot; has now set f";
		ss << function << " to " << (on ? "on" : "off");
		string sOut = ss.str();
		simpleReply(sOut);
	}

	template <typename T> string inputText(string label, string name, T value) {
		stringstream ss;
		// FIXME: label
		ss << "<label>" << label << "</label><input type=\"text\" name=\"" << name << "\" value=\"" << value << "\"><br>";
		return ss.str();
	}

	template <typename T> string inputHidden(string name, T value) {
		stringstream ss;
		ss << "<input type=\"hidden\" name=\"" << name << "\" value=\"" << value << "\">";
		return ss.str();
	}

	void WebClient::handleLocoEdit(const map<string, string>& arguments) {
		stringstream ss;
		locoID_t locoID = 0;
		controlID_t controlID = 0;
		protocol_t protocol = 0;
		address_t address = 0;
		string name("New Loco");
		if (arguments.count("loco")) {
			locoID = stoi(arguments.at("loco"));
			const datamodel::Loco* loco = manager.getLoco(locoID);
			controlID = loco->controlID;
			protocol = loco->protocol;
			address = loco->address;
			name = loco->name;
		}
		ss << "<h1>Edit loco &quot;";
		ss << name;
		ss << "&quot;</h1>";
		ss << "<form id=\"editform\">";
		ss << inputHidden("cmd", "locosave");
		ss << inputHidden("loco", locoID);
		ss << inputText("Loco name:", "name", name);

		std::map<controlID_t,string> controls = manager.controlListNames();
		std::map<string, string> controlOptions;
		for(auto control : controls) {
			controlOptions[to_string(control.first)] = control.second;
		}
		ss << "<label>Control:</label>";
		ss << select("control", controlOptions, to_string(controlID));

		std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
		std::map<string, string> protocolOptions;
		for(auto protocol : protocols) {
			protocolOptions[to_string(protocol.first)] = protocol.second;
		}
		ss << "<label>Protocol:</label>";
		ss << "<div id=\"protocol\">";
		ss << select("protocol", protocolOptions, to_string(protocol));
		ss << "</div>";
		ss << inputText("Address:", "address", address);
		ss << "</form>";
		ss << buttonPopupCancel();
		ss << buttonPopupOK();
		ss << "<script>\n";
		ss << "//# sourceURL=handleLocoEdit.js";
		ss << "</script>\n";
		string sOut = ss.str();
		simpleReply(sOut);
	}

	void WebClient::handleProtocol(const map<string, string>& arguments) {
		stringstream ss;
		if (arguments.count("control")) {
			controlID_t controlID = stoi(arguments.at("control"));
			protocol_t selectedProtocol = ProtocolNone;
			if (arguments.count("protocol")) {
				selectedProtocol = stoi(arguments.at("protocol"));
			}
			std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
			ss << "<label>Protocol:</label><select name=\"protocol\">";
			for (auto protocol : protocols) {
				ss << "<option value=\"" << (unsigned int)protocol.first << "\"" << (protocol.first == selectedProtocol ? " selected" : "") << ">" << protocol.second << "</option>";
			}
			ss << "</select>";
		}
		else {
			ss << "Unknown control";
		}
		string sOut = ss.str();
		simpleReply(sOut);
	}

	void WebClient::handleLocoSave(const map<string, string>& arguments) {
		stringstream ss;
		locoID_t locoID;
		if (arguments.count("loco")) {
			locoID = stoi(arguments.at("loco"));
			string name;
			controlID_t controlID = 0;
			protocol_t protocol = 0;
			address_t address = 0;
			if (arguments.count("name")) name = arguments.at("name");
			if (arguments.count("control")) controlID = stoi(arguments.at("control"));
			if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
			if (arguments.count("address")) address = stoi(arguments.at("address"));
			string result;
			if (!manager.locoSave(locoID, name, controlID, protocol, address, result)) {
				ss << "<p>" << result << "</p>";
			}
			else {
				ss << "<p>Loco &quot;" << locoID << "&quot; saved.</p>";
			}
		}
		else {
			ss << "<p>Unable to save loco.</p>";
		}

		string sOut = ss.str();
		simpleReply(sOut);
	}

	void WebClient::handleUpdater(const map<string, string>& headers) {
		char reply[1024];
		int ret = snprintf(reply, sizeof(reply),
			"HTTP/1.0 200 OK\r\n"
			"Cache-Control: no-cache, must-revalidate\r\n"
			"Pragma: no-cache\r\n"
			"Expires: Sun, 12 Feb 2016 00:00:00 GMT\r\n"
			"Content-Type: text/event-stream; charset=utf-8\r\n\r\n");
		send_timeout(clientSocket, reply, ret, 0);

		unsigned int updateID = 0;
		if (headers.count("Last-Event-ID") == 1) {
			updateID = stoi(headers.at("Last-Event-ID"));
		}
		while(run) {
			string s;
			if (server.nextUpdate(updateID, s)) {
				ret = snprintf(reply, sizeof(reply),
						"id: %i\r\n%s\r\n\r\n", updateID, s.c_str());
				ret = send_timeout(clientSocket, reply, ret, 0);
				++updateID;
				if (ret < 0) {
					return;
				}
			}
			else {
				// FIXME: use conditional variables instead of sleep
				usleep(100000);
			}
		}
	}

	void WebClient::simpleReply(const string& text, const string& code) {
		size_t contentLength = text.length();
		char reply[256 + contentLength];
		snprintf(reply, sizeof(reply),
			"HTTP/1.0 %s\r\n"
			"Cache-Control: no-cache, must-revalidate\r\n"
			"Pragma: no-cache\r\n"
			"Expires: Sun, 12 Feb 2016 00:00:00 GMT\r\n"
			"Content-Type: text/html; charset=utf-8\r\n\r\n"
			"%s",
			code.c_str(), text.c_str());
		send_timeout(clientSocket, reply, strlen(reply), 0);
	}

	string WebClient::selectLoco(const map<string,string>& options) {
		stringstream ss;
		ss << "<form method=\"get\" action=\"/\" id=\"selectLoco_form\">";
		ss << "<select name=\"loco\" onchange=\"loadDivFromForm('selectLoco_form', 'loco')\">";
		for (auto option : options) {
			ss << "<option value=\"" << option.first << "\">" << option.second << "</option>";
		}
		ss << "</select>";
		ss << "<input type=\"hidden\" name=\"cmd\" value=\"loco\">";
		ss << "</form>";
		return ss.str();
	}

	string WebClient::select(const string& name, const map<string,string>& options, const std::string& defaultValue) {
		stringstream ss;
		ss << "<select name=\"" << name << "\" id=\"" << buttonID << "_" << name << "\" onchange=\"reloadProtocol()\">";
		for (auto option : options) {
			ss << "<option value=\"" << option.first << "\"" << (option.first.compare(defaultValue) ? "" : " selected") << ">" << option.second << "</option>";
		}
		ss << "</select>";
		++buttonID;
		return ss.str();
	}

	string WebClient::slider(const string& name, const string& cmd, const unsigned int min, const unsigned int max, const unsigned int value, const map<string,string>& arguments) {
		locoID_t locoID = 0;
		if (arguments.count("loco")) locoID = stoi(arguments.at("loco"));
		stringstream ss;
		ss << "<input class=\"slider\" type=\"range\" min=\"" << min << "\" max=\"" << max << "\" name=\"" << name << "\" id=\"" << cmd << "_" << locoID<< "\" value=\"" << value << "\">";
		ss << "<script type=\"application/javascript\">\n"
			"$(function() {\n"
			" $('#" << cmd << "_" << locoID << "').on('change', function() {\n"
			"  var theUrl = '/?cmd=" << cmd;
		for (auto argument : arguments) {
			ss << "&" << argument.first << "=" << argument.second;
		}
		ss << "&" << name << "=" << "' + document.getElementById('" << cmd << "_" << locoID <<"').value;\n"
			"  var xmlHttp = new XMLHttpRequest();\n"
			"  xmlHttp.open('GET', theUrl, true);\n"
			"  xmlHttp.send(null);\n"
			"  return false;\n"
			" })\n"
			"});\n"
			"</script>";
		return ss.str();
	}

	string WebClient::button(const string& value, const string& cmd, const map<string,string>& arguments) {
		stringstream ss;
		ss <<
			"<input class=\"button\" id=\"" << buttonID << "_" << cmd << "\" type=\"submit\" value=\"" << value << "\">"
			"<script type=\"application/javascript\">\n"
			"$(function() {\n"
			" $('#" << buttonID << "_"<< cmd << "').on('click', function() {\n"
			"  var theUrl = '/?cmd=" << cmd;
		for (auto argument : arguments) {
			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';\n"
			"  var xmlHttp = new XMLHttpRequest();\n"
			"  xmlHttp.open('GET', theUrl, true);\n"
			"  xmlHttp.send(null);\n"
			"  return false;\n"
			" })\n"
			"})\n"
			"</script>";
		++buttonID;
		return ss.str();
	}

	string WebClient::buttonPopup(const string& value, const string& cmd, const map<string,string>& arguments) {
		stringstream ss;
		ss <<
			"<input class=\"button\" id=\"" << buttonID << "_" << cmd << "\" type=\"submit\" value=\"" << value << "\">"
			"<script type=\"application/javascript\">\n"
			"$(function() {\n"
			" $('#" << buttonID << "_"<< cmd << "').on('click', function() {\n"
			"  var theUrl = '/?cmd=" << cmd;
		for (auto argument : arguments) {
			ss << "&" << argument.first << "=" << argument.second;
		}
		ss <<"';\n"
			"  $('#popup').show();\n"
			"  $('#popup').load(theUrl);\n"
			" })\n"
			"})\n"
			"</script>";
		++buttonID;
		return ss.str();
	}

	string WebClient::buttonPopupCancel() {
		stringstream ss;
		ss <<
			"<input class=\"button\" id=\"popup_cancel\" type=\"submit\" value=\"Cancel\">"
			"<script type=\"application/javascript\">\n"
			"$(function() {\n"
			" $('#popup_cancel').on('click', function() {\n"
			"  $('#popup').hide();\n"
			" })\n"
			"})\n"
			"</script>";
		return ss.str();
	}

	string WebClient::buttonPopupOK() {
		stringstream ss;
		ss <<
			"<input class=\"button\" id=\"popup_ok\" type=\"submit\" value=\"Save\">"
			"<script type=\"application/javascript\">\n"
			"$(function() {\n"
			" $('#editform').on('submit', function() {\n"
			"  $.ajax({\n"
			"   data: $(this).serialize(),\n"
			"   type: $(this).attr('get'),\n"
			"   url: $(this).attr('/'),\n"
			"   success: function(response) {\n"
			"    $('#popup').html(response);\n"
			"   }\n"
			"  })\n"
			"  $('#popup').hide(2000);\n"
			"  return false;\n"
			" });\n"
			"});\n"
			"$(function() {\n"
			" $('#popup_ok').on('click', function() {\n"
			"  $('#editform').submit();\n"
			"  return false;\n"
			" });\n"
			"});\n"
			"</script>\n";
		return ss.str();
	}

	void WebClient::printLoco(const map<string, string>& arguments) {
		string sOut;
		if (arguments.count("loco")) {
			map<string,string> buttonArguments;
			string locoIDString = arguments.at("loco");
			locoID_t locoID = stoi(locoIDString);
			buttonArguments["loco"] = locoIDString;
			stringstream ss;
			Loco* loco = manager.getLoco(locoID);
			ss << "<p>";
			ss << loco->name;
			ss << "</p>";
			unsigned int speed = loco->Speed();
			ss << slider("speed", "locospeed", 0, MaxSpeed, speed, buttonArguments);
			buttonArguments["speed"] = "0";
			ss << button("0%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "255";
			ss << button("25%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "511";
			ss << button("50%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "767";
			ss << button("75%", "locospeed", buttonArguments);
			buttonArguments["speed"] = "1023";
			ss << button("100%", "locospeed", buttonArguments);
			buttonArguments.erase("speed");

			buttonArguments["function"] = "0";
			buttonArguments["on"] = "1";
			ss << button("f0 on", "locofunction", buttonArguments);
			buttonArguments["on"] = "0";
			ss << button("f0 off", "locofunction", buttonArguments);
			buttonArguments.erase("function");
			buttonArguments.erase("on");

			buttonArguments["direction"] = "forward";
			ss << button("forward", "locodirection", buttonArguments);
			buttonArguments["direction"] = "reverse";
			ss << button("reverse", "locodirection", buttonArguments);
			buttonArguments.erase("direction");

			ss << buttonPopup("Edit", "locoedit", buttonArguments);
			sOut = ss.str();
		}
		else {
			sOut = "No locoID provided";
		}
		simpleReply(sOut);
	}

	void WebClient::printMainHTML() {
		// handle base request
		stringstream ss;
		ss << "HTTP/1.0 200 OK\r\n"
			"Cache-Control: no-cache, must-revalidate\r\n"
			"Pragma: no-cache\r\n"
			"Expires: Sun, 12 Feb 2016 00:00:00 GMT\r\n"
			"Content-Type: text/html; charset=utf-8\r\n\r\n"
			"<!DOCTYPE html><html><head>"
			"<title>RailControl</title>"
			"<link rel=\"stylesheet\" type=\"text/css\" href=\"/style.css\" />"
			"<script src=\"/jquery-3.1.1.min.js\"></script>"
			"<script src=\"/javascript.js\"></script>"
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			"<meta name=\"robots\" content=\"noindex,nofollow\">"
			"</head>"
			"<body onload=\"loadDivFromForm('selectLoco_form', 'loco')\">"
			"<h1>Railcontrol</h1>"
			"<div class=\"menu\">";
		ss << button("X", "quit");
		ss << button("On", "on");
		ss << button("Off", "off");
		ss << buttonPopup("NewLoco", "locoedit");
		ss << "</div>";
		ss << "<div class=\"locolist\">";
		// locolist
		const map<locoID_t, Loco*>& locos = manager.locoList();
		map<string,string> options;
		for (auto locoTMP : locos) {
			Loco* loco = locoTMP.second;
			options[to_string(loco->objectID)] = loco->name;
		}
		ss << selectLoco(options);
		ss <<"</div>";
		ss << "<div class=\"loco\" id=\"loco\">";
		ss << "</div>";
		ss << "<div class=\"popup\" id=\"popup\">Popup</div>"
			"<div class=\"status\" id=\"status\"></div>"
			"<script type=\"application/javascript\">\n"
			"var updater = new EventSource('/?cmd=updater');\n"
			"updater.onmessage = function(e) {\n"
			" var status = document.getElementById('status');\n"
			" var arguments = e.data.split(';');\n"
			" var argumentMap = new Map();\n"
			" arguments.forEach(function(argument) {\n"
			"  var parts = argument.split('=');\n"
			"  if (parts[0] == 'status') {\n"
			"   status.innerHTML += parts[1] + '<br>';\n"
			"   status.scrollTop = status.scrollHeight - status.clientHeight;\n"
			"  }\n"
			"  else {\n"
			"   argumentMap.set(parts[0], parts[1]);\n"
			"  }\n"
			" })\n"
			" if (argumentMap.get('command') == 'locospeed') {\n"
			"  var elementName = 'locospeed_' + argumentMap.get('loco');\n"
			"  var element = document.getElementById(elementName);\n"
			"  if (element) element.value = argumentMap.get('speed');\n"
			" }\n"
			"};\n"

			// FIXME: get first locoid in db
			"</script>"
			"</body>"
			"</html>";
		string sOut = ss.str();
		const char* html = sOut.c_str();
		send_timeout(clientSocket, html, strlen(html), 0);
	}

} ; // namespace webserver
