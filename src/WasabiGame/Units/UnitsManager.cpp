#include "WasabiGame/Units/UnitsManager.hpp"

std::unordered_map<uint, std::function<class Unit* ()>> UnitsManager::m_unitGenerators;
std::unordered_map<uint, std::pair<uint, Unit*>> UnitsManager::m_units;

void UnitsManager::RegisterUnit(uint id, std::function<class Unit* ()> unitGenerator) {
	m_unitGenerators.insert(std::make_pair(id, unitGenerator));
}

void UnitsManager::ResetUnits() {
	m_unitGenerators.clear();
}

Unit* UnitsManager::LoadUnit(uint type, uint id) {
	auto it = m_unitGenerators.find(type);
	Unit* unit = it->second();
	unit->m_id = id;
	UnitsManager::m_units.insert(std::make_pair(type, std::make_pair(id, unit)));
	return unit;
}

Unit* UnitsManager::GetUnit(uint id) {
	auto it = m_units.find(id);
	if (it != m_units.end()) {
		return it->second.second;
	}
	return nullptr;
}

void UnitsManager::DestroyUnit(Unit* unit) {
	auto it = m_units.find(unit->m_id);
	if (it != m_units.end()) {
		delete it->second.second;
		m_units.erase(it);
	}
}

void UnitsManager::Update(float fDeltaTime) {
	for (auto unit : m_units)
		unit.second.second->Update(fDeltaTime);
}

