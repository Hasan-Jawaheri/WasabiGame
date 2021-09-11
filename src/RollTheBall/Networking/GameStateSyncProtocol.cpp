#include "RollTheBall/Networking/GameStateSyncProtocol.hpp"
#include "Wasabi/Core/WTimer.hpp"


size_t RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT::WriteToMemory(void* memory) {
	SEQUENCE_NUMBER_TYPE seqToWrite = SEQUENCE_NUMBER_HTON(sequenceNumber);
	uint16_t millisSinceLastInputToWrite = htons(millisSinceLastInput);
	unsigned int yawToWrite = htonf(yaw);
	size_t sizeofConvertedData = sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(uint16_t) + sizeof(unsigned long);
	memcpy((char*)memory, &seqToWrite, sizeof(SEQUENCE_NUMBER_TYPE));
	memcpy((char*)memory + sizeof(SEQUENCE_NUMBER_TYPE), &millisSinceLastInputToWrite, sizeof(uint16_t));
	memcpy((char*)memory + sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(uint16_t), &yawToWrite, sizeof(unsigned long));
	memcpy((char*)memory + sizeofConvertedData, &forward, sizeof(INPUT_STRUCT) - sizeofConvertedData);
	return sizeof(INPUT_STRUCT);
}

bool RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT::ReadFromMemory(void* memory, size_t memSize) {
	if (memSize < sizeof(INPUT_STRUCT))
		return false;

	unsigned int yawRead;
	memcpy(&sequenceNumber, memory, sizeof(INPUT_STRUCT));
	memcpy(&yawRead, (char*)memory + sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(uint16_t), sizeof(unsigned int));
	sequenceNumber = SEQUENCE_NUMBER_NTOH(sequenceNumber);
	millisSinceLastInput= ntohs(millisSinceLastInput);
	yaw = ntohf(yawRead);
	return true;
}

size_t RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT::WriteToMemory(void* memory) {
	for (size_t i = 0; i < 14; i++) { // 14 4-byte values (floats and ints)
		long value = htonl(((uint32_t*)&this->uintId)[i]);
		memcpy((char*)memory + 4 * i, &value, 4);
	}
	this->input.WriteToMemory((char*)memory + 4 * 14);

	return sizeof(STATE_STRUCT);
}

bool RollTheBall::UpdateBuilders::GameStateSync::STATE_STRUCT::ReadFromMemory(void* memory, size_t memSize) {
	if (memSize < sizeof(STATE_STRUCT))
		return false;

	for (size_t i = 0; i < 14; i++) { // 14 4-byte values (floats and ints)
		long value = ntohl(((uint32_t*)memory)[i]);
		memcpy((char*)&this->uintId + 4 * i, &value, 4);
	}

	this->input.ReadFromMemory((char*)memory + 4 * 14, memSize - 4 * 14);

	return true;
}

size_t RollTheBall::UpdateBuilders::GameStateSync::UNITS_MOTION_STATE_STRUCT::WriteToMemory(void* memory) {
	SEQUENCE_NUMBER_TYPE sequence = SEQUENCE_NUMBER_HTON(sequenceNumber);
	memcpy(memory, &sequence, sizeof(SEQUENCE_NUMBER_TYPE));
	memcpy((char*)memory + sizeof(SEQUENCE_NUMBER_TYPE), &numStates, sizeof(uint8_t));

	for (uint32_t i = 0; i < numStates; i++) {
		unitStates[i].WriteToMemory((char*)memory + sizeof(uint8_t) + sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(STATE_STRUCT) * i);
	}

	return sizeof(uint8_t) + sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(STATE_STRUCT) * numStates;
}

bool RollTheBall::UpdateBuilders::GameStateSync::UNITS_MOTION_STATE_STRUCT::ReadFromMemory(void* memory, size_t memSize) {
	if (memSize < sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(uint8_t))
		return false;

	// read sequence numer and numStates (struct is packed so memory is contigiuous)
	memcpy(&sequenceNumber, memory, sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(uint8_t));
	sequenceNumber = SEQUENCE_NUMBER_NTOH(sequenceNumber);

	for (uint32_t i = 0; i < numStates; i++) {
		size_t curOffset = sizeof(uint8_t) + sizeof(SEQUENCE_NUMBER_TYPE) + sizeof(STATE_STRUCT) * i;
		if (!unitStates[i].ReadFromMemory((char*)memory + curOffset, memSize - curOffset))
			return false;
	}

	return true;
}

void RollTheBall::UpdateBuilders::GameStateSync::SetPlayerInput(WasabiGame::NetworkUpdate& output, std::vector<INPUT_STRUCT> inputStructs) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_PLAYER_INPUT);
	output.dataSize = 0;
	for (auto iter : inputStructs) {
		output.dataSize += iter.WriteToMemory((char*)output.data + output.dataSize);
	}
}

bool RollTheBall::UpdateBuilders::GameStateSync::ReadSetPlayerInputPacket(WasabiGame::NetworkUpdate& input, std::vector<INPUT_STRUCT>* inputStructs) {
	if (input.dataSize % sizeof(INPUT_STRUCT) != 0)
		return false;

	for (uint32_t i = 0; i < input.dataSize / sizeof(INPUT_STRUCT); i++) {
		INPUT_STRUCT inputStruct;
		if (!inputStruct.ReadFromMemory((char*)input.data + i * sizeof(INPUT_STRUCT), sizeof(INPUT_STRUCT)))
			return false;
		inputStructs->push_back(inputStruct);
	}

	return true;
}

void RollTheBall::UpdateBuilders::GameStateSync::AckPlayerInputs(WasabiGame::NetworkUpdate& output, std::vector<SEQUENCE_NUMBER_TYPE> inputSequences) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_ACK_PLAYER_INPUTS);
	output.dataSize = sizeof(SEQUENCE_NUMBER_TYPE) * inputSequences.size();
	for (uint32_t i = 0; i < inputSequences.size(); i++) {
		SEQUENCE_NUMBER_TYPE sequence = SEQUENCE_NUMBER_HTON(inputSequences[i]);
		memcpy((char*)output.data + i * sizeof(SEQUENCE_NUMBER_TYPE), &sequence, sizeof(SEQUENCE_NUMBER_TYPE));
	}
}

bool RollTheBall::UpdateBuilders::GameStateSync::ReadAckPlayerInputsPacket(WasabiGame::NetworkUpdate& input, std::vector<SEQUENCE_NUMBER_TYPE>* inputSequences) {
	if (input.dataSize % sizeof(SEQUENCE_NUMBER_TYPE) != 0)
		return false;

	for (uint32_t i = 0; i < input.dataSize / sizeof(SEQUENCE_NUMBER_TYPE); i++) {
		SEQUENCE_NUMBER_TYPE sequence;
		memcpy(&sequence, (char*)input.data + i * sizeof(SEQUENCE_NUMBER_TYPE), sizeof(SEQUENCE_NUMBER_TYPE));
		inputSequences->push_back(SEQUENCE_NUMBER_NTOH(sequence));
	}

	return true;
}

void RollTheBall::UpdateBuilders::GameStateSync::SetUnitsMotionStates(WasabiGame::NetworkUpdate& output, UNITS_MOTION_STATE_STRUCT motionStates) {
	output.type = static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNITS_MOTION_STATES);
	output.targetId = 0;
	output.dataSize = motionStates.WriteToMemory(output.data);
}

bool RollTheBall::UpdateBuilders::GameStateSync::ReadSetUnitsMotionStatesPacket(WasabiGame::NetworkUpdate& input, UNITS_MOTION_STATE_STRUCT* motionStates) {
	return motionStates->ReadFromMemory(input.data, input.dataSize);
}

