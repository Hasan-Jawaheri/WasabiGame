#include "RTBServer/Simulation/GameStateSyncBackend.hpp"


RTBServer::GameStateSyncBackend::GameStateSyncBackend(std::shared_ptr<ServerNetworking> networking, WTimer* timer) {
	m_networking = networking;
	m_timer = timer;
	m_lastBroadcastTime = 0.0f;
}

void RTBServer::GameStateSyncBackend::Update(float fDeltaTime) {
	{
		std::scoped_lock lockGuard(m_playersMutex);
		if (m_timer->GetElapsedTime() - m_lastBroadcastTime > 0.05f) {
			m_lastBroadcastTime = m_timer->GetElapsedTime();
			for (auto senderPlayer : m_players) {
				WVector3 pos = senderPlayer.second.second->O()->GetPosition();
				WasabiGame::NetworkUpdate update;
				std::function<void(std::string, void*, uint16_t)> addProp = nullptr;
				RollTheBall::UpdateBuilders::SetUnitProps(update, senderPlayer.second.second->GetId(), &addProp);
				addProp("pos", (void*)&pos, sizeof(WVector3));

				for (auto receiverPlayer : m_players) {
					m_networking->SendUpdate(receiverPlayer.second.first->m_clientId, update, false);
				}
			}
		}
	}
}

void RTBServer::GameStateSyncBackend::AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::shared_ptr<WasabiGame::Unit> unit) {
	std::scoped_lock lockGuard(m_playersMutex);
	m_players.insert(std::make_pair(player->m_clientId, std::make_pair(player, unit)));
}

std::shared_ptr<WasabiGame::Unit> RTBServer::GameStateSyncBackend::RemovePlayer(uint32_t clientId) {
	std::scoped_lock lockGuard(m_playersMutex);
	auto it = m_players.find(clientId);
	if (it != m_players.end()) {
		std::shared_ptr<WasabiGame::Unit> playerUnit = it->second.second;
		m_players.erase(it);
		return playerUnit;
	}
	return nullptr;
}
