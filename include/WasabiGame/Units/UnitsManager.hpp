#pragma once

#include "Common.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"

#include <mutex>


namespace WasabiGame {

	class WasabiBaseGame;

	class UnitsManager {
		std::weak_ptr<WasabiBaseGame> m_app;
		std::shared_ptr<ResourceManager> m_resourceManager;

		std::mutex m_unitsMutex;
		std::unordered_map<uint32_t, std::function<std::shared_ptr<class Unit> ()>> m_unitGenerators;
		std::unordered_map<uint32_t, std::pair<uint32_t, std::shared_ptr<class Unit>>> m_units;

	public:
		UnitsManager(std::shared_ptr<WasabiBaseGame> app, std::shared_ptr<ResourceManager> resourceManager);
		~UnitsManager();

		template<typename U, typename A>
		std::shared_ptr<U> CreateUnitAndAI(std::string modelName) {
			std::shared_ptr<U> u = std::make_shared<U>(m_app, m_resourceManager, this);
			u->SetAI<A>();
			u->m_loadInfo.modelName = modelName;
			return u;
		}

		void RegisterUnit(uint32_t id, std::function<std::shared_ptr<class Unit> ()> unitGenerator);
		void ResetUnits();

		std::shared_ptr<Unit> LoadUnit(uint32_t type, uint32_t id, WVector3 spawnPos = WVector3());
		std::shared_ptr<Unit> GetUnit(uint32_t id);
		void DestroyUnit(std::shared_ptr<Unit> unit);
		void DestroyUnit(uint32_t id);
		void Update(float fDeltaTime);
		void Cleanup();
	};

};
