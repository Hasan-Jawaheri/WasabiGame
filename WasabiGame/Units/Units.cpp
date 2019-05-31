#include "Units.hpp"
#include "AI.hpp"
#include "../ResourceManager/ResourceManager.hpp"

Unit::Unit() : m_model(nullptr), m_AI(nullptr) {
}

Unit::~Unit() {
	if (m_model) {
		ResourceManager::DestroyUnitModel(m_model);
		m_model = nullptr;
	}
	W_SAFE_DELETE(m_AI);
}

WOrientation* Unit::O() const {
	if (m_model->rb)
		return m_model->rb;
	return m_model->obj;
}

WRigidBody* Unit::RB() const {
	return m_model->rb;
}

void Unit::LoadModel(std::string modelName) {
	if (m_model) {
		ResourceManager::DestroyUnitModel(m_model);
		m_model = nullptr;
	}

	m_model = ResourceManager::LoadUnitModel(modelName);
}

void Unit::Update(float fDeltaTime) {
	if (m_AI)
		m_AI->Update(fDeltaTime);
}

AI* Unit::GetAI() const {
	return m_AI;
}
