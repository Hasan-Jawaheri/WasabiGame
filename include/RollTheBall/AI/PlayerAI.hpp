#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/Player.hpp"


namespace RollTheBall {
	
	class PlayerAI : public RTBAI {
		WCamera* m_camera;
		WVector3 m_cameraPivot;
		float m_cameraPitch, m_cameraDistance;

	public:
		PlayerAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~PlayerAI();

		virtual void Update(float fDeltaTime);

		void SetCameraPitch(float pitch);
		void SetCameraDistance(float distance);
		float GetCameraPitch() const;
		float GetCameraDistance() const;
	};

};
