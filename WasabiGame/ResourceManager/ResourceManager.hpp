#pragma once

#include "../Common.hpp"

struct LOADED_MODEL {
	WObject* obj;
	WRigidBody* rb;

	LOADED_MODEL() : obj(nullptr), rb(nullptr) {}
};

class ResourceManager {

	static struct MAP_RESOURCES {
		WFile* mapFile;
		std::vector<LOADED_MODEL> loadedAssets;

		void Cleanup();
	} m_mapResources;

	static struct GENERAL_RESOURCES {
		WFile* assetsFile;
		std::unordered_map<std::string, LOADED_MODEL*> loadedAssets;

		void Cleanup();
	} m_generalResources;

public:
	static void Init();
	static void Cleanup();

	static void LoadMapFile(std::string mapFilename);
	static LOADED_MODEL* LoadUnitModel(std::string unitName);
};


