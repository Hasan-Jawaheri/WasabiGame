#include "RTBServer/Simulation/GameStateSyncBackend.hpp"
#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/AI/PlayerAI.hpp"


RTBServer::GameStateSyncBackend::GameStateSyncBackend(std::shared_ptr<ServerNetworking> networking, WTimer* timer) {
	m_networking = networking;
	m_timer = timer;
	m_lastBroadcastTime = 0.0f;
}

void RTBServer::GameStateSyncBackend::Update(float fDeltaTime) {
	if (m_timer->GetElapsedTime() - m_lastBroadcastTime > RollTheBall::SEND_INPUT_TO_SERVER_PERIOD_S) {
		m_lastBroadcastTime = m_timer->GetElapsedTime();

		std::vector<WasabiGame::NetworkUpdate> updatesToSend;
		std::vector<uint32_t> clientIdsToReceiveUpdates;
		{
			std::scoped_lock lockGuard(m_playersMutex);
			updatesToSend.resize(m_players.size());
			clientIdsToReceiveUpdates.resize(m_players.size());

			uint32_t i = 0;
			for (auto playerIter : m_players) {
				if (playerIter.second.second->queuedInputs.size() > 0) {
					RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT input = playerIter.second.second->queuedInputs[0];
					playerIter.second.second->queuedInputs.erase(playerIter.second.second->queuedInputs.begin());
					playerIter.second.second->nextInputSequence = input.sequenceNumber + 1;
					std::shared_ptr<RollTheBall::RTBAI> playerAI = std::dynamic_pointer_cast<RollTheBall::RTBAI>(playerIter.second.second->unit->GetAI());
					playerAI->SetMoveForward(input.forward);
					playerAI->SetMoveBackward(input.backward);
					playerAI->SetMoveLeft(input.left);
					playerAI->SetMoveRight(input.right);
					playerAI->SetMoveJump(input.jump);
					playerAI->SetYawAngle(input.yaw);
				}


				std::shared_ptr<WasabiGame::Unit> unit = playerIter.second.second->unit;
				WVector3 pos = unit->O()->GetPosition();
				std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
				RollTheBall::UpdateBuilders::SetUnitProps(updatesToSend[i], unit->GetId(), &addProp);
				addProp("pos", (void*)&pos, sizeof(WVector3));
				clientIdsToReceiveUpdates[i] = playerIter.first;
				i++;
			}
		}

		for (uint32_t clientId : clientIdsToReceiveUpdates) {
			// TODO: this can be batched instead of many SendUpdate calls. need to redo the sync mechanism
			for (auto update : updatesToSend) {
				m_networking->SendUpdate(clientId, update, false);
			}
		}
	}
}

void RTBServer::GameStateSyncBackend::AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::shared_ptr<WasabiGame::Unit> unit) {
	std::shared_ptr<PLAYER_INFO> info = std::make_shared<PLAYER_INFO>();
	info->unit = unit;
	info->nextInputSequence = 0;
	std::scoped_lock lockGuard(m_playersMutex);
	m_players.insert(std::make_pair(player->m_clientId, std::make_pair(player, info)));
}

std::shared_ptr<WasabiGame::Unit> RTBServer::GameStateSyncBackend::RemovePlayer(uint32_t clientId) {
	std::scoped_lock lockGuard(m_playersMutex);
	auto it = m_players.find(clientId);
	if (it != m_players.end()) {
		std::shared_ptr<WasabiGame::Unit> playerUnit = it->second.second->unit;
		m_players.erase(it);
		return playerUnit;
	}
	return nullptr;
}

bool RTBServer::GameStateSyncBackend::OnPlayerInputUpdate(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::vector<RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT>& inputStructs) {
	// TODO: WORK AROUND SEQUENCE NUMBER WRAPPING

	// sequence numbers received must be strictly increasing
	for (uint32_t i = 1; i < inputStructs.size(); i++)
		if (inputStructs[i].sequenceNumber <= inputStructs[i - 1].sequenceNumber)
			return false;

	{
		std::scoped_lock lockGuard(m_playersMutex);
		auto it = m_players.find(player->m_clientId);
		if (it != m_players.end()) {
			std::shared_ptr<PLAYER_INFO> info = it->second.second;
			uint32_t lastIndexInPlayerInputs = 0;
			for (uint32_t indexInReceived = 0; indexInReceived < inputStructs.size(); indexInReceived++) {
				RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE curReceivedSequenceNumber = inputStructs[indexInReceived].sequenceNumber;
				if (curReceivedSequenceNumber >= info->nextInputSequence) {
					for (uint32_t indexInPlayerInputs = lastIndexInPlayerInputs; indexInPlayerInputs < info->queuedInputs.size() + 1; indexInPlayerInputs++) {
						lastIndexInPlayerInputs = indexInPlayerInputs;
						if (indexInPlayerInputs == info->queuedInputs.size()) {
							// we don't have this input and its bigger than all sequences, add it to the end
							info->queuedInputs.push_back(inputStructs[indexInReceived]);
							lastIndexInPlayerInputs++; // at this point, all coming inputs are new, this will optimize so we always hit this branch first
						}
						if (info->queuedInputs[indexInPlayerInputs].sequenceNumber == curReceivedSequenceNumber)
							break; // we have this input, ignore
						if (info->queuedInputs[indexInPlayerInputs].sequenceNumber > curReceivedSequenceNumber) {
							// this received sequence number is not seen yet, and should be before indexInPlayerInputs
							info->queuedInputs.insert(info->queuedInputs.begin() + indexInPlayerInputs, inputStructs[indexInReceived]);
							break;
						}
					}
				}
			}
		}
	}

	WasabiGame::NetworkUpdate ackUpdate;
	std::vector<RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE> ackInputs(inputStructs.size());
	for (uint32_t i = 0; i < inputStructs.size(); i++)
		ackInputs[i] = inputStructs[i].sequenceNumber;
	RollTheBall::UpdateBuilders::GameStateSync::AckPlayerInputs(ackUpdate, ackInputs);
	m_networking->SendUpdate(player->m_clientId, ackUpdate, false);

	return true;
}
