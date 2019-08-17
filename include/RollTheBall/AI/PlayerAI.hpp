#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/Player.hpp"

#include "RollTheBall/Networking/Data.hpp"
#include "RollTheBall/Networking/Protocol.hpp"

class PlayerAI : public RTBAI {
	bool m_isJumpKeyDown;

	Wasabi* m_app;
	WCamera* m_camera;
	WVector3 m_cameraPivot;
	float m_yaw, m_pitch, m_dist;
	bool m_draggingCamera;
	bool m_mouseHidden;
	int m_lastMouseX, m_lastMouseY;

	float m_updateTimer;
	RPGNet::NetworkUpdate m_unitUpdate;

	void ApplyMousePivot();

public:
	PlayerAI(class Unit* unit);
	virtual ~PlayerAI();

	virtual void Update(float fDeltaTime);

	void BeginDragCamera();
	void EndDragCamera();
};