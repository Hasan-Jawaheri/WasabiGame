#include "RTBServer/Game/Player.hpp"

RTBPlayer::RTBPlayer(uint32_t clientId) {
	m_clientId = clientId;
	m_loaded = false;
}

bool RTBPlayer::Load(RPGNet::ClientIdentity identity) {
	m_x = (rand() % 10000) / 1000.0f - 5.0f;
	m_y = (rand() % 10000) / 10000.0f + 1.0f;
	m_z = (rand() % 10000) / 1000.0f - 5.0f;
	m_loaded = true;

	return m_loaded;
}
