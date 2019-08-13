#pragma once

#include "RollTheBall/Units/BallUnit.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "WasabiGame/Units/AI.hpp"

class Player : public BallUnit {
	friend class PlayerAI;

	WCamera* m_camera;
	WVector3 m_cameraPivot;
	float m_yaw, m_pitch, m_dist;
	bool m_draggingCamera;
	bool m_mouseHidden;
	int m_lastMouseX, m_lastMouseY;

	void ApplyMousePivot();

protected:

	virtual void Update(float fDeltaTime);

public:

	static const char* modelName;

	Player(Wasabi* app, ResourceManager* resourceManager);
	virtual ~Player();

	void BeginDragCamera();
	void EndDragCamera();
};

class PlayerAI : public AI {
	bool m_isJumpKeyDown;

public:
	PlayerAI(class Unit* unit);
	virtual ~PlayerAI();

	virtual void Update(float fDeltaTime);
};

