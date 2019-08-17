#pragma once

#include "RollTheBall/Networking/Data.hpp"
#include <Wasabi/Core/WMath.h>

#include <unordered_map>
#include <string>
#include <functional>
#include <cstdint>

namespace RTBNet {

	enum UpdateTypeEnum {
		UPDATE_TYPE_ERROR = 1,
		UPDATE_TYPE_LOGIN = 2,
		UPDATE_TYPE_LOAD_UNIT = 10,
		UPDATE_TYPE_UNLOAD_UNIT = 11,
		UPDATE_TYPE_WHOIS_UNIT = 12,
		UPDATE_TYPE_SET_UNIT_PROPS = 13,
	};

	namespace UpdateBuilders {

		// Server --> Client: sends error
		void Error(RPGNet::NetworkUpdate& output, const char* error);
		bool ReadErrorPacket(RPGNet::NetworkUpdate& input, char* error);

		// Server <-- Client: send login credentials
		void Login(RPGNet::NetworkUpdate& output, const char* account, const char* password);
		bool ReadLoginPacket(RPGNet::NetworkUpdate& input, RPGNet::ClientIdentity& identity);

		// Server --> Client: command the client to load a unit
		void LoadUnit(RPGNet::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawn);
		bool ReadLoadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId, WVector3* spawn);

		// Server --> Client: command the client to unload a unit
		void UnloadUnit(RPGNet::NetworkUpdate& output, uint32_t unitId);
		bool ReadUnloadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId);

		// Server <-- Client: ask the server to send the description of a unit
		void WhoIsUnit(RPGNet::NetworkUpdate& output, uint32_t unitId);
		bool ReadWhoIsUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId);

		// Server <-> Client: Commands the receiver to set certain properties of a unit
		void SetUnitProps(RPGNet::NetworkUpdate& output, uint32_t unitId, std::function<void(std::string, void*, uint16_t)>* fillFunc);
		bool ReadSetUnitPropsPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId, std::function<void(std::string, void*, uint16_t)> readFunc);

	};

};

