#pragma once

#include "Common.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"

#include <atomic>

class Unit {
	friend class UnitsManager;

	struct LoadInfo {
		std::string modelName;
		WVector3 spawnPos;
	} m_loadInfo;
	std::atomic<bool> m_canLoad;

	struct LOADED_MODEL* m_model;
	class AI* m_AI;
	uint m_id;

protected:
	Wasabi* m_app;
	ResourceManager* m_resourceManager;

	Unit(Wasabi* app, ResourceManager* resourceManager);
	virtual ~Unit();

	virtual void Update(float fDeltaTime);

public:

	uint GetId() const;
	Wasabi* GetApp() const;

	WOrientation* O() const;
	WRigidBody* RB() const;

	template<typename T> void SetAI() { W_SAFE_DELETE(m_AI); m_AI = new T(this); }
	class AI* GetAI() const;
};
