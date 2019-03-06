#pragma once

#include <map>
#include <string>
#include <vector>

#include "datatypes.h"

class ControlInterface
{
	public:
		ControlInterface(controlType_t controlType) : controlType(controlType) {}
		virtual ~ControlInterface() {};
		const controlType_t ControlType() const { return controlType; }
		virtual const std::string Name() const = 0;
		virtual void AccessoryDelete(const accessoryID_t accessoryID, const std::string& name) {}
		virtual void AccessoryProtocols(std::vector<protocol_t>& protocols) const {};
		virtual bool AccessoryProtocolSupported(protocol_t protocol) const { return false; };
		virtual void AccessorySettings(const accessoryID_t accessoryID, const std::string& name) {}
		virtual void AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on) {};
		virtual void ArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const {}
		virtual void Booster(const controlType_t controlType, const boosterState_t state) {};
		virtual bool CanHandleAccessories() const { return false; }
		virtual bool CanHandleFeedback() const { return false; }
		virtual bool CanHandleLocos() const { return false; }
		virtual void FeedbackDelete(const feedbackID_t feedbackID, const std::string& name) {}
		virtual void FeedbackSettings(const feedbackID_t feedbackID, const std::string& name) {}
		virtual void FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state) {};
		virtual void LayerDelete(const layerID_t layerID, const std::string& name) {};
		virtual void LayerSettings(const layerID_t layerID, const std::string& name) {};
		virtual void LocoDelete(const locoID_t locoID, const std::string& name) {};
		virtual void LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) {};
		virtual void LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction) {};
		virtual void LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on) {};
		virtual void LocoIntoTrack(const locoID_t locoID, const trackID_t trackID) {};
		virtual void LocoProtocols(std::vector<protocol_t>& protocols) const {};
		virtual bool LocoProtocolSupported(protocol_t protocol) const { return false; };
		virtual void LocoRelease(const locoID_t locoID) {};
		virtual void LocoSettings(const locoID_t locoID, const std::string& name) {};
		virtual void LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed) {};
		virtual void LocoStart(const locoID_t locoID) {};
		virtual void LocoStop(const locoID_t locoID) {};
		virtual void LocoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID) {};
		virtual void StreetDelete(const streetID_t streetID, const std::string& name) {};
		virtual void StreetRelease(const streetID_t streetID) {};
		virtual void StreetSettings(const streetID_t streetID, const std::string& name) {};
		virtual void SwitchDelete(const switchID_t switchID, const std::string& name) {};
		virtual void SwitchSettings(const switchID_t switchID, const std::string& name) {};
		virtual void SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on) {};
		virtual void TrackDelete(const trackID_t trackID, const std::string& name) {};
		virtual void TrackRelease(const trackID_t trackID) {};
		virtual void TrackSettings(const trackID_t trackID, const std::string& name) {};
		virtual void TrackState(const controlType_t controlType, const trackID_t trackID, const lockState_t state) {};

	private:
		controlType_t controlType;
};
