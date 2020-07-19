/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <deque>
#include <string>

#include "DataModel/LocoFunctions.h"
#include "Utils/Utils.h"

namespace DataModel
{
	LocoFunctions::LocoFunctions()
	{
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			entries[nr].nr = nr;
			entries[nr].type = LocoFunctionTypeNone;
			entries[nr].icon = LocoFunctionIconNone;
			entries[nr].timer = 0;
		}
	}

	std::string LocoFunctions::Serialize() const
	{
		std::string out;
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			if (entries[nr].type == LocoFunctionTypeNone)
			{
				continue;
			}
			out += "f" + std::to_string(static_cast<unsigned int>(nr));
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].state));
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].type));
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].icon));
			if (entries[nr].type != LocoFunctionTypeTimer)
			{
				continue;
			}
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].icon));
		}
		return out;
	}

	bool LocoFunctions::Deserialize(const std::string& serialized)
	{
		size_t count = serialized.size();
		if (count == 0 || serialized[0] == 'f')
		{
			DeserializeNew(serialized);
			return true;
		}
		DeserializeOld(serialized);
		return true;
	}

	bool LocoFunctions::DeserializeNew(__attribute__((unused))  const std::string& serialized)
	{
		std::deque<std::string> functionsSerialized;
		Utils::Utils::SplitString(serialized, "f", functionsSerialized);
		for (std::string& functionSerialized : functionsSerialized)
		{
			if (functionSerialized.size() == 0)
			{
				continue;
			}
			std::deque<std::string> functionTexts;
			Utils::Utils::SplitString(functionSerialized, ":", functionTexts);
			const size_t nrOfTexts = functionTexts.size();
			if (nrOfTexts != 4 && nrOfTexts != 5)
			{
				continue;
			}
			LocoFunctionNr nr = Utils::Utils::StringToInteger(functionTexts.front());
			functionTexts.pop_front();
			entries[nr].state = static_cast<LocoFunctionState>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionStateOff));
			functionTexts.pop_front();
			entries[nr].type = static_cast<LocoFunctionType>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionTypePermanent));
			functionTexts.pop_front();
			entries[nr].icon = static_cast<LocoFunctionIcon>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionIconDefault));
			functionTexts.pop_front();
			if (nrOfTexts == 4)
			{
				entries[nr].timer = 0;
				continue;
			}
			entries[nr].timer = static_cast<LocoFunctionTimer>(Utils::Utils::StringToInteger(functionTexts.front(), 0));
			functionTexts.pop_front();
		}
		return true;
	}

	// FIXME: remove later
	bool LocoFunctions::DeserializeOld(const std::string& serialized)
	{
		size_t count = serialized.size();
		if (count > MaxCount)
		{
			count = MaxCount;
		}
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			if (nr >= count)
			{
				entries[nr].state = LocoFunctionStateOff;
				entries[nr].type = LocoFunctionTypeNone;
				entries[nr].icon = LocoFunctionIconNone;
				entries[nr].timer = 0;
				continue;
			}
			switch (serialized[nr])
			{
				case '1':
					entries[nr].state = LocoFunctionStateOn;
					break;

				default:
					entries[nr].state = LocoFunctionStateOff;
					break;
			}
			entries[nr].type = LocoFunctionTypePermanent;
			entries[nr].icon = LocoFunctionIconDefault;
			entries[nr].timer = 0;
		}
		return true;
	}

	std::string LocoFunctions::GetLocoFunctionIcon(const LocoFunctionNr nr, const LocoFunctionIcon icon)
	{
		switch (icon)
		{
			case LocoFunctionIconNone:
				return "<svg width=\"36\" height=\"36\" />";

			default:
				return "<svg width=\"36\" height=\"36\">"
					"<text x=\"8\" y=\"24\" fill=\"black\" font-size=\"12\">F" + std::to_string(nr) + "</text>"
					"</svg>";

			case LocoFunctionIconShuntingMode:
				return "<svg width=\"36\" height=\"36\">"
					"<polyline points=\"5,24 5.2,21.9 5.7,19.9 6.6,18 7.8,16.3 9.3,14.8 11,13.6 12.9,12.7 14.9,12.2 17,12 19.1,12.2 21.1,12.7 23,13.6 24.7,14.8 26.2,16.3 27.4,18 28.3,19.9 28.8,21.9 29,24\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<circle r=\"3\" cx=\"11\" cy=\"24\" fill=\"black\" />"
					"<circle r=\"3\" cx=\"23\" cy=\"24\" fill=\"black\" />"
					"<circle r=\"3\" cx=\"29\" cy=\"20\" fill=\"black\" />"
					"</svg>";

			case LocoFunctionIconInertia:
				return "<svg width=\"36\" height=\"36\">"
					"<polyline points=\"8,16 16,16 16,10 20,10 20,16 28,16 28,28 8,28\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"18,7 18.9,7 19.7,7.2 20.5,7.4 21.2,7.7 21.8,8.1 22.3,8.5 22.7,9 22.9,9.5 23,10 22.9,10.5 22.7,11 22.3,11.5 21.8,11.9 21.2,12.3 20.5,12.6 19.7,12.8 18.9,13 18,13 17.1,13 16.3,12.8 15.5,12.6 14.8,12.3 14.2,11.9 13.7,11.5 13.3,11 13.1,10.5 13,10 13.1,9.5 13.3,9 13.7,8.5 14.2,8.1 14.8,7.7 15.5,7.4 16.3,7.2 17.1,7\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<text x=\"12\" y=\"25\" fill=\"white\" font-size=\"10\">kg</text></svg>";

			case LocoFunctionIconLight:
				return "<svg width=\"36\" height=\"36\">"
					"<polyline points=\"15.5,22.3 14.8,21.8 14.2,21.2 13.7,20.5 13.3,19.7 13.1,18.9 13,18 13.1,17.1 13.3,16.3 13.7,15.5 14.2,14.8 14.8,14.2 15.5,13.7 16.3,13.3 17.1,13.1 18,13 18.9,13.1 19.7,13.3 20.5,13.7 21.2,14.2 21.8,14.8 22.3,15.5 22.7,16.3 22.9,17.1 23,18 22.9,18.9 22.7,19.7 22.3,20.5 21.8,21.2 21.2,21.8 20.5,22.3\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"15,23 21,23 21,30 18,32 15,30\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"11.1,22 6.7,24.5\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"10,18 5,18\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"11.1,14 6.7,11.5\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"14,11.1 11.5,6.7\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"18,10 18,5\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"22,11.1 24.5,6.7\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"24.9,14 29.3,11.5\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"26,18 31,18\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"<polyline points=\"24.9,22 29.3,24.5\" stroke=\"black\" stroke-width=\"1\" class=\"button_on\"/>"
					"</svg>";

			case LocoFunctionIconHeadlightLowBeamForward:
				return "<svg width=\"36\" height=\"36\">"
					"<polyline points=\"16,30 12,30 9.9,29.8 7.9,29.3 6,28.4 4.3,27.2 2.8,25.7 1.6,24 0.7,22.1 0.2,20.1 0,18 0.2,15.9 0.7,13.9 1.6,12 2.8,10.3 4.3,8.8 6,7.6 7.9,6.7 9.9,6.2 12,6 16,6\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"17,30 17,6 19,6 19,30\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"21,8 35,14\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,13 35,19\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,18 35,24\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,23 35,29\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,28 35,34\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"</svg>";

			case LocoFunctionIconHeadlightHighBeamForward:
				return "<svg width=\"36\" height=\"36\">"
					"<polyline points=\"16,30 12,30 9.9,29.8 7.9,29.3 6,28.4 4.3,27.2 2.8,25.7 1.6,24 0.7,22.1 0.2,20.1 0,18 0.2,15.9 0.7,13.9 1.6,12 2.8,10.3 4.3,8.8 6,7.6 7.9,6.7 9.9,6.2 12,6 16,6\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"17,30 17,6 19,6 19,30\" stroke=\"black\" stroke-width=\"0\" fill=\"black\"/>"
					"<polyline points=\"21,8 35,8\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,13 35,13\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,18 35,18\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,23 35,23\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"<polyline points=\"21,28 35,28\" stroke=\"black\" stroke-width=\"2\" fill=\"black\" class=\"button_on\"/>"
					"</svg>";
		}
	}
} // namespace DataModel
