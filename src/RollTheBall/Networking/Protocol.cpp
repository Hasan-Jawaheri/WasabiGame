#include "RollTheBall/Networking/Protocol.hpp"


void RollTheBall::UpdateBuilders::Error(WasabiGame::NetworkUpdate& output, const char* error) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_ERROR);
	output.dataSize = std::min(strlen(error), sizeof(WasabiGame::NetworkUpdate::data));
	memcpy(output.data, error, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadErrorPacket(WasabiGame::NetworkUpdate& input, char* error) {
	memcpy(error, input.data, input.dataSize);
	error[input.dataSize] = '\0';
	return true;
}

void RollTheBall::UpdateBuilders::SetClientId(WasabiGame::NetworkUpdate& output, uint32_t clientId) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_CLIENT_ID);
	output.dataSize = 0;
	output.targetId = clientId;
}

bool RollTheBall::UpdateBuilders::ReadSetClientIdPacket(WasabiGame::NetworkUpdate& input, uint32_t* clientId) {
	*clientId = input.targetId;
	return true;
}

void RollTheBall::UpdateBuilders::IdentifyUDPClient(WasabiGame::NetworkUpdate& output, uint32_t clientId) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_IDENTIFY_UDP_CLIENT);
	output.dataSize = 0;
	output.targetId = clientId;
}

bool RollTheBall::UpdateBuilders::ReadIdentifyUDPClientPacket(WasabiGame::NetworkUpdate& input, uint32_t* clientId) {
	*clientId = input.targetId;
	return true;
}

void RollTheBall::UpdateBuilders::UDPClientIdentified(WasabiGame::NetworkUpdate& output) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_UDP_CLIENT_IDENTIFIED);
	output.dataSize = 0;
}

bool RollTheBall::UpdateBuilders::ReadUDPClientIdentifiedPacket(WasabiGame::NetworkUpdate& input) {
	// no-op
	return true;
}

void RollTheBall::UpdateBuilders::Login(WasabiGame::NetworkUpdate& output, const char* account, const char* password) {
	WasabiGame::ClientIdentity identity;
	strcpy(identity.accountName, account);
	strcpy(identity.passwordHash, password);

	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN);
	output.dataSize = sizeof(WasabiGame::ClientIdentity);
	memcpy(output.data, &identity, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadLoginPacket(WasabiGame::NetworkUpdate& input, WasabiGame::ClientIdentity& identity) {
	if (input.dataSize != sizeof(WasabiGame::ClientIdentity))
		return false;
	memcpy(&identity, input.data, input.dataSize);
	return true;
}

void RollTheBall::UpdateBuilders::LoginStatus(WasabiGame::NetworkUpdate& output, bool success) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN_STATUS);
	output.dataSize = 1;
	memcpy(output.data, &success, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadLoginStatusPacket(WasabiGame::NetworkUpdate& input, bool& success) {
	if (input.dataSize != 1)
		return false;
	memcpy(&success, input.data, input.dataSize);
	return true;
}

void RollTheBall::UpdateBuilders::SelectGameMode(WasabiGame::NetworkUpdate& output, uint32_t gameMode) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SELECT_GAME_MODE);
	output.dataSize = sizeof(uint32_t);
	memcpy(output.data, &gameMode, output.dataSize);
}

bool RollTheBall::UpdateBuilders::ReadSelectGameModePacket(WasabiGame::NetworkUpdate& input, uint32_t& gameMode) {
	if (input.dataSize != sizeof(uint32_t))
		return false;
	memcpy(&gameMode, input.data, input.dataSize);
	return true;
}

void RollTheBall::UpdateBuilders::LoadUnit(WasabiGame::NetworkUpdate& output, uint32_t unitType, uint32_t unitId, WVector3 spawnPos) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOAD_UNIT);
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
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_UNLOAD_UNIT);
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RollTheBall::UpdateBuilders::ReadUnloadUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId) {
	*unitId = input.targetId;
	return true;
}

void RollTheBall::UpdateBuilders::WhoIsUnit(WasabiGame::NetworkUpdate& output, uint32_t unitId) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_WHOIS_UNIT);
	output.purpose = 0;
	output.targetId = unitId;
	output.dataSize = 0;
}

bool RollTheBall::UpdateBuilders::ReadWhoIsUnitPacket(WasabiGame::NetworkUpdate& input, uint32_t* unitId) {
	*unitId = input.targetId;
	return true;
}

void RollTheBall::UpdateBuilders::SetUnitProps(WasabiGame::NetworkUpdate& output, uint32_t unitId, std::function<void(std::string, void*, uint16_t)>* fillFunc) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS);
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

void RollTheBall::UpdateBuilders::LoadMap(WasabiGame::NetworkUpdate& output, uint32_t map) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOAD_MAP);
	output.purpose = 0;
	output.targetId = map;
	output.dataSize = 0;
}

bool RollTheBall::UpdateBuilders::ReadLoadMapPacket(WasabiGame::NetworkUpdate& input, uint32_t* map) {
	*map = input.targetId;
	return true;
}
