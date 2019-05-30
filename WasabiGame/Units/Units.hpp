#pragma once

#include "../Common.hpp"

class Unit {
	friend class UnitsManager;

	struct LOADED_MODEL* m_model;
	class AI* m_AI;

protected:
	Unit();
	virtual ~Unit();

	virtual void Update(float fDeltaTime);

public:

	WOrientation* O() const;

	void LoadModel(std::string modelName);

	template<typename T> void SetAI() { W_SAFE_DELETE(m_AI); m_AI = new T(this); }
	class AI* GetAI() const;
};
