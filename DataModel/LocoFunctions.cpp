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
				return "<svg width=\"36\" height=\"36\"><text x=\"8\" y=\"24\" fill=\"black\" font-size=\"12\">F" + std::to_string(nr) + "</text></svg>";

			case LocoFunctionIconShuntingMode:
				return "<svg width=\"36\" height=\"36\"><polyline points=\"5,22 5.2,19.9 5.7,17.9 6.6,16 7.8,14.3 9.3,12.8 11,11.6 12.9,10.7 14.9,10.2 17,10 19.1,10.2 21.1,10.7 23,11.6 24.7,12.8 26.2,14.3 27.4,16 28.3,17.9 28.8,19.9 29,22\" stroke=\"black\"     stroke-width=\"0\" fill=\"black\"/><circle r=\"3\" cx=\"11\" cy=\"22\" fill=\"black\" /><circle r=\"3\" cx=\"23\" cy=\"22\" fill=\"black\" /><circle r=\"3\" cx=\"29\" cy=\"18\" fill=\"black\" /></svg>";
		}
	}
} // namespace DataModel
