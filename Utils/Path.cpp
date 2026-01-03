/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2025 by Teddy / Dominik Mahrer - www.railcontrol.org

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

#include <cstdlib>
#include <iostream>

#include "Utils/Path.h"

namespace Utils::Path
{
	std::filesystem::path getConfDir()
	{
		const std::filesystem::path dir;
		// systemd may set this if ConfigurationDirectory= is set for a service
		const char* env_p = std::getenv("CONFIGURATION_DIRECTORY");
		if (env_p)
		{
			std::filesystem::path dir(env_p);
		}
		else
		{
			std::filesystem::path dir(".");
		}

		return dir;
	}

	std::filesystem::path getDataDir()
	{
#ifdef RAILCONTROL_DATADIR
		const std::filesystem::path dir(RAILCONTROL_DATADIR);
#else
		const std::filesystem::path dir(".");
#endif
		return dir;
	}

	std::filesystem::path getStateDir()
	{
		const std::filesystem::path dir;
		// systemd may set this if StateDirectory= is set for a service
		const char* env_p = std::getenv("STATE_DIRECTORY");
		if (env_p)
		{
			std::filesystem::path dir(env_p);
		}
		else
		{
			std::filesystem::path dir(".");
		}

		return dir;
	}
}
