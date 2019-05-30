#pragma once

#include "../Common.hpp"
#include "Units.hpp"

class UnitsManager {
	static std::unordered_map<uint, std::function<class Unit* ()>> m_unitGenerators;
	static std::unordered_map<class Unit*, uint> m_units;

public:

	template<typename U, typename A>
	static U* CreateUnitAndAI(std::string modelName) {
		U* u = new U();
		u->SetAI<A>();
		u->LoadModel(modelName);
		return u;
	}

	static void RegisterUnit(uint id, std::function<class Unit* ()> unitGenerator);
	static void ResetUnits();

	static Unit* LoadUnit(uint type);
	static void DestroyUnit(Unit* unit);
	static void Update(float fDeltaTime);
};
