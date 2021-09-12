#pragma once

#include "RollTheBall/Networking/Protocol.hpp"
#include "WasabiGame/Networking/Data.hpp"
#include "Wasabi/Core/WMath.hpp"

#include <optional>

#ifdef _MSC_VER
#define PACKED_STRUCT(s) struct s
#else
#define PACKED_STRUCT(s) struct __attribute__((packed)) s
#endif

namespace RollTheBall {
	namespace UpdateBuilders {
		namespace GameStateSync {

			const float PACKET_REPLAY_DELAY_S = 0.1f;
			const uint8_t MAX_INPUTS_PER_PACKET = 25;
			const uint8_t MAX_UNITS_PER_PACKET = 25;
			constexpr uint8_t UNITS_MOTION_STATE_STRUCT_HEADER_SIZE = sizeof(uint32_t) + sizeof(uint8_t);

			typedef uint16_t SEQUENCE_NUMBER_TYPE;
			u_short (__stdcall *const SEQUENCE_NUMBER_HTON)(u_short) = htons;
			SEQUENCE_NUMBER_TYPE (__stdcall *const SEQUENCE_NUMBER_NTOH)(SEQUENCE_NUMBER_TYPE) = ntohs;

			#pragma pack(push, 1)
			PACKED_STRUCT(INPUT_STRUCT) {
				SEQUENCE_NUMBER_TYPE sequenceNumber;
				uint16_t millisSinceLastInput;
				float yaw;
				bool forward;
				bool backward;
				bool left;
				bool right;
				bool jump;

				size_t WriteToMemory(void* memory);
				bool ReadFromMemory(void* memory, size_t memSize);
			};

			PACKED_STRUCT(STATE_STRUCT) {
				uint32_t uintId;
				WVector3 position;
				WQuaternion rotation;
				WVector3 linearVelocity;
				WVector3 angularVelocity;
				INPUT_STRUCT input;

				size_t WriteToMemory(void* memory);
				bool ReadFromMemory(void* memory, size_t memSize);
			};

			PACKED_STRUCT(UNITS_MOTION_STATE_STRUCT) {
				SEQUENCE_NUMBER_TYPE sequenceNumber;
				uint8_t numStates;
				STATE_STRUCT unitStates[MAX_UNITS_PER_PACKET];

				size_t WriteToMemory(void* memory);
				bool ReadFromMemory(void* memory, size_t memSize);
			};
			#pragma pack(pop)


			template<typename PacketType> class PacketsBuffer {
				struct BUFFERED_PACKET_AND_METADATA {
					PacketType packet;
					SEQUENCE_NUMBER_TYPE sequenceNumber;
					float timeToReplayPacket;
				};

				std::vector<BUFFERED_PACKET_AND_METADATA> m_queuedPackets;
				SEQUENCE_NUMBER_TYPE m_nextPacketSequence;

			public:

				PacketsBuffer() : m_nextPacketSequence(0) {}

				void InsertPacket(float curTime, PacketType packet, SEQUENCE_NUMBER_TYPE curReceivedSequenceNumber, uint16_t millisSincePreviousSentPacket) {
					// TODO: WORK AROUND SEQUENCE NUMBER WRAPPING
					BUFFERED_PACKET_AND_METADATA packetToInsert = { packet, curReceivedSequenceNumber, 0.0f };
					if (curReceivedSequenceNumber >= m_nextPacketSequence) {
						for (uint32_t indexInQueuedPackets = 0; indexInQueuedPackets < m_queuedPackets.size() + 1; indexInQueuedPackets++) {
							if (indexInQueuedPackets == m_queuedPackets.size()) {
								// we don't have this packet and its bigger than all sequences, add it to the end
								packetToInsert.timeToReplayPacket =
									millisSincePreviousSentPacket == std::numeric_limits<uint16_t>::max() || m_queuedPackets.size() == 0
									? curTime + PACKET_REPLAY_DELAY_S
									: m_queuedPackets[m_queuedPackets.size() - 1].timeToReplayPacket + (float)millisSincePreviousSentPacket / 1000.0f;
								m_queuedPackets.push_back(packetToInsert);
								break;
							}
							if (m_queuedPackets[indexInQueuedPackets].sequenceNumber == curReceivedSequenceNumber)
								break; // we have this packet, ignore
							if (m_queuedPackets[indexInQueuedPackets].sequenceNumber > curReceivedSequenceNumber) {
								// this received sequence number is not seen yet, and should be before indexInQueuedPackets
								packetToInsert.timeToReplayPacket =
									millisSincePreviousSentPacket == std::numeric_limits<uint16_t>::max() || indexInQueuedPackets == 0
									? curTime + PACKET_REPLAY_DELAY_S
									: m_queuedPackets[indexInQueuedPackets - 1].timeToReplayPacket + (float)millisSincePreviousSentPacket / 1000.0f;
								m_queuedPackets.insert(m_queuedPackets.begin() + indexInQueuedPackets, packetToInsert);
								break;
							}
						}
					}
				}

				std::optional<PacketType> ConsumePacket(float curTime) {
					if (m_queuedPackets.size() > 0 && curTime >= m_queuedPackets[0].timeToReplayPacket) {
						PacketType packet = m_queuedPackets[0].packet;
						m_nextPacketSequence = m_queuedPackets[0].sequenceNumber + 1;
						m_queuedPackets.erase(m_queuedPackets.begin());
						return std::optional<PacketType>(packet);
					}
					return std::nullopt;
				}
			};

			// (UDP) Server <-- Client: send input state to the server
			void SetPlayerInput(WasabiGame::NetworkUpdate& output, std::vector<INPUT_STRUCT> inputStructs);
			bool ReadSetPlayerInputPacket(WasabiGame::NetworkUpdate& input, std::vector<INPUT_STRUCT>* inputStructs);

			// (UDP) Server --> Client: acknowledge reception of some input sequences (to stop client from resending)
			void AckPlayerInputs(WasabiGame::NetworkUpdate& output, std::vector<SEQUENCE_NUMBER_TYPE> inputSequences);
			bool ReadAckPlayerInputsPacket(WasabiGame::NetworkUpdate& input, std::vector<SEQUENCE_NUMBER_TYPE>* inputSequences);

			// (UDP) Server --> Client: server notifying a client of the current state and input of a unit
			void SetUnitsMotionStates(WasabiGame::NetworkUpdate& output, UNITS_MOTION_STATE_STRUCT motionStates);
			bool ReadSetUnitsMotionStatesPacket(WasabiGame::NetworkUpdate& input, UNITS_MOTION_STATE_STRUCT* motionStates);

		}
	}
}
