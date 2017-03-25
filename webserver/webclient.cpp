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
			manager.booster(MANAGER_ID_WEBSERVER, BOOSTER_STOP);
			stopRailControl(SIGTERM);
		}
		else if (arguments["cmd"].compare("on") == 0) {
			simpleReply("Turning booster on");
			manager.booster(MANAGER_ID_WEBSERVER, BOOSTER_GO);
		}
		else if (arguments["cmd"].compare("off") == 0) {
			simpleReply("Turning booster off");
			manager.booster(MANAGER_ID_WEBSERVER, BOOSTER_STOP);
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
								// FIXME: %20 and other coded characters are not interpreted correctly
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

		manager.locoSpeed(MANAGER_ID_WEBSERVER, locoID, speed);

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

		manager.locoDirection(MANAGER_ID_WEBSERVER, locoID, direction);

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

		manager.locoFunction(MANAGER_ID_WEBSERVER, locoID, function, on);

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
		if (!manager.autoMode) {
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
			ss << "<label>Control:</label><select name=\"controlid\">";
			std::map<controlID_t,string> controls = manager.controlList();
			for (auto control : controls) {
				ss << "<option value=\"" << (unsigned int)control.first << "\"" << (control.first == controlID ? " selected" : "") << ">" << control.second << "</option>";
			}
			ss << "</select>";
			ss << "<div id=\"protocol\">";
			std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
			// FIXME: Update protocols on control-change
			ss << "<label>Protocol:</label><select name=\"protocol\">";
			for (auto protocol2 : protocols) {
				ss << "<option value=\"" << (unsigned int)protocol2.first << "\"" << (protocol2.first == protocol ? " selected" : "") << ">" << protocol2.second << "</option>";
			}
			ss << "</select>";
			ss << "</div>";
			ss << inputText("Address:", "address", address);
			ss << "</form>";
			ss << buttonPopupCancel();
			ss << buttonPopupOK();
		}
		else {
			ss << "<h1>Unable to edit locos in auto mode</h1>";
			ss << buttonPopupCancel();
		}
		string sOut = ss.str();
		simpleReply(sOut);
	}

	void WebClient::handleProtocol(const map<string, string>& arguments) {
		stringstream ss;
		if (arguments.count("control")) {
			controlID_t controlID = stoi(arguments.at("control"));
			protocol_t selectedProtocol = PROTOCOL_NONE;
			/*
			FIXME: get selectedProtocol
			if (arguments.count("loco")) {
				locoID_t locoID = stoi(arguments.at("loco"));
				if (locos.count(locoID)) {
					Loco* loco = locos.at(locoID);
					selectedProtocol = loco->protocol;
				}
			}
			*/
			std::map<protocol_t,string> protocols = manager.protocolsOfControl(controlID);
			ss << "<label>Protocol:</label><select name=\"protocol\">";
			for (auto protocol : protocols) {
				ss << "<option value=\"" << (unsigned int)protocol.first << "\"" << (protocol.first == selectedProtocol ? " selected" : "") << ">" << protocol.second << "</option>";
			}
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
			if (arguments.count("controlid")) controlID = stoi(arguments.at("controlid"));
			if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
			if (arguments.count("address")) address = stoi(arguments.at("address"));
			manager.locoSave(locoID, name, controlID, protocol, address);
			ss << "<p>Loco &quot;" << locoID << "&quot; saved.</p>";
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

	string WebClient::select(const string& name, const map<string,string>& options, const string& cmd, const string& target, const map<string,string>& arguments) {
		stringstream ss;
		ss << "<form method=\"get\" action=\"/\" id=\"" << buttonID << "_"<< cmd << "_" << "form\">";
		ss << "<select name=\"" << name << "\" id=\"" << buttonID << "_" << cmd << "\">";
		for (auto option : options) {
			ss << "<option value=\"" << option.first << "\">" << option.second << "</option>";
		}
		ss << "</select>";
		for (auto argument : arguments) {
			ss << "<input type=\"hidden\" name=\"" << argument.first << "\" value=\"" << argument.second << "\">";
		}
		ss << "<input type=\"hidden\" name=\"cmd\" value=\"" << cmd << "\">";
		ss << "</form>";
		ss << "<script>\n"
		"$(function() {\n"
		" $('#" << buttonID << "_"<< cmd << "_" << "form').on('submit', function() {\n"
		"  $.ajax({\n"
		"   data: $(this).serialize(),\n"
		"   type: $(this).attr('get'),\n"
		"   url: $(this).attr('/'),\n"
		"   success: function(response) {\n"
		"    $('#" << target << "').html(response);\n"
		"   }\n"
		"  })\n"
		"  return false;\n"
		" });\n"
		"});\n"
		"$(function() {\n"
		" $('#" << buttonID << "_"<< cmd << "').on('change', function() {\n"
		"  $('#" << buttonID << "_"<< cmd << "_" << "form').submit();\n"
		"  return false;\n"
		" });\n"
		"});\n"
		"</script>";
		++buttonID;
		return ss.str();
	}

	string WebClient::slider(const string& name, const string& cmd, const unsigned int min, const unsigned int max, const map<string,string>& arguments) {
		locoID_t locoID = 0;
		if (arguments.count("loco")) locoID = stoi(arguments.at("loco"));
		stringstream ss;
		ss << "<input class=\"slider\" type=\"range\" min=\"" << min << "\" max=\"" << max << "\" name=\"" << name << "\" id=\"" << cmd << "_" << locoID<< "\">";
		ss << "<script>\n"
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
			"<script>\n"
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
			"<script>\n"
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
			"<script>\n"
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
			"<script>\n"
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
			string locoID = arguments.at("loco");
			buttonArguments["loco"] = locoID;
			stringstream ss;
			ss << "<p>";
			ss << manager.getLocoName(stoi(locoID));
			ss << "</p>";
			ss << slider("speed", "locospeed", 0, 1024, buttonArguments);
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
			"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
			"<meta name=\"robots\" content=\"noindex,nofollow\">"
			"</head>"
			"<body>"
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
			options[std::to_string(loco->locoID)] = loco->name;
		}
		ss << select("loco", options, "loco", "loco");
		ss <<"</div>";
		ss << "<div class=\"loco\" id=\"loco\">";
		ss << "</div>";
		ss << "<div class=\"popup\" id=\"popup\">Popup</div>"
			"<div class=\"status\" id=\"status\"></div>"
			"<script>\n"
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
			"$('#loco').load('/?cmd=loco&loco=1');\n"

			"</script>"
			"</body>"
			"</html>";
		string sOut = ss.str();
		const char* html = sOut.c_str();
		send_timeout(clientSocket, html, strlen(html), 0);
	}

} ; // namespace webserver
