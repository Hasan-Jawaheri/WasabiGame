#pragma once

#include "../Common.hpp"

class Unit {
	class LOADED_MODEL* m_model;

public:
	Unit();
	virtual ~Unit();

	virtual void LoadModel(std::string modelName);
	virtual void Update(float fDeltaTime);
};
