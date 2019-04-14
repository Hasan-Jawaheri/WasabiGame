#pragma once

#include "Map.hpp"

enum MAP_NAME {
	MAP_ICC = 1,
};

class MapLoader {
	static class Map* m_currentMap;

public:
	static void SetMap(MAP_NAME mapName);
	static void Update(float fDeltaTime);
	static void Cleanup();
};
