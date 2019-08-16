#pragma once

#include "Common.hpp"

#include <mutex>

struct LOADED_MODEL {
	WObject* obj;
	WRigidBody* rb;

	LOADED_MODEL() : obj(nullptr), rb(nullptr) {}
};

class ResourceManager {
	Wasabi* m_app;
	std::string m_mediaFolder;

	struct MAP_RESOURCES {
		WFile* mapFile;
		std::vector<LOADED_MODEL> loadedAssets;
		std::vector<WLight*> loadedLights;

		void Cleanup();
	} m_mapResources;

	struct GENERAL_RESOURCES {
		WFile* assetsFile;
		std::unordered_map<LOADED_MODEL*, std::string> loadedAssets;

		void Cleanup();
	} m_generalResources;

	std::mutex m_modelsToFreeMutex;
	std::vector<LOADED_MODEL*> m_modelsToFree;

public:
	ResourceManager(Wasabi* app);

	WError Init(std::string mediaFolder);
	void Update(float fDeltaTime);
	void Cleanup();

	void LoadMapFile(std::string mapFilename);

	LOADED_MODEL* LoadUnitModel(std::string unitName);
	void DestroyUnitModel(LOADED_MODEL* model);
};


