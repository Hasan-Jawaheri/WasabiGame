#pragma once

#include "Common.hpp"
#include "WasabiGame/Maps/Map.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"

class MapLoader {
	Wasabi* m_app;
	ResourceManager* m_resourceManager;

	class Map* m_currentMap;
	std::unordered_map<uint, std::function<class Map* ()>> m_mapGenerators;

public:
	MapLoader(Wasabi* app, ResourceManager* resourceManager);

	void RegisterMap(uint id, std::function<class Map*()> mapGenerator);
	void ResetMaps();

	void SetMap(uint mapId);
	void Update(float fDeltaTime);
	void Cleanup();
};
