#include "WasabiGame/Maps/MapLoader.hpp"

MapLoader::MapLoader(Wasabi* app, ResourceManager* resourceManager) {
	m_app = app;
	m_resourceManager = resourceManager;
	m_currentMap = nullptr;
}

void MapLoader::RegisterMap(uint id, std::function<class Map* ()> mapGenerator) {
	m_mapGenerators.insert(std::make_pair(id, mapGenerator));
}

void MapLoader::ResetMaps() {
	m_mapGenerators.clear();
}

void MapLoader::SetMap(uint mapId) {
	Map* newMap = nullptr;
	auto it = m_mapGenerators.find(mapId);
	if (it != m_mapGenerators.end())
		newMap = it->second();

	Cleanup();
	m_currentMap = newMap;

	if (m_currentMap) {
		Map::MAP_DESCRIPTION desc = m_currentMap->GetDescription();
		m_resourceManager->LoadMapFile(desc.mapFilename);
		m_currentMap->OnLoaded();
	} else
		m_resourceManager->LoadMapFile("");
}

void MapLoader::Update(float fDeltaTime) {
	if (m_currentMap)
		m_currentMap->Update(fDeltaTime);
}

void MapLoader::Cleanup() {
	if (m_currentMap) {
		m_currentMap->Cleanup();
		delete m_currentMap;
	}
}
