#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Main.hpp"
#include "WasabiGame/Units/AI.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"


WasabiGame::Unit::Unit(std::shared_ptr<WasabiBaseGame> app, std::shared_ptr<ResourceManager> resourceManager, std::shared_ptr<UnitsManager> unitsManager) : std::enable_shared_from_this<WasabiGame::Unit>(), m_app(app), m_resourceManager(resourceManager), m_unitsManager(unitsManager), m_model(nullptr), m_AI(nullptr) {
	m_canLoad.store(false);
}

WasabiGame::Unit::~Unit() {
	if (m_model) {
		m_resourceManager->DestroyUnitModel(m_model);
		m_model = nullptr;
	}
}

uint32_t WasabiGame::Unit::GetId() const {
	return m_id;
}

uint32_t WasabiGame::Unit::GetType() const {
	return m_type;
}

std::weak_ptr<WasabiGame::WasabiBaseGame> WasabiGame::Unit::GetApp() const {
	return m_app;
}

WOrientation* WasabiGame::Unit::O() const {
	if (m_model) {
		if (m_model->rb)
			return m_model->rb;
		return m_model->obj;
	}
	return nullptr;
}

WRigidBody* WasabiGame::Unit::RB() const {
	if (m_model)
		return m_model->rb;
	return nullptr;
}

void WasabiGame::Unit::OnLoaded() {
}

void WasabiGame::Unit::Update(float fDeltaTime) {
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

std::shared_ptr<WasabiGame::AI> WasabiGame::Unit::GetAI() const {
	return m_AI;
}
