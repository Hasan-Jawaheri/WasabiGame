#pragma once

#include "WasabiGame/Maps/Map.hpp"

class MapLoader {
	static class Map* m_currentMap;
	static std::unordered_map<uint, std::function<class Map* ()>> m_mapGenerators;

public:
	static void RegisterMap(uint id, std::function<class Map*()> mapGenerator);
	static void ResetMaps();

	static void SetMap(uint mapId);
	static void Update(float fDeltaTime);
	static void Cleanup();
};
