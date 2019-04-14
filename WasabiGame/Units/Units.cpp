#include "Units.hpp"
#include "../ResourceManager/ResourceManager.hpp"

Unit::Unit() : m_model(nullptr) {
}

Unit::~Unit() {
}

void Unit::LoadModel(std::string modelName) {
	m_model = ResourceManager::LoadUnitModel(modelName);
}

void Unit::Update(float fDeltaTime) {
}
