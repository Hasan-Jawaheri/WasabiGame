#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"

Unit::Unit(Wasabi* app, ResourceManager* resourceManager) : m_app(app), m_resourceManager(resourceManager), m_model(nullptr), m_AI(nullptr) {
	m_canLoad.store(false);
}

Unit::~Unit() {
	if (m_model) {
		m_resourceManager->DestroyUnitModel(m_model);
		m_model = nullptr;
	}
	W_SAFE_DELETE(m_AI);
}

uint Unit::GetId() const {
	return m_id;
}

Wasabi* Unit::GetApp() const {
	return m_app;
}

WOrientation* Unit::O() const {
	if (m_model) {
		if (m_model->rb)
			return m_model->rb;
		return m_model->obj;
	}
	return nullptr;
}

WRigidBody* Unit::RB() const {
	if (m_model)
		return m_model->rb;
	return nullptr;
}

void Unit::Update(float fDeltaTime) {
	if (!m_model && m_canLoad.load()) {
		m_model = m_resourceManager->LoadUnitModel(m_loadInfo.modelName);
		O()->SetPosition(m_loadInfo.spawnPos);
	}

	if (m_AI)
		m_AI->Update(fDeltaTime);
}

AI* Unit::GetAI() const {
	return m_AI;
}
