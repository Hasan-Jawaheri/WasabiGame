#include "MapLoader.hpp"
#include "../ResourceManager/ResourceManager.hpp"
#include "Maps/ICC.hpp"

Map* MapLoader::m_currentMap = nullptr;

void MapLoader::SetMap(MAP_NAME mapName) {
	Map* newMap = nullptr;
	switch (mapName) {
	case MAP_ICC:
		newMap = new ICCMap();
		break;
	}

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
