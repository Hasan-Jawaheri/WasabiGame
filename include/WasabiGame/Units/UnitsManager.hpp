#pragma once

#include "Common.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"

#include <mutex>

class UnitsManager {
	Wasabi* m_app;
	ResourceManager* m_resourceManager;

	std::mutex m_unitsMutex;
	std::unordered_map<uint32_t, std::function<class Unit* ()>> m_unitGenerators;
	std::unordered_map<uint32_t, std::pair<uint32_t, class Unit*>> m_units;

public:
	UnitsManager(Wasabi* app, ResourceManager* resourceManager);

	template<typename U, typename A>
	U* CreateUnitAndAI(std::string modelName) {
		U* u = new U(m_app, m_resourceManager);
		u->SetAI<A>();
		u->m_loadInfo.modelName = modelName;
		return u;
	}

	void RegisterUnit(uint32_t id, std::function<class Unit* ()> unitGenerator);
	void ResetUnits();

	Unit* LoadUnit(uint32_t type, uint32_t id, WVector3 spawnPos = WVector3());
	Unit* GetUnit(uint32_t id);
	void DestroyUnit(Unit* unit);
	void DestroyUnit(uint32_t id);
	void Update(float fDeltaTime);
	void Cleanup();
};
