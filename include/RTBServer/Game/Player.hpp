#pragma once

#include <cstdint>
#include "RollTheBall/Networking/Data.hpp"

class RTBPlayer {
	bool m_loaded;
	float m_x, m_y, m_z;

public:
	RTBPlayer();

	bool Load(RPGNet::ClientIdentity identity);
};
