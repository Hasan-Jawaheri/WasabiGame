#pragma once

#include "Common.hpp"

class Unit {
	friend class UnitsManager;

	struct LOADED_MODEL* m_model;
	class AI* m_AI;
	uint m_id;

protected:
	Unit();
	virtual ~Unit();

	virtual void Update(float fDeltaTime);

public:

	uint GetId() const;

	WOrientation* O() const;
	WRigidBody* RB() const;

	void LoadModel(std::string modelName);

	template<typename T> void SetAI() { W_SAFE_DELETE(m_AI); m_AI = new T(this); }
	class AI* GetAI() const;
};
