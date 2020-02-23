#pragma once

#include "WasabiGame/Maps/MapLoader.hpp"


namespace RollTheBall {

	enum MAP_NAME {
		MAP_NONE = 0,
		MAP_TEST = 1,
	};

	void SetupRTBMaps(std::shared_ptr<WasabiGame::MapLoader> loader);

};
