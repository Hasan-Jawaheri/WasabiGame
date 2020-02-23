#pragma once

#include <cstdint>
#include "WasabiGame/Networking/Data.hpp"


namespace RTBServer {

	class RTBConnectedPlayer {
	public:
		RTBConnectedPlayer(uint32_t clientId);

		uint32_t m_clientId;
		bool m_loaded;
		float m_x, m_y, m_z;

		bool Load(WasabiGame::ClientIdentity identity);
	};

};
