#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Main.hpp"


WasabiGame::MapLoader::MapLoader(std::shared_ptr<WasabiBaseGame> app, std::shared_ptr<ResourceManager> resourceManager) {
	m_app = app;
	m_resourceManager = resourceManager;
	m_currentMap = nullptr;
}

WasabiGame::MapLoader::~MapLoader() {
	Cleanup();
}

void WasabiGame::MapLoader::RegisterMap(uint32_t id, std::function<std::shared_ptr<class Map> ()> mapGenerator) {
	m_mapGenerators.insert(std::make_pair(id, mapGenerator));
}

void WasabiGame::MapLoader::ResetMaps() {
	m_mapGenerators.clear();
}

void WasabiGame::MapLoader::SetMap(uint32_t mapId) {
	std::shared_ptr<Map> newMap = nullptr;
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

void WasabiGame::MapLoader::Update(float fDeltaTime) {
	if (m_currentMap)
		m_currentMap->Update(fDeltaTime);
}

void WasabiGame::MapLoader::Cleanup() {
	if (m_currentMap) {
		m_currentMap.reset();
	}
}
