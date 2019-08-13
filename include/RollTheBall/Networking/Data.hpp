#pragma once

#include <cstdint>

#ifdef WIN32
#include <WinSock2.h>
#endif

#include "RollTheBall/Utilities/CircularBuffer.hpp"

namespace RPGNet {

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
			int hdataSize = dataSize;

			type = htons(type);
			purpose = htons(purpose);
			targetId = htonl(targetId);
			dataSize = htons(dataSize);

			memcpy(packet, (char*)& type, sizeof(NetworkUpdateType));
			memcpy(packet, (char*)& purpose, sizeof(PacketPurpose));
			memcpy(packet, (char*)& targetId, sizeof(uint32_t));
			memcpy(packet, (char*)& dataSize, sizeof(uint16_t));
			if (hdataSize > 0)
				memcpy(packet, (char*)data, hdataSize);

			return sizeof(NetworkUpdateType) + sizeof(PacketPurpose) + sizeof(uint32_t);
		}

		size_t readPacket(HBUtils::CircularBuffer* packet) {
			if (packet->GetSize() >= PACKET_META_SIZE) {
				char tmp[PACKET_META_SIZE];
				size_t availableToRead = min(packet->GetAvailableContigiousConsume(), PACKET_META_SIZE);
				memcpy(tmp, packet->GetMem(), availableToRead);
				if (availableToRead < PACKET_META_SIZE)
					memcpy(tmp + availableToRead, packet->mem, PACKET_META_SIZE - availableToRead);

				type = ntohs(*(NetworkUpdateType*)(tmp + 0));
				purpose = ntohs(*(PacketPurpose*)(tmp + 2));
				targetId = ntohl(*(uint32_t*)(tmp + 4));
				dataSize = ntohs(*(uint16_t*)(tmp + 8));

				if (PACKET_META_SIZE + dataSize >= packet->GetSize()) {
					packet->OnConsumed(PACKET_META_SIZE);
					size_t dataRead = 0;
					while (dataRead < dataSize) {
						size_t sizeToRead = min(packet->GetAvailableContigiousConsume(), dataSize - dataRead);
						memcpy(data + dataRead, packet->GetMem(), sizeToRead);
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
