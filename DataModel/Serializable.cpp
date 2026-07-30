/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

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

#include "DataModel/Serializable.h"
#include "Utils/Integer.h"
#include "Utils/Utils.h"

using std::deque;
using std::string;
using std::map;

namespace DataModel
{
	string Serializable::SerializeBinaryData(const string& data)
	{
		static const char hex[] = "0123456789abcdef";
		string encoded;
		encoded.reserve(data.size() * 2);
		for (const unsigned char value : data)
		{
			encoded += hex[value >> 4];
			encoded += hex[value & 0x0F];
		}
		return encoded;
	}

	string Serializable::DeserializeBinaryData(const string& data)
	{
		string decoded;
		decoded.reserve(data.size() / 2);
		for (size_t pos = 0; pos + 1 < data.size(); pos += 2)
		{
			const unsigned char highNibble = Utils::Integer::HexToChar(data[pos]);
			const unsigned char lowNibble = Utils::Integer::HexToChar(data[pos + 1]);
			decoded += static_cast<char>((highNibble << 4) + lowNibble);
		}
		return decoded;
	}

	void Serializable::ParseArguments(const string& serialized, map<string, string>& arguments)
	{
		deque<string> parts;
		Utils::Utils::SplitString(serialized, ";", parts);
		for (auto& part : parts)
		{
			if (part.length() == 0)
			{
				continue;
			}
			deque<string> keyValue;
			Utils::Utils::SplitString(part, "=", keyValue);
			if (keyValue.size() < 2)
			{
				continue;
			}
			string value = keyValue[1];
			arguments[keyValue[0]] = value;
		}
	}
}
