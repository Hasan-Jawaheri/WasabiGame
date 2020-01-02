#pragma once

#include <cstdint>
#include <algorithm>

#ifdef WIN32
#define NOMINMAX
#include <WinSock2.h>
#endif

#include "RollTheBall/Utilities/CircularBuffer.hpp"

namespace RPGNet {

	struct ClientIdentity {
		char accountName[256];
		char passwordHash[256];
	};

	const size_t MAX_PACKET_SIZE = 4096;
	const size_t PACKET_META_SIZE = 10;

	typedef short NetworkUpdateType;
	typedef short PacketPurpose;

	struct NetworkUpdate {
		NetworkUpdateType type;
		PacketPurpose purpose;
		uint32_t targetId;
		uint16_t dataSize;
		char data[MAX_PACKET_SIZE - PACKET_META_SIZE];

		size_t fillPacket(char* packet) {
			size_t hdataSize = (size_t)dataSize;

			type = htons(type);
			purpose = htons(purpose);
			targetId = htonl(targetId);
			dataSize = htons(dataSize);

			memcpy(packet, (char*)& type, sizeof(NetworkUpdateType));
			memcpy(packet + 2, (char*)& purpose, sizeof(PacketPurpose));
			memcpy(packet + 4, (char*)& targetId, sizeof(uint32_t));
			memcpy(packet + 8, (char*)& dataSize, sizeof(uint16_t));
			if (hdataSize > 0)
				memcpy(packet + 10, (char*)data, hdataSize);

			return 10 + hdataSize;
		}

		size_t readPacket(HBUtils::CircularBuffer* packet) {
			if (packet->GetSize() >= PACKET_META_SIZE) {
				char tmp[PACKET_META_SIZE];
				size_t availableToRead = std::min(packet->GetAvailableContigiousConsume(), PACKET_META_SIZE);
				if (availableToRead < PACKET_META_SIZE)
					memcpy(tmp + availableToRead, packet->mem, PACKET_META_SIZE - availableToRead);

				type = ntohs(*(NetworkUpdateType*)(tmp + 0));
				purpose = ntohs(*(PacketPurpose*)(tmp + 2));
				targetId = ntohl(*(uint32_t*)(tmp + 4));
				dataSize = ntohs(*(uint16_t*)(tmp + 8));

				if (packet->GetSize() >= PACKET_META_SIZE + dataSize) {
					packet->OnConsumed(PACKET_META_SIZE);
					size_t dataRead = 0;
					while (dataRead < dataSize) {
						size_t sizeToRead = std::min(packet->GetAvailableContigiousConsume(), dataSize - dataRead);
						packet->OnConsumed(sizeToRead);
						dataRead += sizeToRead;
					}
					return PACKET_META_SIZE + dataSize;
				}
			}
			return 0;
		}
	};
    
};
