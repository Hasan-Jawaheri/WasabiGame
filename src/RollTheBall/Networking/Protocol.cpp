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
