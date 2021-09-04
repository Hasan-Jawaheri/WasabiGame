#pragma once

#include "Common.hpp"
#include "WasabiGame/Maps/Map.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"


namespace WasabiGame {

	class WasabiBaseGame;

	class MapLoader {
		std::weak_ptr<WasabiBaseGame> m_app;
		std::shared_ptr<ResourceManager> m_resourceManager;

		std::shared_ptr<class Map> m_currentMap;
		std::unordered_map<uint, std::function<std::shared_ptr<class Map> ()>> m_mapGenerators;
		
		void Cleanup();

	public:
		MapLoader(std::shared_ptr<WasabiBaseGame> app, std::shared_ptr<ResourceManager> resourceManager);
		~MapLoader();

		void RegisterMap(uint32_t id, std::function<std::shared_ptr<class Map> ()> mapGenerator);
		void ResetMaps();

		void SetMap(uint32_t mapId);
		void Update(float fDeltaTime);
	};

};
