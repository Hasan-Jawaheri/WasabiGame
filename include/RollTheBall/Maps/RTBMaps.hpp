#pragma once

#include "WasabiGame/Maps/MapLoader.hpp"

enum MAP_NAME {
	MAP_NONE = 0,
	MAP_TEST = 1,
};

void SetupRTBMaps(MapLoader* loader);
