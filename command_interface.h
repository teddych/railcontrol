#pragma once

#include <map>
#include <string>
#include <vector>

#include "datatypes.h"

class CommandInterface
{
	public:
		CommandInterface(controlType_t controlType) : controlType(controlType) {}
		virtual ~CommandInterface() {};
		const controlType_t getcontrolType() const { return controlType; }
		virtual const std::string getName() const = 0;
		virtual void Booster(const controlType_t controlType, const boosterState_t state) = 0;
		virtual void locoSettings(const locoID_t locoID, const std::string& name) {};
		virtual void locoDelete(const locoID_t locoID, const std::string& name) {};
		virtual void LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed) = 0;
		virtual void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction) = 0;
		virtual void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on) = 0;
		virtual void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) = 0;
		virtual void accessorySettings(const accessoryID_t accessoryID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) {}
		virtual void accessoryDelete(const accessoryID_t accessoryID, const std::string& name) {}
		virtual void FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state) = 0;
		virtual void FeedbackSettings(const feedbackID_t feedbackID, const std::string& name) {}
		virtual void FeedbackDelete(const feedbackID_t feedbackID, const std::string& name) {}
		virtual void track(const controlType_t controlType, const trackID_t trackID, const lockState_t state) = 0;
		virtual void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on) = 0;
		virtual void switchSettings(const switchID_t switchID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const std::string& rotation) {};
		virtual void switchDelete(const switchID_t switchID, const std::string& name) {};
		virtual void locoIntoTrack(const locoID_t locoID, const trackID_t trackID) = 0;
		virtual void locoRelease(const locoID_t) = 0;
		virtual void trackSettings(const trackID_t trackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const std::string& rotation) {};
		virtual void trackDelete(const trackID_t trackID, const std::string& name) {};
		virtual void trackRelease(const trackID_t) = 0;
		virtual void streetSettings(const streetID_t streetID, const std::string& name) {};
		virtual void streetDelete(const streetID_t streetID, const std::string& name) {};
		virtual void streetRelease(const streetID_t) = 0;
		virtual void layerSettings(const layerID_t layerID, const std::string& name) {};
		virtual void layerDelete(const layerID_t layerID, const std::string& name) {};
		virtual void locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) = 0;
		virtual void locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) = 0;
		virtual void locoStart(const locoID_t locoID) = 0;
		virtual void locoStop(const locoID_t locoID) = 0;
		virtual bool CanHandleLocos() const { return false; }
		virtual bool CanHandleAccessories() const { return false; }
		virtual bool CanHandleFeedback() const { return false; }
		virtual void GetLocoProtocols(std::vector<protocol_t>& protocols) const {};
		virtual bool LocoProtocolSupported(protocol_t protocol) const { return false; };
		virtual void GetAccessoryProtocols(std::vector<protocol_t>& protocols) const {};
		virtual bool AccessoryProtocolSupported(protocol_t protocol) const { return false; };
		virtual void GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const {}

	private:
		controlType_t controlType;
};
