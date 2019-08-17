#pragma once

#include <cstdint>
#include "RollTheBall/Networking/Data.hpp"

class RTBConnectedPlayer {
public:
	RTBConnectedPlayer(uint32_t clientId);

	uint32_t m_clientId;
	bool m_loaded;
	float m_x, m_y, m_z;

	bool Load(RPGNet::ClientIdentity identity);
};
