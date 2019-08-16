#pragma once

#include "RollTheBall/Networking/Data.hpp"
#include <Wasabi/Core/WMath.h>

namespace RTBNet {

	enum UpdateTypeEnum {
		UPDATE_TYPE_ERROR = 1,
		UPDATE_TYPE_LOGIN = 2,
		UPDATE_TYPE_LOAD_UNIT = 10,
		UPDATE_TYPE_UNLOAD_UNIT = 11,
	};

	namespace UpdateBuilders {

		void Error(RPGNet::NetworkUpdate& output, const char* error);
		bool ReadErrorPacket(RPGNet::NetworkUpdate& input, char* error);

		void Login(RPGNet::NetworkUpdate& output, const char* account, const char* password);
		bool ReadLoginPacket(RPGNet::NetworkUpdate& input, RPGNet::ClientIdentity& identity);

		void LoadUnit(RPGNet::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawn);
		bool ReadLoadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId, WVector3* spawn);

		void UnloadUnit(RPGNet::NetworkUpdate& output, uint32_t unitId);
		bool ReadUnloadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId);

	};

};

