#include "RollTheBall/Networking/Protocol.hpp"


void RTBNet::UpdateBuilders::Error(RPGNet::NetworkUpdate& output, const char* error) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_ERROR;
	output.dataSize = std::min(strlen(error), sizeof(RPGNet::NetworkUpdate::data));
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

void RTBNet::UpdateBuilders::LoadUnit(RPGNet::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawnPos) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_LOAD_UNIT;
	output.purpose = unitType;
	output.targetId = unitId;
	output.dataSize = sizeof(WVector3);
	memcpy(output.data, &spawnPos, output.dataSize);
}

bool RTBNet::UpdateBuilders::ReadLoadUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId, WVector3* spawnPos) {
	if (input.dataSize != sizeof(WVector3))
		return false;
	*unitType = input.purpose;
	*unitId = input.targetId;
	memcpy((void*)spawnPos, input.data, input.dataSize);
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

void RTBNet::UpdateBuilders::WhoIsUnit(RPGNet::NetworkUpdate& output, uint32_t unitId) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT;
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RTBNet::UpdateBuilders::ReadWhoIsUnitPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId) {
	*unitId = input.targetId;
	return true;
}

void RTBNet::UpdateBuilders::SetUnitProps(RPGNet::NetworkUpdate& output, uint32_t unitId, std::function<void(std::string, void*, uint16_t)>* fillFunc) {
	output.type = RTBNet::UpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS;
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
	*fillFunc = [&output](std::string name, void* data, uint16_t size) {
		uint8_t nameLen = name.length();
		uint16_t nsize = htons(size);
		memcpy(output.data + output.dataSize + 0, &nameLen, 1);
		memcpy(output.data + output.dataSize + 1, &nsize, 2);
		memcpy(output.data + output.dataSize + 3, name.c_str(), nameLen);
		memcpy(output.data + output.dataSize + 3 + nameLen, data, size);
		output.dataSize += 3 + nameLen + size;
	};
}

bool RTBNet::UpdateBuilders::ReadSetUnitPropsPacket(RPGNet::NetworkUpdate& input, uint32_t* unitId, std::function<void(std::string, void*, uint16_t)> readFunc) {
	*unitId = input.targetId;
	size_t pos = 0;
	char tmpName[256];
	while (pos < input.dataSize) {
		if (input.dataSize < pos + 3)
			return false;
		uint8_t nameLen;
		uint16_t size;
		memcpy(&nameLen, input.data + pos + 0, 1);
		memcpy(&size, input.data + pos + 1, 2);
		size = ntohs(size);
		if (input.dataSize < pos + 3 + nameLen + size)
			return false;
		memcpy(tmpName, input.data + pos + 3, nameLen);
		readFunc(std::string(tmpName, nameLen), input.data + pos + 3 + nameLen, size);
		pos += 3 + nameLen + size;
	}
	return true;
}
