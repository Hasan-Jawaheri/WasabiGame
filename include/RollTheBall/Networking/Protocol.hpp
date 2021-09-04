#pragma once

#include "WasabiGame/Networking/Data.hpp"
#include <Wasabi/Core/WMath.hpp>

#include <unordered_map>
#include <string>
#include <functional>
#include <cstdint>
#include <algorithm>


namespace RollTheBall {

	enum class NetworkUpdateTypeEnum : uint32_t {
		UPDATE_TYPE_ERROR = 1,
		UPDATE_TYPE_SET_CLIENT_ID = 2,
		UPDATE_TYPE_IDENTIFY_UDP_CLIENT = 3,
		UPDATE_TYPE_UDP_CLIENT_IDENTIFIED = 4,
		UPDATE_TYPE_LOGIN = 5,
		UPDATE_TYPE_LOGIN_STATUS = 6,
		UPDATE_TYPE_SELECT_GAME_MODE = 7,
		UPDATE_TYPE_LOAD_UNIT = 10,
		UPDATE_TYPE_UNLOAD_UNIT = 11,
		UPDATE_TYPE_WHOIS_UNIT = 12,
		UPDATE_TYPE_SET_UNIT_PROPS = 13,
		UPDATE_TYPE_LOAD_MAP = 14,
	};

	namespace UpdateBuilders {

		// (TCP) Server --> Client: sends error
		void Error(WasabiGame::NetworkUpdate& output, const char* error);
		bool ReadErrorPacket(WasabiGame::NetworkUpdate& input, char* error);

		// (TCP) Server --> Client: send client id
		void SetClientId(WasabiGame::NetworkUpdate& output, uint32_t clientId);
		bool ReadSetClientIdPacket(WasabiGame::NetworkUpdate& input, uint32_t* clientId);

		// (UDP) Server <-- Client: send client's clientId in a UDP packet to identify the UDP source
		void IdentifyUDPClient(WasabiGame::NetworkUpdate& output, uint32_t clientId);
		bool ReadIdentifyUDPClientPacket(WasabiGame::NetworkUpdate& input, uint32_t* clientId);

		// (TCP) Server --> Client: acknowledge that the server figured out the client's UDP stream
		void UDPClientIdentified(WasabiGame::NetworkUpdate& output);
		bool ReadUDPClientIdentifiedPacket(WasabiGame::NetworkUpdate& input);

		// (TCP) Server <-- Client: send login credentials
		void Login(WasabiGame::NetworkUpdate& output, const char* account, const char* password);
		bool ReadLoginPacket(WasabiGame::NetworkUpdate& input, WasabiGame::ClientIdentity& identity);

		// (TCP) Server --> Client: login success/failure
		void LoginStatus(WasabiGame::NetworkUpdate& output, bool success);
		bool ReadLoginStatusPacket(WasabiGame::NetworkUpdate& input, bool& success);

		// (TCP) Server <-- Client: select game mode to play
		void SelectGameMode(WasabiGame::NetworkUpdate& output, uint32_t gameMode);
		bool ReadSelectGameModePacket(WasabiGame::NetworkUpdate& input, uint32_t& gameMode);

		// (TCP) Server --> Client: command the client to load a unit
		void LoadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawn);
		bool ReadLoadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId, WVector3* spawn);

		// (TCP) Server --> Client: command the client to unload a unit
		void UnloadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId);
		bool ReadUnloadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId);

		// (TCP) Server <-- Client: ask the server to send the description of a unit
		void WhoIsUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId);
		bool ReadWhoIsUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId);

		// (UDP or TCP) Server <-> Client: Commands the receiver to set certain properties of a unit
		void SetUnitProps(WasabiGame::NetworkUpdate& output, uint32_t unitId, std::function<void(std::string, void*, uint16_t)>* fillFunc);
		bool ReadSetUnitPropsPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId, std::function<void(std::string, void*, uint16_t)> readFunc);

		// (TCP) Server --> Client: Commands the client to load a map
		void LoadMap(WasabiGame::NetworkUpdate& output, uint32_t map);
		bool ReadLoadMapPacket(WasabiGame::NetworkUpdate& input, uint32_t* map);
	};

};

