#include "UnitsManager.hpp"

std::unordered_map<uint, std::function<class Unit* ()>> UnitsManager::m_unitGenerators;
std::unordered_map<Unit*, uint> UnitsManager::m_units;

void UnitsManager::RegisterUnit(uint id, std::function<class Unit* ()> unitGenerator) {
	m_unitGenerators.insert(std::make_pair(id, unitGenerator));
}

void UnitsManager::ResetUnits() {
	m_unitGenerators.clear();
}

Unit* UnitsManager::LoadUnit(uint type) {
	auto it = m_unitGenerators.find(type);
	Unit* unit = it->second();
	UnitsManager::m_units.insert(std::make_pair(unit, type));
	return unit;
}

void UnitsManager::DestroyUnit(Unit* unit) {
	auto it = m_units.find(unit);
	delete it->first;
	m_units.erase(it);
}

void UnitsManager::Update(float fDeltaTime) {
	for (auto unit : m_units)
		unit.first->Update(fDeltaTime);
}

