#pragma once

#include <map>
#include <mutex>
#include <vector>

#include "ControlInterface.h"
#include "manager.h"
#include "network/TcpServer.h"

namespace webserver
{
	class WebClient;

	class WebServer : public ControlInterface, private Network::TcpServer
	{
		public:
			WebServer(Manager& manager, const unsigned short port);
			~WebServer();

			void Work(Network::TcpConnection* connection) override;

			bool NextUpdate(unsigned int& updateIDClient, std::string& s);

			const std::string Name() const override { return "Webserver"; }
			void AccessoryDelete(const accessoryID_t accessoryID, const std::string& name) override;
			void AccessorySettings(const accessoryID_t accessoryID, const std::string& name) override;
			void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) override;
			void Booster(const controlType_t controlType, const boosterState_t status) override;
			void FeedbackDelete(const feedbackID_t feedbackID, const std::string& name) override;
			void FeedbackSettings(const feedbackID_t feedbackID, const std::string& name) override;
			void FeedbackState(const controlType_t controlType, const std::string& name, const feedbackID_t feedbackID, const feedbackState_t state) override;
			void LayerDelete(const layerID_t layerID, const std::string& name) override;
			void LayerSettings(const layerID_t layerID, const std::string& name) override;
			void LocoDelete(const locoID_t locoID, const std::string& name) override;
			void LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
			void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction) override;
			void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on) override;
			void LocoIntoTrack(const locoID_t locoID, const trackID_t trackID) override;
			void LocoRelease(const locoID_t locoID) override;
			void LocoSettings(const locoID_t locoID, const std::string& name) override;
			void LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed) override;
			void LocoStart(const locoID_t locoID) override;
			void LocoStop(const locoID_t locoID) override;
			void LocoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) override;
			void StreetDelete(const streetID_t streetID, const std::string& nam) override;
			void StreetRelease(const streetID_t streetID) override;
			void StreetSettings(const streetID_t streetID, const std::string& name) override;
			void SwitchDelete(const switchID_t switchID, const std::string& name) override;
			void SwitchSettings(const switchID_t switchID, const std::string& name) override;
			void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on) override;
			void TrackDelete(const trackID_t trackID, const std::string& name) override;
			void TrackSettings(const trackID_t trackID, const std::string& name) override;
			void TrackState(const controlType_t controlType, const std::string& name, const trackID_t trackID, const feedbackState_t state, const std::string& locoName) override;

		private:
			void AddUpdate(const std::string& command, const std::string& status);

			volatile bool run;
			unsigned int lastClientID;
			std::vector<WebClient*> clients;
			Manager& manager;

			std::map<unsigned int,std::string> updates;
			std::mutex updateMutex;
			unsigned int updateID;
			const unsigned int MaxUpdates = 10;
	};
}; // namespace webserver

