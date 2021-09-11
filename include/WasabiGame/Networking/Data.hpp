#pragma once

#include <cstdint>
#include <algorithm>

#if (defined WIN32 || defined _WIN32)
#define NOMINMAX
#include <WinSock2.h>
#endif

#include "WasabiGame/Utilities/CircularBuffer.hpp"


namespace WasabiGame {

	struct ClientIdentity {
		char accountName[256];
		char passwordHash[256];
	};

	const size_t MAX_PACKET_SIZE = 4096;
	const size_t PACKET_META_SIZE = 8;

	typedef uint16_t NetworkUpdateType;

	struct NetworkUpdate {
		NetworkUpdateType type;
		uint16_t dataSize;
		uint32_t targetId;
		char data[MAX_PACKET_SIZE - PACKET_META_SIZE];

		size_t fillPacket(char* packet) {
			size_t hdataSize = (size_t)dataSize;

			NetworkUpdateType typeToWrite = htons(type);
			uint32_t targetIdToWrite = htonl(targetId);
			uint16_t dataSizeToWrite = htons(dataSize);

			memcpy(packet, (char*)&typeToWrite, sizeof(NetworkUpdateType));
			memcpy(packet + 2, (char*)&dataSizeToWrite, sizeof(uint16_t));
			memcpy(packet + 4, (char*)&targetIdToWrite, sizeof(uint32_t));
			if (hdataSize > 0)
				memcpy(packet + 8, (char*)data, hdataSize);

			return 8 + hdataSize;
		}

		size_t readPacket(WasabiGame::CircularBuffer* packet) {
			if (packet->GetSize() >= PACKET_META_SIZE) {
				char tmp[PACKET_META_SIZE];
				size_t availableToRead = std::min(packet->GetAvailableContigiousConsume(), PACKET_META_SIZE);
				memcpy(tmp, packet->GetReadingMem(), availableToRead);
				if (availableToRead < PACKET_META_SIZE)
					memcpy(tmp + availableToRead, packet->mem, PACKET_META_SIZE - availableToRead);

				type = ntohs(*(NetworkUpdateType*)(tmp + 0));
				dataSize = ntohs(*(uint16_t*)(tmp + 2));
				targetId = ntohl(*(uint32_t*)(tmp + 4));

				if (packet->GetSize() >= PACKET_META_SIZE + dataSize) {
					packet->OnConsumed(PACKET_META_SIZE);
					size_t dataRead = 0;
					while (dataRead < dataSize) {
						size_t sizeToRead = std::min(packet->GetAvailableContigiousConsume(), dataSize - dataRead);
						memcpy(data + dataRead, packet->GetReadingMem(), sizeToRead);
						packet->OnConsumed(sizeToRead);
						dataRead += sizeToRead;
					}
					return PACKET_META_SIZE + dataSize;
				}
			}
			return 0;
		}

		size_t readPacket(void* buffer, size_t bufLen) {
			if (bufLen >= PACKET_META_SIZE) {
				type = ntohs(*(NetworkUpdateType*)((char*)buffer + 0));
				dataSize = ntohs(*(uint16_t*)((char*)buffer + 2));
				targetId = ntohl(*(uint32_t*)((char*)buffer + 4));

				if (bufLen >= PACKET_META_SIZE + dataSize) {
					memcpy(data, (char*)buffer + PACKET_META_SIZE, dataSize);
					return bufLen;
				}
			}
			return 0;
		}
	};
    
};
