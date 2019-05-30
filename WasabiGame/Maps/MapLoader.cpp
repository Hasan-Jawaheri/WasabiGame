#include "MapLoader.hpp"
#include "../ResourceManager/ResourceManager.hpp"

Map* MapLoader::m_currentMap = nullptr;
std::unordered_map<uint, std::function<class Map* ()>> MapLoader::m_mapGenerators;

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
		ResourceManager::LoadMapFile(desc.mapFilename);
		m_currentMap->OnLoaded();
	} else
		ResourceManager::LoadMapFile("");
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
