#pragma once

#include "Common.hpp"

#include <mutex>


namespace WasabiGame {

	class WasabiBaseGame;

	struct LOADED_MODEL {
		std::string name;
		WObject* obj;
		WRigidBody* rb;
		WSkeleton* skeleton;

		LOADED_MODEL() : obj(nullptr), rb(nullptr), skeleton(nullptr) {}
	};

	class ResourceManager {
		std::weak_ptr<WasabiBaseGame> m_app;
		std::string m_mediaFolder;
		bool m_onlyPhysics;

		struct MAP_RESOURCES {
			WFile* mapFile;
			std::vector<LOADED_MODEL> loadedAssets;
			std::vector<WLight*> loadedLights;

			void Cleanup();
		} m_mapResources;

		struct GENERAL_RESOURCES {
			WFile* assetsFile;
			std::unordered_map<std::string, LOADED_MODEL*> loadedAssets;

			void Cleanup();
		} m_generalResources;

		std::mutex m_modelsToFreeMutex;
		std::vector<LOADED_MODEL*> m_modelsToFree;

	public:
		ResourceManager(std::shared_ptr<WasabiBaseGame> app, bool onlyPhysics = false);
		~ResourceManager();

		WError Init(std::string mediaFolder);
		void Update(float fDeltaTime);
		void Cleanup();

		void LoadMapFile(std::string mapFilename);

		LOADED_MODEL* LoadUnitModel(std::string unitName);
		void DestroyUnitModel(LOADED_MODEL* model);
	};

};

