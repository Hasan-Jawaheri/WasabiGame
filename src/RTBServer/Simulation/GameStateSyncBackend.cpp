#include "RTBServer/Simulation/GameStateSyncBackend.hpp"
#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/AI/PlayerAI.hpp"


RTBServer::GameStateSyncBackend::GameStateSyncBackend(std::shared_ptr<ServerNetworking> networking, WTimer* timer) {
	m_networking = networking;
	m_timer = timer;
	m_lastBroadcastTime = 0.0f;
	m_motionStatesCurrentSequenceNumber = 0;
}

void RTBServer::GameStateSyncBackend::Update(float fDeltaTime) {
	if (m_timer->GetElapsedTime() - m_lastBroadcastTime > RollTheBall::SEND_INPUT_TO_SERVER_PERIOD_S) {
		m_lastBroadcastTime = m_timer->GetElapsedTime();

		WasabiGame::NetworkUpdate gameStateUpdate;
		RollTheBall::UpdateBuilders::GameStateSync::UNITS_MOTION_STATE_STRUCT motionState;
		std::vector<uint32_t> clientIdsToReceiveUpdates;

		motionState.sequenceNumber = m_motionStatesCurrentSequenceNumber++;

		{
			std::scoped_lock lockGuard(m_playersMutex);
			motionState.numStates = m_players.size(); // TODO: make sure to pack only useful updates to each player
			clientIdsToReceiveUpdates.resize(m_players.size());

			uint32_t i = 0;
			for (auto playerIter : m_players) {
				clientIdsToReceiveUpdates[i] = playerIter.first;
				std::optional<RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT> consumedPacket = playerIter.second.second->inputsBuffer.ConsumePacket(m_lastBroadcastTime);
				if (consumedPacket.has_value()) {
					RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT input = consumedPacket.value();
					playerIter.second.second->latestInput = input;
					std::shared_ptr<RollTheBall::RTBAI> playerAI = std::dynamic_pointer_cast<RollTheBall::RTBAI>(playerIter.second.second->unit->GetAI());
					playerAI->SetMoveForward(input.forward);
					playerAI->SetMoveBackward(input.backward);
					playerAI->SetMoveLeft(input.left);
					playerAI->SetMoveRight(input.right);
					playerAI->SetMoveJump(input.jump);
					playerAI->SetYawAngle(input.yaw);
				}

				std::shared_ptr<WasabiGame::Unit> unit = playerIter.second.second->unit;
				motionState.unitStates[i].uintId = unit->GetId();
				motionState.unitStates[i].input = playerIter.second.second->latestInput;
				if (unit->RB()) {
					motionState.unitStates[i].position = unit->RB()->GetPosition();
					motionState.unitStates[i].rotation = unit->RB()->GetRotation();
					motionState.unitStates[i].linearVelocity = unit->RB()->getLinearVelocity();
					motionState.unitStates[i].angularVelocity = unit->RB()->getAngularVelocity();
				} else {
					motionState.unitStates[i].position = unit->O()->GetPosition();
					motionState.unitStates[i].rotation = unit->O()->GetRotation();
				}
				i++;
			}
		}

		RollTheBall::UpdateBuilders::GameStateSync::SetUnitsMotionStates(gameStateUpdate, motionState);

		for (uint32_t clientId : clientIdsToReceiveUpdates) {
			m_networking->SendUpdate(clientId, gameStateUpdate, false);
		}
	}
}

void RTBServer::GameStateSyncBackend::AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::shared_ptr<WasabiGame::Unit> unit) {
	std::shared_ptr<PLAYER_INFO> info = std::make_shared<PLAYER_INFO>();
	info->unit = unit;
	info->latestInput = { 0 };
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
	{
		std::scoped_lock lockGuard(m_playersMutex);
		auto it = m_players.find(player->m_clientId);
		if (it != m_players.end()) {
			std::shared_ptr<PLAYER_INFO> info = it->second.second;
			for (auto input : inputStructs) {
				info->inputsBuffer.InsertPacket(m_timer->GetElapsedTime(), input, input.sequenceNumber, input.millisSinceLastInput);
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
