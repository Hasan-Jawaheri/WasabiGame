#include "WasabiGame/Units/UnitsManager.hpp"
#include "WasabiGame/Main.hpp"


WasabiGame::UnitsManager::UnitsManager(std::shared_ptr<WasabiBaseGame> app, std::shared_ptr<ResourceManager> resourceManager) {
	m_app = app;
	m_resourceManager = resourceManager;
}

WasabiGame::UnitsManager::~UnitsManager() {
}

void WasabiGame::UnitsManager::RegisterUnit(uint32_t id, std::function<std::shared_ptr<class Unit> ()> unitGenerator) {
	m_unitGenerators.insert(std::make_pair(id, unitGenerator));
}

void WasabiGame::UnitsManager::ResetUnits() {
	m_unitGenerators.clear();
}

std::shared_ptr<WasabiGame::Unit> WasabiGame::UnitsManager::LoadUnit(uint32_t type, uint32_t id, WVector3 spawnPos) {
	{
		std::scoped_lock lockGuard(m_unitsMutex);
		auto it = m_units.find(id);
		if (it != m_units.end())
			return it->second.second;
	}

	auto it = m_unitGenerators.find(type);
	std::shared_ptr<Unit> unit = it->second();
	unit->m_loadInfo.spawnPos = spawnPos;
	unit->m_canLoad.store(true);
	unit->m_id = id;
	unit->m_type = type;

	{
		std::scoped_lock lockGuard(m_unitsMutex);
		m_units.insert(std::make_pair(id, std::make_pair(type, unit)));
	}
	return unit;
}

std::shared_ptr<WasabiGame::Unit> WasabiGame::UnitsManager::GetUnit(uint32_t id) {
	std::scoped_lock lockGuard(m_unitsMutex);
	auto it = m_units.find(id);
	if (it != m_units.end()) {
		return it->second.second;
	}
	return nullptr;
}

void WasabiGame::UnitsManager::DestroyUnit(std::shared_ptr<Unit> unit) {
	DestroyUnit(unit->m_id);
}

void WasabiGame::UnitsManager::DestroyUnit(uint32_t id) {
	std::scoped_lock lockGuard(m_unitsMutex);
	auto it = m_units.find(id);
	if (it != m_units.end()) {
		m_units.erase(it);
	}
}

void WasabiGame::UnitsManager::Update(float fDeltaTime) {
	std::scoped_lock lockGuard(m_unitsMutex);
	std::shared_ptr<Unit> unitToDelete = nullptr;
	for (auto unit : m_units) {
		unit.second.second->Update(fDeltaTime);
		if (!unit.second.second->m_model && !unitToDelete) {
			// unit failed to load, only delete one per update
			unitToDelete = unit.second.second;
		}
	}
	if (unitToDelete)
		m_units.erase(unitToDelete->GetId());
}

void WasabiGame::UnitsManager::Cleanup() {
	std::scoped_lock lockGuard(m_unitsMutex);

	m_units.clear();

	ResetUnits();
}

