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

#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>		//close;
#include <vector>

#include "Logger/Logger.h"
#include "Network/Serial.h"

static volatile unsigned int runSniffer;

void stopSignal(int signo)
{
	runSniffer = false;
}

int main (int argc, char* argv[])
{
	signal(SIGINT, stopSignal);
	signal(SIGTERM, stopSignal);

	runSniffer = true;
	Logger::Logger* logger = Logger::Logger::GetLogger("CAN bus sniffer");
	logger->AddConsoleLogger();
	logger->SetLogLevel(Logger::Logger::LevelDebug);

	logger->Debug("Starting CAN bus sniffer");

	Network::Serial serial(logger, "/dev/ttyUSB0", B500000, 8, 'N', 1);

	do
	{
		unsigned char buffer[13];
		int ret = serial.ReceiveExact(buffer, sizeof(buffer));
		if (ret == 0)
		{
			// do nothing
		}
		else if (ret != sizeof(buffer))
		{
			logger->Debug("Received != 13 bytes");
		}
		else
		{
			logger->Hex(buffer, sizeof(buffer));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} while (runSniffer);

	logger->Debug("Terminating CAN bus sniffer");
	return 0;
}

