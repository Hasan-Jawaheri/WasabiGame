#include "RollTheBall/Networking/Protocol.hpp"


void RTBNet::UpdateBuilders::Error(RPGNet::NetworkUpdate& output, const char* error) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_ERROR;
	output.dataSize = min(strlen(error), sizeof(RPGNet::NetworkUpdate::data));
	memcpy(output.data, error, output.dataSize);
}

bool RTBNet::UpdateBuilders::ReadErrorPacket(RPGNet::NetworkUpdate& input, char* error) {
	memcpy(error, input.data, input.dataSize);
	error[input.dataSize] = '\0';
	return true;
}

void RTBNet::UpdateBuilders::Login(RPGNet::NetworkUpdate& output, const char* account, const char* password) {
	RPGNet::ClientIdentity identity;
	strcpy(identity.accountName, account);
	strcpy(identity.passwordHash, password);

	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_LOGIN;
	output.dataSize = sizeof(RPGNet::ClientIdentity);
	memcpy(output.data, &identity, output.dataSize);
}

bool RTBNet::UpdateBuilders::ReadLoginPacket(RPGNet::NetworkUpdate& input, RPGNet::ClientIdentity& identity) {
	if (input.dataSize != sizeof(RPGNet::ClientIdentity))
		return false;
	memcpy(&identity, input.data, input.dataSize);
	return true;
}

void RTBNet::UpdateBuilders::LoadUnit(RPGNet::NetworkUpdate& output, uint32_t unitType, uint32_t unitId) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_LOAD_UNIT;
	output.purpose = unitType;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RTBNet::UpdateBuilders::ReadLoadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId) {
	*unitType = input.purpose;
	*unitId = input.targetId;
	return true;
}

void RTBNet::UpdateBuilders::UnloadUnit(RPGNet::NetworkUpdate& output, uint32_t unitId) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_UNLOAD_UNIT;
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RTBNet::UpdateBuilders::ReadUnloadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId) {
	*unitId = input.targetId;
	return true;
}
