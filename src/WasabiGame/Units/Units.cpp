#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"

Unit::Unit(Wasabi* app, ResourceManager* resourceManager, UnitsManager* unitsManager) : m_app(app), m_resourceManager(resourceManager), m_unitsManager(unitsManager), m_model(nullptr), m_AI(nullptr) {
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

uint Unit::GetType() const {
	return m_type;
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

void Unit::OnLoaded() {
}

void Unit::Update(float fDeltaTime) {
	if (!m_model && m_canLoad.load()) {
		m_model = m_resourceManager->LoadUnitModel(m_loadInfo.modelName);
		if (m_model) {
			O()->SetPosition(m_loadInfo.spawnPos);
			OnLoaded();
		} else {
			// failed to load the model
			return;
		}
	}

	if (m_AI && m_model)
		m_AI->Update(fDeltaTime);
}

AI* Unit::GetAI() const {
	return m_AI;
}
