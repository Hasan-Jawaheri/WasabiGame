#include "WasabiGame/Units/UnitsManager.hpp"

UnitsManager::UnitsManager(Wasabi* app, ResourceManager* resourceManager) {
	m_app = app;
	m_resourceManager = resourceManager;
}

void UnitsManager::RegisterUnit(uint32_t id, std::function<class Unit* ()> unitGenerator) {
	m_unitGenerators.insert(std::make_pair(id, unitGenerator));
}

void UnitsManager::ResetUnits() {
	m_unitGenerators.clear();
}

Unit* UnitsManager::LoadUnit(uint32_t type, uint32_t id, WVector3 spawnPos) {
	auto it = m_unitGenerators.find(type);
	Unit* unit = it->second();
	unit->m_loadInfo.spawnPos = spawnPos;
	unit->m_canLoad.store(true);
	unit->m_id = id;

	{
		std::lock_guard lockGuard(m_unitsMutex);
		m_units.insert(std::make_pair(id, std::make_pair(type, unit)));
	}
	return unit;
}

Unit* UnitsManager::GetUnit(uint32_t id) {
	std::lock_guard lockGuard(m_unitsMutex);
	auto it = m_units.find(id);
	if (it != m_units.end()) {
		return it->second.second;
	}
	return nullptr;
}

void UnitsManager::DestroyUnit(Unit* unit) {
	DestroyUnit(unit->m_id);
}

void UnitsManager::DestroyUnit(uint32_t id) {
	Unit* unit = nullptr;
	{
		std::lock_guard lockGuard(m_unitsMutex);
		auto it = m_units.find(id);
		if (it != m_units.end()) {
			unit = it->second.second;
			m_units.erase(it);
		}
	}
	if (unit)
		delete unit;
}

void UnitsManager::Update(float fDeltaTime) {
	std::lock_guard lockGuard(m_unitsMutex);
	Unit* unitToDelete = nullptr;
	for (auto unit : m_units) {
		unit.second.second->Update(fDeltaTime);
		if (!unit.second.second->m_model && !unitToDelete) {
			// unit failed to load
			unitToDelete = unit.second.second;
		}
	}
	if (unitToDelete) {
		m_units.erase(unitToDelete->GetId());
		delete unitToDelete;
	}
}

void UnitsManager::Cleanup() {
	std::lock_guard lockGuard(m_unitsMutex);

	for (auto unit : m_units)
		delete unit.second.second;
	m_units.clear();

	ResetUnits();
}

