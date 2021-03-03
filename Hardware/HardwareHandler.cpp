/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2021 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#include "DataModel/Loco.h"
#include "DataModel/LocoFunctions.h"
#include "DataTypes.h"
#include "Logger/Logger.h"
#include "Hardware/CS2Udp.h"
#include "Hardware/CS2Tcp.h"
#include "Hardware/CcSchnitte.h"
#include "Hardware/Ecos.h"
#include "Hardware/HardwareHandler.h"
#include "Hardware/Hsi88.h"
#include "Hardware/M6051.h"
#include "Hardware/OpenDcc.h"
#include "Hardware/RM485.h"
#include "Hardware/Virtual.h"
#include "Hardware/Z21.h"
#include "Utils/Utils.h"

using std::string;

namespace Hardware
{
	const std::string HardwareHandler::hardwareSymbols[] =
	{
		"none",
		"Virtual",
		"CS2Udp",
		"M6051",
		"RM485",
		"OpenDcc",
		"Hsi88",
		"Z21",
		"CcSchnitte",
		"Ecos",
		"CS2Tcp"
	};
	const std::string HardwareHandler::Unknown = "Unknown, not running";

	void HardwareHandler::Init(const HardwareParams* params)
	{
		this->params = params;
		HardwareType type = params->GetHardwareType();

		switch(type)
		{
			case HardwareTypeCS2Udp:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_CS2Udp);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_CS2Udp);
				break;

			case HardwareTypeCS2Tcp:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_CS2Tcp);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_CS2Tcp);
				break;

			case HardwareTypeVirtual:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_Virtual);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_Virtual);
				break;


			case HardwareTypeM6051:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_M6051);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_M6051);
				break;

			case HardwareTypeRM485:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_RM485);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_RM485);
				break;

			case HardwareTypeOpenDcc:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_OpenDcc);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_OpenDcc);
				break;

			case HardwareTypeHsi88:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_Hsi88);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_Hsi88);
				break;

			case HardwareTypeZ21:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_Z21);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_Z21);
				break;

			case HardwareTypeCcSchnitte:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_CcSchnitte);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_CcSchnitte);
				break;

			case HardwareTypeEcos:
				createHardware = (Hardware::HardwareInterface* (*)(const Hardware::HardwareParams*))(&create_Ecos);
				destroyHardware = (void (*)(Hardware::HardwareInterface*))(&destroy_Ecos);
				break;

			default:
				createHardware = nullptr;
				destroyHardware = nullptr;
				break;
		}

		// start control
		if (createHardware != nullptr)
		{
			instance = createHardware(params);
		}
	}

	void HardwareHandler::Close()
	{
		Hardware::HardwareInterface* instanceTemp = instance;
		instance = nullptr;
		createHardware = nullptr;
		if (instanceTemp != nullptr)
		{
			destroyHardware(instanceTemp);
		}
		destroyHardware = nullptr;
		params = nullptr;
	}

	const std::string& HardwareHandler::GetName() const
	{
		if (instance == nullptr)
		{
			return Unknown;
		}
		return instance->GetFullName();
	}

	const std::string& HardwareHandler::GetShortName() const
	{
		if (instance == nullptr)
		{
			return Unknown;
		}
		return instance->GetShortName();
	}

	Hardware::Capabilities HardwareHandler::GetCapabilities() const
	{
		if (instance == nullptr)
		{
			return Hardware::CapabilityNone;
		}

		return instance->GetCapabilities();
	}

	void HardwareHandler::LocoProtocols(std::vector<Protocol>& protocols) const
	{
		if (instance == nullptr)
		{
			protocols.push_back(ProtocolNone);
			return;
		}

		instance->GetLocoProtocols(protocols);
	}

	bool HardwareHandler::LocoProtocolSupported(Protocol protocol) const
	{
		if (instance == nullptr)
		{
			return false;
		}
		return instance->LocoProtocolSupported(protocol);
	}

	void HardwareHandler::AccessoryProtocols(std::vector<Protocol>& protocols) const
	{
		if (instance == nullptr)
		{
			protocols.push_back(ProtocolNone);
			return;
		}

		instance->GetAccessoryProtocols(protocols);
	}

	bool HardwareHandler::AccessoryProtocolSupported(Protocol protocol) const
	{
		if (instance == nullptr)
		{
			return false;
		}
		return instance->AccessoryProtocolSupported(protocol);
	}

	void HardwareHandler::Booster(const ControlType controlType, const BoosterState status)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		instance->Booster(status);
	}

	void HardwareHandler::LocoSpeed(const ControlType controlType, const DataModel::Loco* loco, const Speed speed)
	{
		if (controlType == ControlTypeHardware || instance == nullptr || loco->GetControlID() != GetControlID())
		{
			return;
		}
		instance->LocoSpeed(loco->GetProtocol(), loco->GetAddress(), speed);
	}

	void HardwareHandler::LocoOrientation(const ControlType controlType, const DataModel::Loco* loco, const Orientation orientation)
	{
		if (controlType == ControlTypeHardware || instance == nullptr || loco->GetControlID() != GetControlID())
		{
			return;
		}
		instance->LocoOrientation(loco->GetProtocol(), loco->GetAddress(), orientation);
	}

	void HardwareHandler::LocoFunction(const ControlType controlType,
		const DataModel::Loco* loco,
		const DataModel::LocoFunctionNr function,
		const DataModel::LocoFunctionState on)
	{
		if (controlType == ControlTypeHardware || instance == nullptr || loco->GetControlID() != GetControlID())
		{
			return;
		}
		instance->LocoFunction(loco->GetProtocol(), loco->GetAddress(), function, on);
	}

	void HardwareHandler::LocoSpeedOrientationFunctions(const DataModel::Loco* loco,
		const Speed speed,
		const Orientation orientation,
		std::vector<DataModel::LocoFunctionEntry>& functions)
	{
		if (instance == nullptr || loco->GetControlID() != GetControlID())
		{
			return;
		}
		instance->LocoSpeedOrientationFunctions(loco->GetProtocol(), loco->GetAddress(), speed, orientation, functions);
	}

	void HardwareHandler::LocoSettings(const LocoID locoId,
		__attribute__((unused)) const std::string& name,
		const std::string& matchKey)
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->SetLocoIdOfMatch(locoId, matchKey);
	}

	void HardwareHandler::AccessoryState(const ControlType controlType, const DataModel::Accessory* accessory)
	{
		if (controlType == ControlTypeHardware
			|| instance == nullptr
			|| accessory == nullptr
			|| accessory->GetControlID() != this->GetControlID())
		{
			return;
		}
		instance->Accessory(accessory->GetProtocol(), accessory->GetAddress(), accessory->GetInvertedAccessoryState(), accessory->GetAccessoryPulseDuration());
	}

	void HardwareHandler::SwitchState(const ControlType controlType, const DataModel::Switch* mySwitch)
	{
		if (controlType == ControlTypeHardware
			|| instance == nullptr
			|| mySwitch == nullptr
			|| mySwitch->GetControlID() != this->GetControlID())
		{
			return;
		}

		Protocol protocol =  mySwitch->GetProtocol();
		Address address =  mySwitch->GetAddress();
		DataModel::AccessoryPulseDuration duration = mySwitch->GetAccessoryPulseDuration();
		if (mySwitch->GetType() == DataModel::SwitchTypeThreeWay)
		{
			switch (mySwitch->GetAccessoryState())
			{
				case DataModel::SwitchStateTurnout:
					instance->Accessory(protocol, address + 1, mySwitch->CalculateInvertedAccessoryState(DataModel::AccessoryStateOff), duration);
					instance->Accessory(protocol, address, mySwitch->CalculateInvertedAccessoryState(DataModel::AccessoryStateOff), duration);
					break;

				case DataModel::SwitchStateStraight:
					instance->Accessory(protocol, address, mySwitch->CalculateInvertedAccessoryState(DataModel::AccessoryStateOn), duration);
					instance->Accessory(protocol, address + 1, mySwitch->CalculateInvertedAccessoryState(DataModel::AccessoryStateOff), duration);
					break;

				case DataModel::SwitchStateThird:
					instance->Accessory(protocol, address, mySwitch->CalculateInvertedAccessoryState(DataModel::AccessoryStateOn), duration);
					instance->Accessory(protocol, address + 1, mySwitch->CalculateInvertedAccessoryState(DataModel::AccessoryStateOn), duration);
					break;

				default:
					break;
			}
			return;
		}
		// else left or right switch
		instance->Accessory(protocol, address, mySwitch->GetInvertedAccessoryState(), duration);
	}

	void HardwareHandler::SignalState(const ControlType controlType, const DataModel::Signal* signal)
	{
		if (controlType == ControlTypeHardware
			|| instance == nullptr
			|| signal == nullptr
			|| signal->GetControlID() != this->GetControlID())
		{
			return;
		}
		instance->Accessory(signal->GetProtocol(), signal->GetAddress(), signal->GetInvertedAccessoryState(), signal->GetAccessoryPulseDuration());
	}

	bool HardwareHandler::ProgramCheckValues(const ProgramMode mode, const CvNumber cv, const CvValue value)
	{
		if (cv == 0)
		{
			return false; // cvs are one based, so cv zero is not allowed
		}
		if (cv == 1 && value == 0)
		{
			return false; // loco/accessory address zero is not allowed for DCC decoders and can not be undone. It would destroy some DCC decoders.
		}
		CvNumber maxCv;
		switch (mode)
		{
			case ProgramModeMm:
			case ProgramModeMmPom:
				maxCv = 0x100;
				break;

			case ProgramModeDccDirect:
			case ProgramModeDccPomLoco:
			case ProgramModeMfx:
				maxCv = 0x4000;
				break;

			case ProgramModeDccPomAccessory:
				maxCv = 0x800;
				break;

			default:
				return false;
		}
		return (cv <= maxCv);
	}

	void HardwareHandler::ProgramRead(const ProgramMode mode, const Address address, const CvNumber cv)
	{
		if (ProgramCheckValues(mode, cv) == false)
		{
			return;
		}
		if (instance == nullptr)
		{
			return;
		}

		instance->ProgramRead(mode, address, cv);
	}

	void HardwareHandler::ProgramWrite(const ProgramMode mode, const Address address, const CvNumber cv, const CvValue value)
	{
		if (ProgramCheckValues(mode, cv, value) == false)
		{
			return;
		}
		if (instance == nullptr)
		{
			return;
		}
		instance->ProgramWrite(mode, address, cv, value);
	}

	void HardwareHandler::AddUnmatchedLocos(std::map<std::string,DataModel::LocoConfig>& list) const
	{
		if (instance == nullptr)
		{
			return;
		}

		const std::map<std::string,Hardware::LocoCacheEntry>& database = instance->GetLocoDatabase();
		for (auto entry : database)
		{
			Hardware::LocoCacheEntry& loco = entry.second;
			if (loco.GetLocoID() != LocoNone)
			{
				continue;
			}
			list[loco.GetName() + " (" + instance->GetShortName() + ")"] = loco;
		}
	}

	std::map<std::string,DataModel::LocoConfig> HardwareHandler::GetUnmatchedLocos() const
	{
		std::map<std::string,DataModel::LocoConfig> out;
		if (instance == nullptr)
		{
			return out;
		}

		const std::map<std::string,Hardware::LocoCacheEntry>& database = instance->GetLocoDatabase();
		for (auto entry : database)
		{
			Hardware::LocoCacheEntry& loco = entry.second;
			if (loco.GetLocoID() != LocoNone)
			{
				continue;
			}
			out[loco.GetName()] = loco;
		}
		return out;
	}

	std::map<std::string,DataModel::LocoConfig> HardwareHandler::GetAllLocos() const
	{
		std::map<std::string,DataModel::LocoConfig> out;
		if (instance == nullptr)
		{
			return out;
		}

		const std::map<std::string,Hardware::LocoCacheEntry>& database = instance->GetLocoDatabase();
		for (auto entry : database)
		{
			Hardware::LocoCacheEntry& loco = entry.second;
			out[loco.GetName()] = loco;
		}
		return out;
	}

	DataModel::LocoConfig HardwareHandler::GetLocoByMatch(const std::string& match) const
	{
		if (instance == nullptr)
		{
			return DataModel::LocoConfig();
		}
		return instance->GetLocoByMatch(match);
	}

	void HardwareHandler::ArgumentTypesOfHardwareTypeAndHint(const HardwareType hardwareType, std::map<unsigned char,ArgumentType>& arguments, std::string& hint)
	{
		switch (hardwareType)
		{
			case HardwareTypeCS2Udp:
				Hardware::CS2Udp::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeCS2Tcp:
				Hardware::CS2Tcp::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeM6051:
				Hardware::M6051::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeRM485:
				Hardware::RM485::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeOpenDcc:
				Hardware::OpenDcc::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeHsi88:
				Hardware::Hsi88::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeZ21:
				Hardware::Z21::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeCcSchnitte:
				Hardware::CcSchnitte::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeEcos:
				Hardware::Ecos::GetArgumentTypesAndHint(arguments, hint);
				return;

			case HardwareTypeVirtual:
				Hardware::Virtual::GetHint(hint);
				return;

			default:
				hint = "";
				return;
		}
	}
} // namespace Hardware
