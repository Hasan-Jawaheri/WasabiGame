#pragma once

#include "Common.hpp"

struct LOADED_MODEL {
	WObject* obj;
	WRigidBody* rb;

	LOADED_MODEL() : obj(nullptr), rb(nullptr) {}
};

class ResourceManager {

	static std::string m_mediaFolder;

	static struct MAP_RESOURCES {
		WFile* mapFile;
		std::vector<LOADED_MODEL> loadedAssets;
		std::vector<WLight*> loadedLights;

		void Cleanup();
	} m_mapResources;

	static struct GENERAL_RESOURCES {
		WFile* assetsFile;
		std::unordered_map<LOADED_MODEL*, std::string> loadedAssets;

		void Cleanup();
	} m_generalResources;

public:
	static WError Init(std::string mediaFolder);
	static void Cleanup();

	static void LoadMapFile(std::string mapFilename);

	static LOADED_MODEL* LoadUnitModel(std::string unitName);
	static void DestroyUnitModel(LOADED_MODEL* model);
};


