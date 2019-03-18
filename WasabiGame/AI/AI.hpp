#pragma once

#include "../Common.hpp"
#include "../AI/AI.hpp"
#include "../Entities/Object.hpp"

#include <string>
#include <vector>

class AIFactory {
	struct AIEntry {
		class AI* ai;
		std::string name;
		AIEntry(class AI* a, std::string n) : name(n), ai(a) {}
	};
	static std::vector<AIEntry> dynamicAIs; //loaded per map
	static AI* defaultAI;

public:

	static void Init(void);
	static void Cleanup(void);
	static void AddAIEntry(AI* ai, std::string name);
	static void ClearAIEntries();
	static AI* CreateAI(std::string name);
};

class AI {
public:
	AI(void);
	~AI(void);

	virtual void Apply(Object* obj, float fDeltaTime);
	virtual AI* Clone(void) const;
};

class UnitAI : public AI {
protected:
	struct MOVEMENT_STRUCT {
		bool bJump;
		int forward;
		int right;
		float fAng;
		float fActualAng;
		bool bMoving;
		bool bEnabled;
		float fSpeed;
	} movement;

	float fH, fR;
	/*class hkpCharacterContext* characterContext;
	class hkpCharacterRigidBody* characterRigidBody;*/
	bool onGround;

public:
	UnitAI(float fHeight, float fRadius, bool bReference = false);
	~UnitAI(void);

	virtual void Step(Object* obj, float fDeltaTime);
	virtual AI* Clone(void) const;

	void SetPosition(WVector3 pos);
	virtual void Forward(bool bDo);
	virtual void Backward(bool bDo);
	virtual void Right(bool bDo);
	virtual void Left(bool bDo);
	virtual void Jump(bool bDo);
	virtual void SetAngle(float fAng);
	virtual void SetSpeed(float fSpeed);

	void DisableMotion(void);
	void EnableMotion(void);

	bool IsOnGround(void) const;

	void ApplyForce(WVector3 vec, float fDeltaTime);
	void SetLinearVelocity(WVector3 vec, float fDeltaTime);
};

class PlayerAI : public UnitAI {
	float fYaw;
	float fPitch;

public:
	PlayerAI(float fHeight, float fRadius);
	~PlayerAI(void);

	virtual void Step(Object* obj, float fDeltaTime);
	virtual AI* Clone(void) const;
};

class CreatureAI : public UnitAI {

protected:
	struct ATTRIBUTES {
		float fHitRange;
		float fAtenntionRange;
		Object* target;
	} attribs;

public:
	CreatureAI(float fHeight, float fRadius);
	~CreatureAI(void);

	virtual void Step(Object* obj, float fDeltaTime);
	virtual AI* Clone(void) const;
};

