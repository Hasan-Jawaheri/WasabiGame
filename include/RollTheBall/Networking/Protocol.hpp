#pragma once

#include "RollTheBall/Networking/Data.hpp"
#include "RollTheBall/Networking/Data.hpp"

namespace RTBNet {

	enum UpdateTypeEnum {
		UPDATE_TYPE_ERROR = 1,
		UPDATE_TYPE_LOGIN = 2,
		UPDATE_TYPE_UNIT = 10,
	};

	namespace UpdateBuilders {

		void Error(RPGNet::NetworkUpdate& output, const char* error);
		bool ReadErrorPacket(RPGNet::NetworkUpdate& input, char* error);

		void Login(RPGNet::NetworkUpdate& output, const char* account, const char* password);
		bool ReadLoginPacket(RPGNet::NetworkUpdate& input, RPGNet::ClientIdentity& identity);

	};

};

