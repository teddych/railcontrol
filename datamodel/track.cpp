
#include <map>
#include <sstream>
#include <string>

#include <datamodel/track.h>

using std::map;
using std::string;

namespace datamodel
{
	std::string Track::Serialize() const
	{
		std::stringstream ss;
		ss << "objectType=Track;" << LayoutItem::Serialize()
			<< ";type=" << static_cast<int>(type)
			<< ";lockState=" << static_cast<int>(lockState)
			<< ";locoID=" << (int) locoID
			<< ";locoDirection=" << static_cast<int>(locoDirection);
		return ss.str();
	}

	bool Track::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		parseArguments(serialized, arguments);
		LayoutItem::Deserialize(arguments);
		width = Width1;
		visible = VisibleYes;
		if (!arguments.count("objectType") || arguments.at("objectType").compare("Track") != 0)
		{
			return false;
		}
		type = static_cast<trackType_t>(GetIntegerMapEntry(arguments, "type", TrackTypeStraight));
		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		locoID = GetIntegerMapEntry(arguments, "locoID", LocoNone);
		locoDirection = static_cast<direction_t>(GetBoolMapEntry(arguments, "locoDirection", DirectionLeft));
		return true;
	}

	bool Track::reserve(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (locoID == this->locoID)
		{
			if (lockState == LockStateFree)
			{
				lockState = LockStateReserved;
			}
			return true;
		}
		if (lockState != LockStateFree)
		{
			return false;
		}
		lockState = LockStateReserved;
		this->locoID = locoID;
		return true;
	}

	bool Track::lock(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState != LockStateReserved)
		{
			return false;
		}
		if (this->locoID != locoID)
		{
			return false;
		}
		lockState = LockStateHardLocked;
		return true;
	}

	bool Track::release(const locoID_t locoID)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		if (lockState == LockStateFree)
		{
			return true;
		}
		if (this->locoID != locoID)
		{
			return false;
		}
		this->locoID = LocoNone;
		lockState = LockStateFree;
		return true;
	}

	bool Track::addStreet(Street* street)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto s : streets)
		{
			if (s == street)
			{
				return false;
			}
		}
		streets.push_back(street);
		return true;
	}

	bool Track::removeStreet(Street* street)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		/* FIXME */
		return false;
	}

	bool Track::getValidStreets(std::vector<Street*>& validStreets)
	{
		std::lock_guard<std::mutex> Guard(updateMutex);
		for (auto street : streets)
		{
			if (street->fromTrackDirection(objectID, locoDirection))
			{
				validStreets.push_back(street);
			}
		}
		return true;
	}
} // namespace datamodel
