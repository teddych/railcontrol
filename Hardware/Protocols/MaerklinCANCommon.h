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

#pragma once

#include <atomic>
#include <map>
#include <mutex>

#include "DataModel/AccessoryBase.h"
#include "DataModel/LocoFunctions.h"
#include "Hardware/Capabilities.h"
#include "Hardware/HardwareInterface.h"
#include "Hardware/HardwareParams.h"
#include "Hardware/LocoCache.h"
#include "Hardware/Protocols/DecoderHandlerMfx.h"
#include "Logger/Logger.h"
#include "Utils/Integer.h"

// CAN protocol specification:  https://streaming.maerklin.de/public-media/cs2/cs2CAN-Protokoll-2_0.pdf
// Additions to CAN protocol:   http://www.mbernstein.de/modellbahn/can/bem.htm
// Additions to CAN protocol:   https://modellbahn.mahrer.net/technisches/maerklin_can/

namespace Hardware { namespace Protocols
{
	class MaerklinCANCommon : protected DecoderHandlerMfx
	{
		public:
			MaerklinCANCommon() = delete;
			MaerklinCANCommon(const MaerklinCANCommon&) = delete;
			MaerklinCANCommon& operator=(const MaerklinCANCommon&) = delete;

			void Booster(const BoosterState status);
			void LocoSpeed(const Protocol protocol, const Address address, const Speed speed);
			void LocoOrientation(const Protocol protocol, const Address address, const Orientation orientation);

			void LocoFunction(const Protocol protocol,
				const Address address,
				const DataModel::LocoFunctionNr function,
				const DataModel::LocoFunctionState on);

			void Accessory(const Protocol protocol,
				const Address address,
				const DataModel::AccessoryState state,
				const bool on);

			void ProgramReadMfx(const Address address);
			void ProgramRead(const ProgramMode mode, const Address address, const CvNumber cv, const uint8_t count = 1);
			void ProgramWrite(const ProgramMode mode, const Address address, const CvNumber cv, const CvValue value);

			static inline DataModel::LocoFunctionIcon MapLocoFunctionCs2ToRailControl(const uint8_t input)
			{
				return LocoFunctionMapCs2ToRailControl[input];
			}

			static inline uint8_t MapLocofunctionRailControlToCs2(const DataModel::LocoFunctionNr nr,
				const DataModel::LocoFunctionIcon railcontrolIcon)
			{
				LocoFunctionCs2Icon icon = LocoFunctionMapRailControlToCs2[railcontrolIcon];
				if (icon == LocoFunctionCs2IconDefault)
				{
					icon = static_cast<LocoFunctionCs2Icon>(icon + nr);
				}
				return icon;
			}

		protected:
			void ReadMfxCVValue(const Address sid, const CvNumber cv, const CvIndex index, const uint8_t count) override;

			inline MaerklinCANCommon(const std::string& uid,
				ControlID controlID,
				Manager* manager,
				const bool isCs2MainEmulation,
				Logger::Logger* logger)
			:	DecoderHandlerMfx(manager, controlID, logger),
				run(true),
				uid(Utils::Integer::HexToInteger(uid)),
				hash(CalcHash(this->uid)),
				mfxCounter(0),
				hasCs2Main(false),
				isCs2MainEmulation(isCs2MainEmulation)
			{
				logger->Debug(Languages::TextMaerklinCanStarting, uid, Utils::Integer::IntegerToHex(hash, 4), Languages::GetOnOff(isCs2MainEmulation));
			}

			void Init();
			void Stop();

			virtual ~MaerklinCANCommon();

			void Parse(const unsigned char* buffer);

			void RequestLoks();

			void ReceiverInternal();

			virtual void Receiver() = 0;

			virtual void CacheSave(LocoCacheEntry& entry) = 0;

			virtual void CacheSave(LocoCacheEntry& entry, const std::string& oldMatchKey) = 0;

			virtual LocoID CacheDelete(const std::string& matchKey) = 0;

			virtual void CacheUpdateSlaves() = 0;

			void IncrementMfxCounter();


			static const unsigned char CANCommandBufferLength = 13;

			std::atomic_bool run;

		private:
			typedef uint16_t MfxCounter;

			enum CanCommand : unsigned char
			{
				CanCommandSystem            = 0x00,
				CanCommandDiscovery         = 0x01,
				CanCommandMfxBind           = 0x02,
				CanCommandMfxVerify         = 0x03,
				CanCommandLocoSpeed         = 0x04,
				CanCommandLocoDirection     = 0x05,
				CanCommandLocoFunction      = 0x06,
				CanCommandReadConfig        = 0x07,
				CanCommandWriteConfig       = 0x08,
				CanCommandAccessory         = 0x0B,
				CanCommandS88Event          = 0x11,
				CanCommandPing              = 0x18,
				CanCommandBootloaderCan     = 0x1B,
				CanCommandRequestConfigData = 0x20,
				CanCommandConfigData        = 0x21,
				CanCommandHello             = 0x42
			};

			enum CanSubCommand : unsigned char
			{
				CanSubCommandStop       = 0x00,
				CanSubCommandGo         = 0x01,
				CanSubCommandMfxCounter = 0x09
			};

			enum CanResponse : unsigned char
			{
				CanResponseCommand  = 0x00,
				CanResponseResponse = 0x01
			};

			enum CanDeviceType : uint16_t
			{
				CanDeviceGfp            = 0x0000,
				CanDeviceGleisbox       = 0x0010,
				CanDeviceGleisbox_2     = 0x0011, // undocumented
				CanDeviceConnect6021    = 0x0020,
				CanDeviceMs2            = 0x0030,
				CanDeviceMs2_2          = 0x0032, // undocumented
				CanDeviceMs2_3          = 0x0033, // undocumented
				CanDeviceMs2_4          = 0x0034, // undocumented
				CanDeviceLinkS88        = 0x0040, // undocumented
				CanDeviceCs2Secondary_2 = 0xeeee, // undocumented
				CanDeviceWireless       = 0xffe0,
				CanDeviceCs2Secondary   = 0xfff0, // undocumented
				CanDeviceCs2Main        = 0xffff
			};

			enum LocoFunctionCs2Icon : uint8_t
			{
				LocoFunctionCs2IconNone = 0,
				LocoFunctionCs2IconLight,
				LocoFunctionCs2IconInteriorLight1,
				LocoFunctionCs2IconBackLightForward,
				LocoFunctionCs2IconHeadlightHighBeamForward,
				LocoFunctionCs2IconSoundGeneral,
				LocoFunctionCs2IconPanto12,
				LocoFunctionCs2IconSmokeGenerator,
				LocoFunctionCs2IconShuntingMode,
				LocoFunctionCs2IconTelex12,
				LocoFunctionCs2IconHorn1,
				LocoFunctionCs2IconWhistle1,
				LocoFunctionCs2IconWhistle2,
				LocoFunctionCs2IconBell,
				LocoFunctionCs2IconLeftRight,
				LocoFunctionCs2IconUpDown1,
				LocoFunctionCs2IconTurnLeft,
				LocoFunctionCs2IconUpDown2,
				LocoFunctionCs2IconInertia,
				LocoFunctionCs2IconFan2,
				LocoFunctionCs2IconBreak1,
				LocoFunctionCs2IconGearBox,
				LocoFunctionCs2IconGenerator,
				LocoFunctionCs2IconRunning1,
				LocoFunctionCs2IconEngine1,
				LocoFunctionCs2IconStationAnnouncement1,
				LocoFunctionCs2IconShovelCoal,
				LocoFunctionCs2IconCloseDoor,
				LocoFunctionCs2IconOpenDoor,
				LocoFunctionCs2IconFan1,
				LocoFunctionCs2IconFan,
				LocoFunctionCs2IconFireBox,
				LocoFunctionCs2IconInteriorLight2,
				LocoFunctionCs2IconTableLight3,
				LocoFunctionCs2IconTableLight2,
				LocoFunctionCs2IconTableLight1,
				LocoFunctionCs2IconShakingRust,
				LocoFunctionCs2IconRailJoint,
				LocoFunctionCs2IconLocomotiveNumberIndicator,
				LocoFunctionCs2IconMusic1,
				LocoFunctionCs2IconTrainDestinationIndicator,
				LocoFunctionCs2IconCabLight2,
				LocoFunctionCs2IconCabLight1,
				LocoFunctionCs2IconCoupler,
				LocoFunctionCs2IconBufferPush,
				LocoFunctionCs2IconStationAnnouncement3,
				LocoFunctionCs2IconCraneHook,
				LocoFunctionCs2IconBlinkingLight,
				LocoFunctionCs2IconCabLight12,
				LocoFunctionCs2IconCompressedAir,
				LocoFunctionCs2IconDefault,

				LocoFunctionCs2IconTelex2 = 82,
				LocoFunctionCs2IconTelex1,
				LocoFunctionCs2IconPanto2,
				LocoFunctionCs2IconPanto1,
				LocoFunctionCs2IconHeadlightReverse,
				LocoFunctionCs2IconHeadlightForward,
				LocoFunctionCs2IconUp2,
				LocoFunctionCs2IconFan3,
				LocoFunctionCs2IconEngineLight,
				LocoFunctionCs2IconSteamBlowOut,
				LocoFunctionCs2IconSteamBlow,
				LocoFunctionCs2IconCrane,
				LocoFunctionCs2IconUp1,
				LocoFunctionCs2IconDown,
				LocoFunctionCs2IconLeft,
				LocoFunctionCs2IconRight,
				LocoFunctionCs2IconTurnRight,
				LocoFunctionCs2IconMagnet,

				LocoFunctionCs2IconPanto = 101,

				LocoFunctionCs2IconRadio = 103,
				LocoFunctionCs2IconStationAnnouncement2,
				LocoFunctionCs2IconBackLightReverse,
				LocoFunctionCs2IconAirPump,
				LocoFunctionCs2IconSpeak,
				LocoFunctionCs2IconEngine2,
				LocoFunctionCs2IconNoSound,
				LocoFunctionCs2IconStairsLight,
				LocoFunctionCs2IconFillWater,
				LocoFunctionCs2IconBreak2
			};


			typedef unsigned char CanPrio;
			typedef unsigned char CanLength;
			typedef uint32_t CanUid;
			typedef uint16_t CanHash;
			typedef uint16_t CanFileCrc;

			struct CanFile
			{
				CanHash hash;
				unsigned char* data;
				unsigned char* dataPointer;
				size_t dataSize;
				CanFileCrc crc;
				size_t crcSize;
			};

			enum ConfigRequestType : uint8_t
			{
				ConfigRequestLocoNames,
				ConfigRequestLocoInfo,
				ConfigRequestVersion
			};

			struct ConfigRequest
			{
				ConfigRequestType type;
				std::string data;
			};

			void CreateCommandHeader(unsigned char* const buffer,
				const CanCommand command,
				const CanResponse response,
				const CanLength length);

			inline void ParseAddressProtocol(const unsigned char* const buffer,
				Address& address,
				Protocol& protocol,
				LocoType& type)
			{
				Address input = ParseAddress(buffer);
				ParseAddressProtocol(input, address, protocol, type);
			}

			static void ParseAddressProtocol(const Address input,
				Address& address,
				Protocol& protocol,
				LocoType& type);

			static inline CanPrio ParsePrio(const unsigned char* const buffer)
			{
				return buffer[0] >> 1;
			}

			static inline CanCommand ParseCommand(const unsigned char* const buffer)
			{
				return static_cast<CanCommand>((buffer[0] << 7) | (buffer[1] >> 1));
			}

			static inline CanSubCommand ParseSubCommand(const unsigned char* const buffer)
			{
				return static_cast<CanSubCommand>(buffer[9]);
			}

			static inline CanResponse ParseResponse(const unsigned char* const buffer)
			{
				return static_cast<CanResponse>(buffer[1] & 0x01);
			}

			static inline CanLength ParseLength(const unsigned char* const buffer)
			{
				return buffer[4];
			}

			static inline Address ParseAddress(const unsigned char* const buffer)
			{
				return static_cast<Address>(Utils::Integer::DataBigEndianToInt(buffer + 5));
			}

			static inline void ParseFeedbackPin(const unsigned char* const buffer,
				FeedbackDevice& device,
				FeedbackBus& bus,
				FeedbackPin& pin)
			{
				pin = static_cast<FeedbackPin>(Utils::Integer::DataBigEndianToInt(buffer + 5));
				device = (pin >> 16) & 0x000000FF;
				bus = 0;
				pin &= 0x0000FFFF;
				while (pin > 1000)
				{
					++bus;
					pin -= 1000;
				}
			}

			static inline CanHash ParseHash(const unsigned char* const buffer)
			{
				return Utils::Integer::DataBigEndianToShort(buffer + 2);
			}

			static inline CanUid ParseUid(const unsigned char* const buffer)
			{
				return Utils::Integer::DataBigEndianToInt(buffer + 5);
			}

			static inline bool MaerklinCANDiscoveryIsMfx(const uint8_t protocol)
			{
				return protocol <= 32 || (protocol >= 64 && protocol <= 96);
			}

			void ParseCommandSystem(const unsigned char* const buffer);
			void ParseResponseDiscovery(const unsigned char* const buffer);
			void ParseResponseMfxBind(const unsigned char* const buffer);
			void ParseResponseMfxVerify(const unsigned char* const buffer);
			void ParseResponseLocoSpeed(const unsigned char* const buffer);
			void ParseResponseLocoDirection(const unsigned char* const buffer);
			void ParseResponseLocoFunction(const unsigned char* const buffer);
			void ParseResponseAccessory(const unsigned char* const buffer);
			void ParseCommandPing(const unsigned char* const buffer);
			void ParseCommandBootloaderCan(const unsigned char* const buffer);

			void ParseCommandRequestConfigData(const unsigned char* const buffer);

			std::string GetMs2LocoNames(const size_t offset,
				const size_t count) const;

			std::string GetMs2LocoInfo(const std::string& name) const;

			void SendCompressedFile(const std::string& dataPlain,
				const unsigned char* fileName = nullptr);

			void SendUncompressedFile(const std::string& data,
				const unsigned char* fileName,
				const CanHash destinationHash);

			void SendFile(const std::vector<unsigned char>& data,
				const uint32_t dataSize,
				const unsigned char* fileName,
				const CanHash* destinationHash = nullptr);

			void DeleteCanFile(CanFile* canFile);

			struct CanFile* GetCanFile(const CanHash hash);

			void ParseCommandConfigData(const unsigned char* const buffer);

			void ParseCommandConfigDataFirst(const unsigned char* const buffer,
				struct CanFile* canFile);

			void ParseCommandConfigDataNext(const unsigned char* const buffer,
				struct CanFile* canFile);

			void ParseResponseS88Event(const unsigned char* const buffer);
			void ParseResponseReadConfig(const unsigned char* const buffer);
			void ParseResponsePing(const unsigned char* const buffer);
			void ParseResponseBootloaderCan(const unsigned char* const buffer);

			bool ParseCs2FileKeyValue(const std::string& line, std::string& key, std::string& value);
			bool ParseCs2FileSubkeyValue(const std::string& line, std::string& key, std::string& value);
			void ParseCs2FileLocomotiveFunction(std::deque<std::string>& lines, LocoCacheEntry& cacheEntry);
			void ParseCs2FileLocomotiveTraktion(std::deque<std::string>& lines, LocoCacheEntry& cacheEntry);
			void ParseCs2FileLocomotive(std::deque<std::string>& lines);
			void ParseCs2FileLocomotivesSession(std::deque<std::string>& lines);
			void ParseCs2FileLocomotivesVersion(std::deque<std::string>& lines);
			void ParseCs2FileLocomotives(std::deque<std::string>& lines);
			void ParseCs2File(std::deque<std::string>& lines);

			static const char* DeviceTypeAsString(const uint16_t deviceType);

			static CanFileCrc CalcCrc(const unsigned char *data, const size_t length);

			static CanHash CalcHash(const CanUid uid);
			void GenerateUidHash();

			void CreateLocalIDLoco(unsigned char* buffer, const Protocol protocol, const Address address) const;
			void CreateLocalIDAccessory(unsigned char* buffer, const Protocol protocol, const Address address) const;

			void Wait(const unsigned int duration) const;
			void PingSender();
			void SendPing();
			void SendBootloaderCan();
			void VerifyAllMfxLocos();
			Address RegisterMfxDiscovery(const MfxUid mfxUid);
			bool IsPendingMfxBinding(const MfxUid mfxUid, const Address sid) const;
			void SendMfxBind(const MfxUid mfxUid, const Address sid);
			void SendMfxVerify(const MfxUid mfxUid, const Address sid);

			inline void SendInternal(const unsigned char* buffer)
			{
				logger->HexOut(buffer, 5 + ParseLength(buffer));
				Send(buffer);
			}
			virtual void Send(const unsigned char* buffer) = 0;

			CanUid uid;
			CanHash hash;
			MfxCounter mfxCounter;
			std::atomic_bool hasCs2Main; // true if a CS2 main station is detected on the CAN bus
			const bool isCs2MainEmulation; // true if this is a CS2 main station emulation (e.g. for a CS2 secondary station or an MS2)
			std::thread receiverThread;
			std::thread pingThread;

			std::vector<struct CanFile> canFiles;
			std::map<CanHash,ConfigRequest> configRequests;
			std::map<MfxUid,Address> pendingMfxBindings;
			std::mutex pendingMfxVerificationsMutex;
			std::map<MfxUid,Address> pendingMfxVerifications;

			static const uint8_t EmulateCS2VersionMajor = 4;
			static const uint8_t EmulateCS2VersionMinor = 3;

			static const uint8_t MaxNrOfCs2FunctionIcons = 128;
			static const DataModel::LocoFunctionIcon LocoFunctionMapCs2ToRailControl[MaxNrOfCs2FunctionIcons];
			static const MaerklinCANCommon::LocoFunctionCs2Icon LocoFunctionMapRailControlToCs2[DataModel::MaxLocoFunctionIcons];
	};
}} // namespace
