#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/Player.hpp"

class PlayerAI : public RTBAI {
	bool m_isJumpKeyDown;

	WCamera* m_camera;
	WVector3 m_cameraPivot;
	float m_yaw, m_pitch, m_dist;
	bool m_draggingCamera;
	bool m_mouseHidden;
	int m_lastMouseX, m_lastMouseY;

	void ApplyMousePivot();

public:
	PlayerAI(class Unit* unit);
	virtual ~PlayerAI();

	virtual void Update(float fDeltaTime);

	void BeginDragCamera();
	void EndDragCamera();
};