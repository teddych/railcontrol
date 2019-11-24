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
/* TextAccessoryAddressDccTooHigh */ { "Addresses higher then 2044 are not supported by DCC", "Adressen grösser als 2044 werden nicht unterstützt von DCC", "Direcciones más grandes que 2044 no están compatibles con DCC" },
/* TextAccessoryAddressMmTooHigh */ { "Addresses higher then 320 are not supported by MM1/MM2", "Adressen grösser als 320 werden nicht unterstützt von MM1/MM2", "Direcciones más grandes que 320 no están compatibles con MM1/MM2" },
/* TextAccessoryDeleted */ { "Accessory {0} deleted", "Zubehörartikel {0} gelöscht", "Accessorio {0} eliminado" },
/* TextAccessoryStateIsGreen */ { "Accessory state of {0} is now green", "Der Status des Zubehörartikels ist nun grün", "El estado del accessorio está verde" },
/* TextAccessoryStateIsRed */ { "Accessory state of {0} is now red", "Der Status des Zubehörartikels ist nun rot", "El estado del accessorio está rojo" },
/* TextAccessoryUpdated */ { "Accessory {0} updated", "Zubehörartikel {0} aktualisiert", "Accessorio {0} actualizado" },
/* TextAddingFeedback */ { "Adding feedback {0}", "Rückmelder {0} hinzugefügt", "Retroseñal añadido" },
/* TextAddressMustBeHigherThen0 */ { "Address must be higher then 0", "Adresse muss grösser sein als 0", "Dirección tiene que ser mas que 0" },
/* TextConnectionReset */ { "Connection reset", "Verbindung zurückgesetzt", "Conexión reajustado" },
/* TextControlDoesNotExist */ { "Control does not exist", "Zentrale existiert nicht", "Control no existe" },
/* TextDebounceThreadStarted */ { "Debounce thread started", "Entprellthread gestartet", "Antirebote thread encendido" },
/* TextDebounceThreadTerminated */ { "Debounce thread terminated", "Entprellthread beedet", "Antirebote thread apagado" },
/* TextDebouncer */ { "Debouncer", "Entpreller", "Antirebote" },
/* TextDifferentCommuterTypes */ { "Loco and street {0} have different commuter types", "Lok und Fahrstrasse {0} haben verschiedene Pendelzugeinstellungen", "Tren e itinerario tienen ajustes de viavén differentes" },
/* TextDifferentDirections */ { "Loco and street {0} have different running directions", "Lok und Fahrstrasse {0} haben verschiedene Fahrtrichtungen", "Tren e itinerario tienen sentidos de la marcha differentes" },
/* TextFeedbackChange */ { "State of pin {0} on S88 module {1} is {2}", "Status von Pin {0} an S88 Modul {1} ist {2}", "Estado de contacto {0} del S88 módulo {1} está {2}" },
/* TextFeedbackStateIsOff */ { "Feedback state of {0} is now off", "Der Status des Rückmelders ist nun aus", "El estado del retroseñal {0} está apagado" },
/* TextFeedbackStateIsOn */ { "Feedback state of {0} is now on", "Der Status des Rückmelders ist nun ein", "El estado del retroseñal {0} está encendido" },
/* TextGreen */ { "green", "grün", "verde" },
/* TextHeightIs0 */ { "Height is zero", "Höhe ist null", "Altura está zero" },
/* TextHsi88Configured */ { "{0} ({1}/{2}/{3}) S88 modules configured.", "{0} ({1}/{2}/{3}) S88 Module konfiguriert", "{0} ({1}/{2}/{3}) S88 módulos configurado" },
/* TextHsi88ErrorConfiguring */ { "Unable to configure HSI-88. HSI-88 returned {0} configured modules", "HSI-88 kann nicht konfiguriert werden. HSI-88 meldet {0} module", "Imposible configurar HSI-88. HSI-88 denuncia {0} módulos" },
/* TextInvalidDataReceived */ { "Invalid data received", "Ungültige Daten empfangen", "Recibido datos non validos" },
/* TextIsLocked */ { "{0} is locked", "{0} ist gesperrt", "{0} está bloqueado" },
/* TextLayer1 */ { "Layer 1", "Schicht 1", "Capa 1" },
/* TextLeft */ { "left", "links", "izquierda" },
/* TextLoadedAccessory */ { "Loaded accessory {0}: {1}", "Zubehörartikel {0} geladen: {1}", "Cargado accesorio {0}: {1}" },
/* TextLoadedControl */ { "Loaded control {0}: {1}", "Zentrale {0} geladen: {1}", "Cargado control {0}: {1}" },
/* TextLoadedFeedback */ { "Loaded feedback {0}: {1}", "Rückmelder {0} geladen: {1}", "Cargado retroseñal {0}: {1}" },
/* TextLoadedLayer */ { "Loaded layer {0}: {1}", "Schicht {0} geladen: {1}", "Cargado capa {0}: {1}" },
/* TextLoadedLoco */ { "Loaded locomotive {0}: {1}", "Lokomotive {0} geladen: {1}", "Cargado locomotora {0}: {1}" },
/* TextLoadedSignal */ { "Loaded signal {0}: {1}", "Signal {0} geladen: {1}", "Cargado señal {0}: {1}" },
/* TextLoadedStreet */ { "Loaded street {0}: {1}", "Fahrstrasse {0} geladen: {1}", "Cargado itinerario {0}: {1}" },
/* TextLoadedSwitch */ { "Loaded switch {0}: {1}", "Weiche {0} geladen: {1}", "Cargado desvío {0}: {1}" },
/* TextLoadedTrack */ { "Loaded track {0}: {1}", "Gleis {0} geladen: {1}", "Cargado vía {0}: {1}" },
/* TextLocoAddressDccTooHigh */ { "Addresses higher then 10239 are not supported by DCC", "Adressen grösser als 10239 werden nicht unterstützt von DCC", "Direcciones más grandes que 10239 no están compatibles con DCC" },
/* TextLocoAddressMmTooHigh */ { "Addresses higher then 80 are not supported by MM1/MM2", "Adressen grösser als 80 werden nicht unterstützt von MM1/MM2", "Direcciones más grandes que 80 no están compatibles con MM1/MM2" },
/* TextLocoDirectionIsLeft */ { "Direction of loco {0} is now left", "Die Fahrtrichtung der Lok {0} ist links", "La dirección de la locomotora {0} está izquierda" },
/* TextLocoDirectionIsRight */ { "Direction of loco {0} is now right", "Die Fahrtrichtung der Lok {0} ist rechts", "La dirección de la locomotora {0} está derecha" },
/* TextLocoFunctionIsOff */ { "F{1} of loco {0} is now off", "F{1} der Lok {0} ist aus", "F{1} de la locomotora {0} está apagado" },
/* TextLocoFunctionIsOn */ { "F{1} of loco {0} is now on", "F{1} der Lok {0} ist an", "F{1} de la locomotora {0} está encendido" },
/* TextLocoSpeedIs */ { "Speed of loco {0} is now {1}", "Die Geschwindigkeit der Lok {0} ist {1}", "La velocidad de la locomotora {0} está {1}" },
/* TextManager */ { "Manager", "Manager", "Manager" },
/* TextNoS88Modules */ { "No S88 modules configured", "Keine S88 Module konfiguriert", "No hay módulos S88 configurado" },
/* TextOff */ { "off", "aus", "apagado" },
/* TextOn */ { "on", "ein", "encendido" },
/* TextPositionAlreadyInUse */ { "Position {0}/{1}/{2} is already used by {3} \"{4}\".", "Position {0}/{1}/{2} ist bereits verwendet von {3} \"{4}\".", "Positión {0}/{1}/{2} está usado de {3} \"{4}\"." },
/* TextProtocolNotSupported */ { "Protocol {0}is not supported by control. Please use one of: {1}", "Protokoll {0} wird nicht unterstützt von der Zentrale. Verwende eines aus {1}", "Protocolo {0}no compatible con esa control. Utilisa uno de {1}" },
/* TextRailControlStarted */ { "RailControl started", "RailControl wurde gestartet", "RailControl encendido" },
/* TextReceivedSignalKill */ { "Received a signal kill {0} times. Exiting without saving.", "Signal Kill {0} mal erhalten. Beende RailControl ohne zu speichern.", "Apago RailControl sin guardar." },
/* TextReceiverThreadStarted */ { "Receiver thread started", "Empfangs-Thread gestartet", "Thread recibiendo creado" },
/* TextRed */ { "red", "rot", "rojo" },
/* TextRight */ { "right", "rechts", "derecho" },
/* TextSaving */ { "Saving {0}", "Speichere {0}", "Guardando {0}" },
/* TextSenderSocketCreated */ {"Sender socket created", "Sender socket erstellt", "Socket para enviar datos creado" },
/* TextSerialNumberIs */ { "Serialnumber is {0}", "Seriennummer ist {0}", "Número de serie es {0}" },
/* TextSettingAccessory */ { "Setting accessory {0}/{1} to {2}", "Setze Zubehörartikel {0}/{1} auf {2}", "Ajusto accesorio {0}/{1} a {2}" },
/* TextSettingAccessoryWithProtocol */ { "Setting accessory {0}/{1}/{2} to {3}", "Setze Zubehörartikel {0}/{1}/{2} auf {3}", "Ajusto accesorio {0}/{1}/{2} a {3}" },
/* TextSettingDirection */ { "Changing loco {0} direction", "Wechsle die Richtung der Lok {0}", "Cambio la direccion de la locomotora {0}" },
/* TextSettingDirectionWithProtocol */ { "Setting loco {0}/{1} to direction {2}", "Setze Lok {0}/{1} in Richtung {2}", "Ajusto la direccion de la locomotora {0}/{1} a {2}" },
/* TextSettingFunction */ { "Setting function {0} of loco {1} to {2}", "Setze Funktion {0} der Lok {1} auf {2}", "Ajusto la function {0} de la locomotora {1} a {2}" },
/* TextSettingFunctionWithProtocol */ { "Setting function {0} of loco {1}/{2} to {3}", "Setze Funktion {0} der Lok {1}/{2} auf {3}", "Ajusto la function {0} de la locomotora {1}/{2} a {3}" },
/* TextSettingSpeed */ { "Setting loco {0} to speed {1}", "Setze Lok {0} auf Geschwindigkeit {1}", "Ajusto la velocidad de la locomotora {0} a {1}" },
/* TextSettingSpeedDirectionLight */ { "Setting loco {0} to speed {1} and direction {2} and light {3}", "Setze Geschwindigkeit {1}, Richtung {2} und Licht {3} der Lok {0}", "Ajusto velocidad a {1}, direccion {2} y luz {3} de la locomotora {0}" },
/* TextSettingSpeedWithProtocol */ { "Setting loco {0}/{1} to speed {2}", "Setze Lok {0}/{1} auf Geschwindigkeit {2}", "Ajusto la velocidad de la locomotora {0}/{1} a {2}" },
/* TextStoppingRailControl */ { "Stopping RailControl", "Beende RailControl", "Apaga RailControl" },
/* TextStoppingRequestedBySignal */ { "Stopping RailControl requested by signal {0}", "Beenden von RailControl angefordert mit Signal {0}", "Apagar RailControl pedido con señal {0}" },
/* TextStoppingRequestedByWebClient */ { "Stopping RailControl requested by webclient", "Beenden von RailControl angefordert von Webclient", "Apagar RailControl pedido del webclient" },
/* TextTerminatingReceiverThread */ { "Terminating receiver thread", "Beende Empfangs-Thread", "Apagando thread recibiendo" },
/* TextTerminatingSenderSocket */ { "Terminating sender socket", "Beende Sende Socket", "Apagando socket enviando" },
/* TextTooManyS88Modules */ { "Too many S88 modules configured: {0}. Max is {1}", "Zu viele S88 Module konfiguriert: {0}. Maximum ist {1}", "Demasiado módulos S88 configurado: {0}. El maximo es {1}" },
/* TextTrainIsToLong */ { "Train is to long for street {0}", "Zug ist zu lang für die Fahrstrasse {0}", "El tren es demasiado largo para el itinerario {0}" },
/* TextTrainIsToShort */ { "Train is to short for street {0}", "Zug ist zu kurz für die Fahrstrasse {0}", "El tren es demasiado corto para el itinerario {0}" },
/* TextTurningBoosterOn */ { "Turning booster off", "Dektiviere booster", "Apagando booster" },
/* TextTurningBoosterOn */ { "Turning booster on", "Aktiviere booster", "Encendiendo booster" },
/* TextUnableAddLayer1 */ { "Unable to add initial layer 1", "Unmöglich die Schicht 1 hinzuzufügen", "Imposible añadir capa 1" },
/* TextUnableCreateStorageHandler */ { "Unable to create storage handler", "Unmöglich den Speicher Handler zu erstellen", "Imposible crear manipulador de almacenamiento" },
/* TextUnableToBindSocketToPort */ { "Unable to bind connection to port {0}", "Binden der Verbindung an Port {0} fehlgeschlagen", "Vincular la conexión al puerto {0} imposible" },
/* TextUnableToBindUdpSocket */ { "Unable to bind UDP socket to address", "Nicht möglich den UDP socket an eine Adresse zu binden", "Imposible enlazar el UDP socket a la dirección" },
/* TextUnableToCreatUdpSocketForReceivingData */ { "Unable to create UDP socket to receive data from control", "Nicht möglich ein UDP socket zu erstellen um Daten von der Zentrale zu empfangen", "Imposible crear UDP socket para recibir datos del control" },
/* TextUnableToCreatUdpSocketForSendingData */ { "Unable to create UDP socket to send data to control", "Nicht möglich ein UDP socket zu erstellen um Daten an die Zentrale zu senden", "Imposible crear UDP socket para enviar datos al control" },
/* TextUnableToCreateSocket */ { "Unable to create socket", "Socket erstellen fehlgeschlagen", "Crear socket imposible" },
/* TextUnableToExecuteStreet */ { "Unable to execute street {0}", "Fahrstrasse {0} konnte nicht ausgeführt werden", "Ejecutar el itinerario {0} imposible" },
/* TextUnableToLockStreet */ { "Unable to lock street {0}", "Fahrstrasse {0} konnte nicht gesperrt werden", "Bloquear el itinerario {0} imposible" },
/* TextUnableToReceiveData */ { "Unable to receive data", "Nicht möglich Daten zu empfangen", "Imposible recibir datos" },
/* TextUnableToReserveStreet */ { "Unable to reserve street {0}", "Fahrstrasse {0} konnte nicht reserviert werden", "Reservar el itinerario {0} imposible" },
/* TextUnableToResolveAddress */ { "Unable to resolve address", "Adresse auflösen fehlgeschalgen", "Resolver la dirección imposible" },
/* TextUnableToSendDataToControl */ { "Unable to send data to control", "Nicht möglich daten an die Zentrale zu senden", "Imposible enviar datos al control" },
/* TextUnknownAccessory */ { "Unknown accessory", "Unbekannter Zubehörartikel", "Accesorio desconocido" },
/* TextUnknownControl */ { "Unknown control", "Unbekannte Zentrale", "Control desconocido" },
/* TextUnknownFeedback */ { "Unknown feedback", "Unbekannter Rückmelder", "Retroseñal desconocida" },
/* TextUnknownLoco */ { "Unknown locomotive", "Unbekannte Lokomotive", "Locomotora desconocida" },
/* TextUnknownSignal */ { "Unknown signal", "Unbekanntes Signal", "Señal desconocida" },
/* TextUnknownStreet */ { "Unknown street", "Unbekannte Fahrstrasse", "Itinerario desconocido" },
/* TextUnknownSwitch */ { "Unknown switch", "Unbekannte Weiche", "Desvío desconocido" },
/* TextUnknownTrack */ { "Unknown track", "Unbekanntes Gleis", "Vía desconocida" },
/* TextUnloadingControl */ { "Unloading control {0}: {1}", "Entlade Zentrale {0}: {1}", "Descargando control {0}: {1}" },
/* TextWebServerStarted */ { "Webserver started", "Webserver wurde gestartet", "Servidor web encendido" },
/* TextWebServerStopped */ { "Webserver stopped", "Webserver wurde beendet", "Servidor web apagado" },
/* TextWidthIs0 */ { "Width is zero", "Breite ist null", "Anchura está zero" },
/* TextZ21Black2012 */ { "black Z21, hardware 2012", "schwarze Z21, Hardware 2012", "Z21 negro, hardware 2012" },
/* TextZ21Black2013 */ { "black Z21, hardware 2013", "schwarze Z21, Hardware 2013", "Z21 negro, hardware 2013" },
/* TextZ21DoesNotUnderstand */ { "Z21 does not understand our command", "Z21 versteht under Kommando nicht", "Z21 no ha endendido nuestro comando" },
/* TextZ21SmartRail2012 */ { "SmartRail", "SmartRail", "SmartRail" },
/* TextZ21Start2016 */ { "z21 start", "z21 start", "z21 start" },
/* TextZ21Type */ { "Connected to a {0} with firmware version {1}", "Verbunden mit einer {0} mit firmware version", "Conectado a {0} con firmware versión {1}" },
/* TextZ21Unknown */ { "unknown Z21", "unbekannte Z21", "Z21 desconosido" },
/* TextZ21White2013 */ { "white z21", "weisse z21", "z21 blanco" },
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
