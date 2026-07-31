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
#include "Utils/Utils.h"

using std::deque;
using std::string;
using std::map;

namespace DataModel
{
	const char Serializable::Base64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	void Serializable::ParseArguments(const string& serialized, map<string, string>& arguments)
	{
		deque<string> parts;
		Utils::Utils::SplitString(serialized, ";", parts);
		for (auto& part : parts)
		{
			const size_t pos = part.find('=');
			if (pos == string::npos)
			{
				continue;
			}
			arguments[part.substr(0, pos)] = part.substr(pos + 1);
		}
	}

	string Serializable::SerializeBinaryData(const string& data)
	{
		string encoded;
		encoded.reserve(((data.size() + 2) / 3) * 4);
		for (size_t pos = 0; pos < data.size(); pos += 3)
		{
			const unsigned char byte1 = data[pos];
			const unsigned char byte2 = pos + 1 < data.size() ? data[pos + 1] : 0;
			const unsigned char byte3 = pos + 2 < data.size() ? data[pos + 2] : 0;
			encoded += Base64Alphabet[byte1 >> 2];
			encoded += Base64Alphabet[((byte1 & 0x03) << 4) | (byte2 >> 4)];
			if (pos + 1 < data.size())
			{
				encoded += Base64Alphabet[((byte2 & 0x0F) << 2) | (byte3 >> 6)];
			}
			if (pos + 2 < data.size())
			{
				encoded += Base64Alphabet[byte3 & 0x3F];
			}
		}
		return encoded;
	}

	string Serializable::DeserializeBinaryData(const string& data)
	{
		string decoded;
		decoded.reserve((data.size() / 4) * 3);
		unsigned int buffer = 0;
		unsigned int bits = 0;
		for (const char character : data)
		{
			const signed char value = DecodeBase64Value(character);
			if (value < 0)
			{
				continue;
			}
			buffer = (buffer << 6) | value;
			bits += 6;
			if (bits >= 8)
			{
				bits -= 8;
				decoded += static_cast<char>((buffer >> bits) & 0xFF);
			}
		}
		return decoded;
	}

	signed char Serializable::DecodeBase64Value(const char character)
	{
		if (character >= 'A' && character <= 'Z')
		{
			return character - 'A';
		}
		if (character >= 'a' && character <= 'z')
		{
			return character - 'a' + 26;
		}
		if (character >= '0' && character <= '9')
		{
			return character - '0' + 52;
		}
		if (character == '+')
		{
			return 62;
		}
		if (character == '/')
		{
			return 63;
		}
		return -1;
	}
}
