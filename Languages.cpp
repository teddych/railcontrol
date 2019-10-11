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
/* TextLayer1 */ { "Layer 1", "Schicht 1", "Capa 1" },
/* TextLoadedAccessory */ { "Loaded accessory {0}: {1}", "Zubehörartikel {0} geladen: {1}", "Cargado accesorio {0}: {1}" },
/* TextLoadedControl */ { "Loaded control {0}: {1}", "Zentrale {0} geladen: {1}", "Cargado control {0}: {1}" },
/* TextLoadedFeedback */ { "Loaded feedback {0}: {1}", "Rückmelder {0} geladen: {1}", "Cargado retroseñal {0}: {1}" },
/* TextLoadedLayer */ { "Loaded layer {0}: {1}", "Schicht {0} geladen: {1}", "Cargado capa {0}: {1}" },
/* TextLoadedLoco */ { "Loaded locomotive {0}: {1}", "Lokomotive {0} geladen: {1}", "Cargado locomotora {0}: {1}" },
/* TextLoadedSignal */ { "Loaded signal {0}: {1}", "Signal {0} geladen: {1}", "Cargado señal {0}: {1}" },
/* TextLoadedStreet */ { "Loaded street {0}: {1}", "Fahrstrasse {0} geladen: {1}", "Cargado itinerario {0}: {1}" },
/* TextLoadedSwitch */ { "Loaded switch {0}: {1}", "Weiche {0} geladen: {1}", "Cargado desvío {0}: {1}" },
/* TextLoadedTrack */ { "Loaded track {0}: {1}", "Gleis {0} geladen: {1}", "Cargado vía {0}: {1}" },
/* TextManager */ { "Manager", "Manager", "Manager" },
/* TextReceivedSignalKill */ { "Received a signal kill {0} times. Exiting without saving.", "Signal Kill {0} mal erhalten. Beende RailControl ohne zu speichern.", "Apago RailControl sin guardar." },
/* TextSaving */ { "Saving {0}", "Speichere {0}", "Guardando {0}" },
/* TextStoppingRailControl */ { "Stopping RailControl", "Beende RailControl", "Apaga RailControl" },
/* TextStoppingRequestedBySignal */ { "Stopping RailControl requested by signal {0}", "Beenden von RailControl angefordert mit Signal {0}", "Apagar RailControl pedido con señal {0}" },
/* TextStoppingRequestedByWebClient */ { "Stopping RailControl requested by webclient", "Beenden von RailControl angefordert von Webclient", "Apagar RailControl pedido del webclient" },
/* TextUnableAddLayer1 */ { "Unable to add initial layer 1", "Unmöglich die Schicht 1 hinzuzufügen", "Imposible de añadir capa 1" },
/* TextUnableCreateStorageHandler */ { "Unable to create storage handler", "Unmöglich den Speicher Handler zu erstellen", "Imposible de crear manipulador de almacenamiento" },
/* TextUnknownAccessory */ { "Unknown accessory", "Unbekannter Zubehörartikel", "Accesorio desconocido" },
/* TextUnknownControl */ { "Unknown control", "Unbekannte Zentrale", "Control desconocido" },
/* TextUnknownFeedback */ { "Unknown feedback", "Unbekannter Rückmelder", "Retroseñal desconocida" },
/* TextUnknownLoco */ { "Unknown locomotive", "Unbekannte Lokomotive", "Locomotora desconocida" },
/* TextUnknownSignal */ { "Unknown signal", "Unbekanntes Signal", "Señal desconocida" },
/* TextUnknownStreet */ { "Unknown street", "Unbekannte Fahrstrasse", "Itinerario desconocido" },
/* TextUnknownSwitch */ { "Unknown switch", "Unbekannte Weiche", "Desvío desconocido" },
/* TextUnknownTrack */ { "Unknown track", "Unbekanntes Gleis", "Vía desconocida" },
///*  */ { "", "", "" },
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
