/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include "Languages.h"
#include "Utils/Utils.h"

const char* Languages::languages[MaxTexts][MaxLanguages] =
{
/* TextReceivedSignalKill */ { "Received a signal kill {0} times. Exiting without saving.", "Signal Kill {0} mal erhalten. Beende RailControl ohne zu speichern.", "Apago RailControl sin guardar." },
/* TextStoppingRailControl */ { "Stopping RailControl", "Beende RailControl", "Apaga RailControl" },
/* TextStoppingRequestedBySignal */ { "Stopping RailControl requested by signal {0}", "Beenden von RailControl angefordert mit Signal {0}", "Apagar RailControl pedido con signal {0}" },
/* TextStoppingRequestedByWebClient */ { "Stopping RailControl requested by webclient", "Beenden von RailControl angefordert von Webclient", "Apagar RailControl pedido del webclient" },
};

Languages::language_t Languages::defaultLanguage = Languages::EN;

Languages::language_t Languages::ParseLanguage(std::string& languageString)
{
	if (languageString.size() < 2)
	{
		return Languages::EN;
	}

	Utils::Utils::StringToUpper(languageString);
	if (languageString[0] == 'D' && languageString[1] == 'E')
	{
		return Languages::DE;
	}

	if (languageString[0] == 'E' && languageString[1] == 'S')
	{
		return Languages::ES;
	}

	return Languages::EN;
}
