#pragma once

#include "WasabiGame/Networking/Data.hpp"
#include <Wasabi/Core/WMath.hpp>

#include <unordered_map>
#include <string>
#include <functional>
#include <cstdint>
#include <algorithm>


namespace RollTheBall {

	enum NetworkUpdateTypeEnum {
		UPDATE_TYPE_ERROR = 1,
		UPDATE_TYPE_LOGIN = 2,
		UPDATE_TYPE_LOAD_UNIT = 10,
		UPDATE_TYPE_UNLOAD_UNIT = 11,
		UPDATE_TYPE_WHOIS_UNIT = 12,
		UPDATE_TYPE_SET_UNIT_PROPS = 13,
	};

	namespace UpdateBuilders {

		// Server --> Client: sends error
		void Error(WasabiGame::NetworkUpdate& output, const char* error);
		bool ReadErrorPacket(WasabiGame::NetworkUpdate& input, char* error);

		// Server <-- Client: send login credentials
		void Login(WasabiGame::NetworkUpdate& output, const char* account, const char* password);
		bool ReadLoginPacket(WasabiGame::NetworkUpdate& input, WasabiGame::ClientIdentity& identity);

		// Server --> Client: command the client to load a unit
		void LoadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawn);
		bool ReadLoadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId, WVector3* spawn);

		// Server --> Client: command the client to unload a unit
		void UnloadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId);
		bool ReadUnloadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId);

		// Server <-- Client: ask the server to send the description of a unit
		void WhoIsUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId);
		bool ReadWhoIsUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId);

		// Server <-> Client: Commands the receiver to set certain properties of a unit
		void SetUnitProps(WasabiGame::NetworkUpdate& output, uint32_t unitId, std::function<void(std::string, void*, uint16_t)>* fillFunc);
		bool ReadSetUnitPropsPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId, std::function<void(std::string, void*, uint16_t)> readFunc);

	};

};

