#include "RollTheBall/Networking/Protocol.hpp"


void RollTheBall::UpdateBuilders::Error(WasabiGame::NetworkUpdate& output, const char* error) {
	output.type = RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_ERROR;
	output.dataSize = std::min(strlen(error), sizeof(WasabiGame::NetworkUpdate::data));
	memcpy(output.data, error, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadErrorPacket(WasabiGame::NetworkUpdate& input, char* error) {
	memcpy(error, input.data, input.dataSize);
	error[input.dataSize] = '\0';
	return true;
}

void RollTheBall::UpdateBuilders::Login(WasabiGame::NetworkUpdate& output, const char* account, const char* password) {
	WasabiGame::ClientIdentity identity;
	strcpy(identity.accountName, account);
	strcpy(identity.passwordHash, password);

	output.type = RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN;
	output.dataSize = sizeof(WasabiGame::ClientIdentity);
	memcpy(output.data, &identity, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadLoginPacket(WasabiGame::NetworkUpdate& input, WasabiGame::ClientIdentity& identity) {
	if (input.dataSize != sizeof(WasabiGame::ClientIdentity))
		return false;
	memcpy(&identity, input.data, input.dataSize);
	return true;
}

void RollTheBall::UpdateBuilders::LoadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawnPos) {
	output.type = RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOAD_UNIT;
	output.purpose = unitType;
	output.targetId = unitId;
	output.dataSize = sizeof(WVector3);
	memcpy(output.data, &spawnPos, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadLoadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitType, uint32_t* unitId, WVector3* spawnPos) {
	if (input.dataSize != sizeof(WVector3))
		return false;
	*unitType = input.purpose;
	*unitId = input.targetId;
	memcpy((void*)spawnPos, input.data, input.dataSize);
	return true;
}

void RollTheBall::UpdateBuilders::UnloadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId) {
	output.type = RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_UNLOAD_UNIT;
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RollTheBall::UpdateBuilders::ReadUnloadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId) {
	*unitId = input.targetId;
	return true;
}

void RollTheBall::UpdateBuilders::WhoIsUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId) {
	output.type = RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT;
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RollTheBall::UpdateBuilders::ReadWhoIsUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId) {
	*unitId = input.targetId;
	return true;
}

void RollTheBall::UpdateBuilders::SetUnitProps(WasabiGame::NetworkUpdate& output, uint32_t unitId, std::function<void(std::string, void*, uint16_t)>* fillFunc) {
	output.type = RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS;
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

bool RollTheBall::UpdateBuilders::ReadSetUnitPropsPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId, std::function<void(std::string, void*, uint16_t)> readFunc) {
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
