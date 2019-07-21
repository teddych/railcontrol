#pragma once

#include <map>
#include <string>
#include <thread>
#include <vector>

#include "manager.h"
#include "network/TcpConnection.h"
#include "webserver/Response.h"

namespace webserver
{
	class WebServer;

	class WebClient
	{
		public:
			WebClient(const unsigned int id, Network::TcpConnection* connection, WebServer &webserver, Manager& m)
			:	logger(Logger::Logger::GetLogger("Webserver")),
				id(id),
				connection(connection),
				run(false),
				server(webserver),
				clientThread(&webserver::WebClient::Worker, this),
				manager(m),
				buttonID(0)
			{}

			~WebClient();
			void Worker();
			int Stop();

		private:
			void InterpretClientRequest(const std::vector<std::string>& lines, std::string& method, std::string& uri, std::string& protocol, std::map<std::string,std::string>& arguments, std::map<std::string,std::string>& headers);
			void PrintLoco(const std::map<std::string, std::string>& arguments);
			void PrintMainHTML();
			void HtmlReplyErrorWithHeader(const std::string& erroText);
			void HtmlReplyWithHeader(const HtmlTag& tag);
			void HtmlReplyWithHeaderAndParagraph(const std::string& content) { HtmlReplyWithHeader(HtmlTag("p").AddContent(content)); }
			void HtmlReplyWithHeaderAndParagraph(const char* content) { HtmlReplyWithHeaderAndParagraph(std::string(content)); }
			void DeliverFile(const std::string& file);
			void DeliverFileInternal(FILE* f, const char* realFile, const std::string& file);
			HtmlTag HtmlTagLocoSelector() const;
			HtmlTag HtmlTagLayerSelector() const;
			HtmlTag HtmlTagControlArgument(const unsigned char argNr, const argumentType_t type, const std::string& value);
			HtmlTag HtmlTagProtocolLoco(const controlID_t controlID, const protocol_t selectedProtocol);
			HtmlTag HtmlTagProtocolAccessory(const controlID_t controlID, const protocol_t selectedProtocol);
			HtmlTag HtmlTagDuration(const accessoryDuration_t duration, const std::string& label = "Duration (ms):") const;
			HtmlTag HtmlTagPosition(const layoutPosition_t posx, const layoutPosition_t posy, const layoutPosition_t posz);
			HtmlTag HtmlTagPosition(const layoutPosition_t posx, const layoutPosition_t posy, const layoutPosition_t posz, const visible_t visible);
			HtmlTag HtmlTagRotation(const layoutRotation_t rotation) const;
			HtmlTag HtmlTagSelectTrack(const std::string& name, const std::string& label, const trackID_t trackId, const direction_t direction, const std::string& onchange = "") const;
			HtmlTag HtmlTagSelectFeedbacksOfTrack(const trackID_t trackId, const feedbackID_t feedbackIdReduced, const feedbackID_t feedbackIdCreep, const feedbackID_t feedbackIdStop, const feedbackID_t feedbackIdOver) const;
			HtmlTag HtmlTagRelation(const std::string& priority, const objectType_t objectType = ObjectTypeSwitch, const objectID_t objectId = ObjectNone, const accessoryState_t state = datamodel::Accessory::AccessoryStateOff);
			HtmlTag HtmlTagRelationObject(const std::string& priority, const objectType_t objectType, const objectID_t objectId = ObjectNone, const accessoryState_t state = datamodel::Accessory::AccessoryStateOff);
			HtmlTag HtmlTagTabMenuItem(const std::string& tabName, const std::string& buttonValue, const bool selected = false) const;
			HtmlTag HtmlTagSelectFeedbackForTrack(const unsigned int counter, const trackID_t trackID, const feedbackID_t feedbackID = FeedbackNone);
			static HtmlTag HtmlTagSelectSelectStreetApproach(const datamodel::Track::selectStreetApproach_t selectStreetApproach, const bool addDefault);
			static HtmlTag HtmlTagNrOfTracksToReserve(const datamodel::Loco::nrOfTracksToReserve_t nrOfTracksToReserve);
			void HandleSelectLoco(const std::map<std::string, std::string>& arguments);
			void HandleLayerEdit(const std::map<std::string, std::string>& arguments);
			void HandleLayerSave(const std::map<std::string, std::string>& arguments);
			void HandleLayerList();
			void HandleLayerAskDelete(const std::map<std::string, std::string>& arguments);
			void HandleLayerDelete(const std::map<std::string, std::string>& arguments);
			void HandleControlEdit(const std::map<std::string, std::string>& arguments);
			void HandleControlSave(const std::map<std::string, std::string>& arguments);
			void HandleControlList();
			void HandleControlAskDelete(const std::map<std::string, std::string>& arguments);
			void HandleControlDelete(const std::map<std::string, std::string>& arguments);
			void HandleLocoSpeed(const std::map<std::string,std::string>& arguments);
			void HandleLocoDirection(const std::map<std::string,std::string>& arguments);
			void HandleLocoFunction(const std::map<std::string, std::string>& arguments);
			void HandleLocoEdit(const std::map<std::string, std::string>& arguments);
			void HandleLocoSave(const std::map<std::string, std::string>& arguments);
			void HandleLocoList();
			void HandleLocoAskDelete(const std::map<std::string, std::string>& arguments);
			void HandleLocoDelete(const std::map<std::string, std::string>& arguments);
			void HandleLocoRelease(const std::map<std::string, std::string>& arguments);
			void HandleProtocolLoco(const std::map<std::string, std::string>& arguments);
			void HandleProtocolAccessory(const std::map<std::string, std::string>& arguments);
			void HandleProtocolSwitch(const std::map<std::string, std::string>& arguments);
			void HandleLayout(const std::map<std::string,std::string>& arguments);
			void HandleAccessoryEdit(const std::map<std::string,std::string>& arguments);
			void HandleAccessorySave(const std::map<std::string,std::string>& arguments);
			void HandleAccessoryState(const std::map<std::string,std::string>& arguments);
			void HandleAccessoryList();
			void HandleAccessoryAskDelete(const std::map<std::string,std::string>& arguments);
			void HandleAccessoryDelete(const std::map<std::string,std::string>& arguments);
			void HandleAccessoryGet(const std::map<std::string,std::string>& arguments);
			void HandleAccessoryRelease(const std::map<std::string,std::string>& arguments);
			void HandleSwitchEdit(const std::map<std::string,std::string>& arguments);
			void HandleSwitchSave(const std::map<std::string,std::string>& arguments);
			void HandleSwitchState(const std::map<std::string,std::string>& arguments);
			void HandleSwitchList();
			void HandleSwitchAskDelete(const std::map<std::string,std::string>& arguments);
			void HandleSwitchDelete(const std::map<std::string,std::string>& arguments);
			void HandleSwitchGet(const std::map<std::string,std::string>& arguments);
			void HandleSwitchRelease(const std::map<std::string,std::string>& arguments);
			void HandleSignalEdit(const std::map<std::string,std::string>& arguments);
			void HandleSignalSave(const std::map<std::string,std::string>& arguments);
			void HandleSignalState(const std::map<std::string,std::string>& arguments);
			void HandleSignalList();
			void HandleSignalAskDelete(const std::map<std::string,std::string>& arguments);
			void HandleSignalDelete(const std::map<std::string,std::string>& arguments);
			void HandleSignalGet(const std::map<std::string,std::string>& arguments);
			void HandleSignalRelease(const std::map<std::string,std::string>& arguments);
			void HandleStreetEdit(const std::map<std::string,std::string>& arguments);
			void HandleStreetSave(const std::map<std::string,std::string>& arguments);
			void HandleStreetList();
			void HandleStreetAskDelete(const std::map<std::string,std::string>& arguments);
			void HandleStreetDelete(const std::map<std::string,std::string>& arguments);
			void HandleStreetGet(const std::map<std::string,std::string>& arguments);
			void HandleStreetExecute(const std::map<std::string,std::string>& arguments);
			void HandleStreetRelease(const std::map<std::string,std::string>& arguments);
			void HandleTrackEdit(const std::map<std::string,std::string>& arguments);
			void HandleTrackSave(const std::map<std::string,std::string>& arguments);
			void HandleTrackList();
			void HandleTrackAskDelete(const std::map<std::string,std::string>& arguments);
			void HandleTrackDelete(const std::map<std::string,std::string>& arguments);
			void HandleTrackGet(const std::map<std::string, std::string>& arguments);
			void HandleTrackSetLoco(const std::map<std::string, std::string>& arguments);
			void HandleTrackRelease(const std::map<std::string, std::string>& arguments);
			void HandleTrackStartLoco(const std::map<std::string, std::string>& arguments);
			void HandleTrackStopLoco(const std::map<std::string, std::string>& arguments);
			void HandleTrackBlock(const std::map<std::string, std::string>& arguments);
			void HandleFeedbackEdit(const std::map<std::string,std::string>& arguments);
			void HandleFeedbackSave(const std::map<std::string,std::string>& arguments);
			void HandleFeedbackState(const std::map<std::string,std::string>& arguments);
			void HandleFeedbackList();
			void HandleFeedbackAskDelete(const std::map<std::string,std::string>& arguments);
			void HandleFeedbackDelete(const std::map<std::string,std::string>& arguments);
			void HandleFeedbackGet(const std::map<std::string,std::string>& arguments);
			void HandleFeedbacksOfTrack(const std::map<std::string,std::string>& arguments);
			void HandleLocoSelector();
			void HandleLayerSelector();
			void HandleRelationAdd(const std::map<std::string,std::string>& arguments);
			void HandleFeedbackAdd(const std::map<std::string,std::string>& arguments);
			void HandleRelationObject(const std::map<std::string, std::string>& arguments);
			void HandleSettingsEdit();
			void HandleSettingsSave(const std::map<std::string, std::string>& arguments);
			void HandleTimestamp(const std::map<std::string,std::string>& arguments);
			void HandleUpdater(const std::map<std::string,std::string>& headers);
			void UrlDecode(std::string& argumentValue);
			char ConvertHexToInt(char c);
			void WorkerImpl();

			Logger::Logger* logger;
			unsigned int id;
			Network::TcpConnection* connection;
			volatile unsigned char run;
			WebServer& server;
			std::thread clientThread;
			Manager& manager;
			bool headOnly;
			unsigned int buttonID;
	};

}; // namespace webserver

