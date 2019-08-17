#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <random>
#include <ctime>

RTBConnectedPlayer::RTBConnectedPlayer(uint32_t clientId) {
	m_clientId = clientId;
	m_loaded = false;
}

bool RTBConnectedPlayer::Load(RPGNet::ClientIdentity identity) {
	m_x = (float)(std::rand() % 100000) / 20000.0f - 2.5f;
	m_y = (float)(std::rand() % 100000) / 20000.0f + 4.0f;
	m_z = (float)(std::rand() % 100000) / 20000.0f - 2.5f;
	m_loaded = true;

	return m_loaded;
}
