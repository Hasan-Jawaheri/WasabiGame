#pragma once

#include "WasabiGame/Maps/MapLoader.hpp"


namespace RollTheBall {

	enum class MAP_NAME : uint32_t {
		MAP_NONE = 0,
		MAP_TEST = 1,
	};

	void SetupRTBMaps(std::shared_ptr<WasabiGame::MapLoader> loader);

};
