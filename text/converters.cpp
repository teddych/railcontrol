#include "text/converters.h"
#include "datamodel/accessory.h"
#include "datamodel/signal.h"
#include "datamodel/switch.h"

using std::string;

namespace text
{
	void Converters::accessoryStatus(const accessoryState_t state, string& onText)
	{
		onText.assign(state == datamodel::Accessory::AccessoryStateOn ? "green" : "red");
	}

	void Converters::switchStatus(const switchState_t state, string& stateText)
	{
		stateText.assign(state == datamodel::Switch::SwitchStateStraight ? "straight" : "turnout");
	}

	void Converters::signalStatus(const signalState_t state, string& stateText)
	{
		stateText.assign(state == datamodel::Signal::SignalStateGreen ? "green" : "red");
	}
}
