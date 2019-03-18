#include "AI.hpp"
#include "../Entities/Player.hpp"

/*
#include <Physics/Utilities/CharacterControl/StateMachine/hkpCharacterContext.h>
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBody.h>
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBodyListener.h>
#include <Physics/Utilities/CharacterControl/StateMachine/hkpDefaultCharacterStates.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>
*/

vector<AIFactory::AIEntry> AIFactory::dynamicAIs; //loaded per map
AI* AIFactory::defaultAI = nullptr;

void AIFactory::Init(void) {
	defaultAI = new CreatureAI(7.0f, 3.0f);
}

void AIFactory::Cleanup(void) {
	ClearAIEntries();
	delete defaultAI;
}

void AIFactory::AddAIEntry(AI* ai, std::string name) {
	dynamicAIs.push_back(AIEntry(ai, name));
}

void AIFactory::ClearAIEntries() {
	for (UINT i = 0; i < dynamicAIs.size(); i++)
		delete dynamicAIs[i].ai;
	dynamicAIs.clear();
}

AI* AIFactory::CreateAI(std::string name) {
	if (name == "")
		return defaultAI->Clone();

	for (UINT i = 0; i < dynamicAIs.size(); i++) {
		if (dynamicAIs[i].name == name) {
			return dynamicAIs[i].ai->Clone();
		}
	}
	return nullptr;
}

AI::AI(void) {
}

AI::~AI(void) {
}

void AI::Apply(Object* obj, float fDeltaTime) {
}

AI* AI::Clone(void) const {
	return new AI();
}

UnitAI::UnitAI(float fHeight, float fRadius, bool bReference) : AI() {
	fH = fHeight;
	fR = fRadius;
	ZeroMemory(&movement, sizeof MOVEMENT_STRUCT);
	movement.bEnabled = true;
	movement.fSpeed = 1.0f;
	/*
	if (!bReference) {
		APPHANDLE->PhysicsComponent->GetWorld()->lock();

		// Create a capsule to the character standing
		hkVector4 vertexA(0, fHeight, 0);
		hkVector4 vertexB(0, fRadius, 0);
		hkpShape* standShape = new hkpCapsuleShape(vertexA, vertexB, fRadius);

		// Construct a character rigid body
		hkpCharacterRigidBodyCinfo info;
		info.m_mass = 250.0f;
		info.m_friction = 0.0f;
		info.m_shape = standShape;

		info.m_maxForce = 50.0f;
		info.m_up = hkVector4(0, 1, 0);
		info.m_position.set(0, 0, 0);
		info.m_maxSlope = 35.0f * HK_REAL_DEG_TO_RAD;

		characterRigidBody = new hkpCharacterRigidBody(info);
		standShape->removeReference();
		hkpCharacterRigidBodyListener* listener = new hkpCharacterRigidBodyListener();
		characterRigidBody->setListener(listener);
		listener->removeReference();
		APPHANDLE->core->PhysicsDevice->GetWorld()->addEntity(characterRigidBody->getRigidBody());

		hkpCharacterState* state;
		hkpCharacterStateManager* manager = new hkpCharacterStateManager();

		state = new hkpCharacterStateOnGround();
		manager->registerState(state, HK_CHARACTER_ON_GROUND);
		state->removeReference();

		state = new hkpCharacterStateInAir();
		manager->registerState(state, HK_CHARACTER_IN_AIR);
		state->removeReference();

		state = new hkpCharacterStateJumping();
		manager->registerState(state, HK_CHARACTER_JUMPING);
		state->removeReference();

		state = new hkpCharacterStateClimbing();
		manager->registerState(state, HK_CHARACTER_CLIMBING);
		state->removeReference();

		characterContext = new hkpCharacterContext(manager, HK_CHARACTER_ON_GROUND);
		manager->removeReference();

		// Set character type
		characterContext->setCharacterType(hkpCharacterContext::HK_CHARACTER_RIGIDBODY);

		APPHANDLE->core->PhysicsDevice->GetWorld()->unlock();
	} else
		characterContext = nullptr;*/
}

UnitAI::~UnitAI(void) {
	/*if (characterContext) {
		APPHANDLE->core->PhysicsDevice->GetWorld()->lock();
		characterRigidBody->removeReference();
		delete characterContext;
		APPHANDLE->core->PhysicsDevice->GetWorld()->unlock();
	}*/
}

void UnitAI::Step(Object* obj, float fDeltaTime) {
	AI::Apply(obj, fDeltaTime);

	/*if (characterContext) {
		APPHANDLE->core->PhysicsDevice->GetWorld()->lock();

		hkpCharacterInput input;
		hkpCharacterOutput output;

		input.m_wantJump = movement.bJump && movement.bEnabled;
		input.m_atLadder = false;

		hkpCharacterStateType state = characterContext->getState();
		onGround = state == HK_CHARACTER_ON_GROUND;
		bool bMoving = (movement.forward || movement.right) && movement.bEnabled;// && (state == HK_CHARACTER_ON_GROUND);
		if (bMoving && !movement.bMoving)
			((Unit*)obj)->StartMoving();
		if (!bMoving && movement.bMoving)
			((Unit*)obj)->StopMoving();
		movement.bMoving = bMoving;

		hxVector3 l;
		float fDstAng = movement.forward == 1 ? (movement.right == -1 ? -45.0f : movement.right == 0 ? 0.0f : 45.0f) :
			movement.forward == 0 ? (movement.right == -1 ? -90.0f : 90.0f) :
			(movement.right == -1 ? -135.0f : movement.right == 0 ? 180.0f : 135.0f);
		fDstAng += movement.fAng;
		fDstAng = fmod(fDstAng, 360);
		float fDistToRight = movement.fActualAng > fDstAng ? (360 - movement.fActualAng) + fDstAng :
			fDstAng - movement.fActualAng;
		float fDistToLeft = movement.fActualAng > fDstAng ? movement.fActualAng - fDstAng :
			(360 - fDstAng) + movement.fActualAng;
		if (fDistToRight > fDistToLeft) {
			//go left
			if (fDistToLeft > 70.0f) { //need an immediate turn to left, angle too big
				movement.fActualAng = fDstAng;
			} else { //smooth turn
				movement.fActualAng -= fDistToLeft * 3.0f * fDeltaTime;
			}
		} else {
			//go right
			if (fDistToRight > 70.0f) { //need an immediate turn to right, angle too big 
				movement.fActualAng = fDstAng;
			} else { //smooth turn
				movement.fActualAng += fDistToRight * 3.0f * fDeltaTime;
			}
		}
		movement.fActualAng = fmod(movement.fActualAng, 360);
		if (bMoving)
			((Unit*)obj)->SetAngle(movement.fActualAng);
		hxMatrix ml = hxRotationMatrixY(HX_DEGTORAD(movement.fActualAng));
		l = hxVec3TransformNormal(hxVec3Normalize(hxVector3(0, 0, 1)), ml);

		input.m_up = hkVector4(0, 1, 0);
		input.m_forward.set(l.x, l.y, l.z);
		input.m_inputLR = 0;
		input.m_inputUD = (bMoving ? -1 : 0) * movement.fSpeed;// 0.185f;

		hkStepInfo stepInfo;
		stepInfo.m_deltaTime = fDeltaTime;
		stepInfo.m_invDeltaTime = 1.0f / fDeltaTime;

		input.m_stepInfo = stepInfo;

		input.m_characterGravity.set(0, -20, 0);
		if (!bMoving && state == HK_CHARACTER_ON_GROUND)
			characterRigidBody->getRigidBody()->setLinearVelocity(hkVector4(0, 0, 0));
		input.m_velocity = characterRigidBody->getRigidBody()->getLinearVelocity();
		input.m_position = characterRigidBody->getRigidBody()->getPosition();

		characterRigidBody->checkSupport(stepInfo, input.m_surfaceInfo);

		// Apply the character state machine
		characterContext->update(input, output);

		//Apply the player character controller
		// Set output velocity from state machine into character rigid body
		characterRigidBody->setLinearVelocity(output.m_velocity, fDeltaTime);

		obj->GetOrientation()->SetPosition(input.m_position(0), input.m_position(1), input.m_position(2));

		APPHANDLE->core->PhysicsDevice->GetWorld()->unlock();
	}*/
}

AI* UnitAI::Clone(void) const {
	return new UnitAI(fH, fR);
}

void UnitAI::SetPosition(WVector3 pos) {
	/*APPHANDLE->core->PhysicsDevice->GetWorld()->lock();
	characterRigidBody->getRigidBody()->setPosition(hkVector4(pos.x, pos.y, pos.z));
	APPHANDLE->core->PhysicsDevice->GetWorld()->unlock();*/
}

void UnitAI::Forward(bool bMove) {
	movement.forward += bMove ? 1 : -1;
	movement.forward = min(max(-1, movement.forward), 1);
}

void UnitAI::Backward(bool bMove) {
	movement.forward += bMove ? -1 : 1;
	movement.forward = min(max(-1, movement.forward), 1);
}

void UnitAI::Left(bool bMove) {
	movement.right += bMove ? -1 : 1;
	movement.right = min(max(-1, movement.right), 1);
}

void UnitAI::Right(bool bMove) {
	movement.right += bMove ? 1 : -1;
	movement.right = min(max(-1, movement.right), 1);
}

void UnitAI::Jump(bool bDo) {
	movement.bJump = bDo;
}

void UnitAI::SetAngle(float fAng) {
	movement.fAng = fAng;
}

void UnitAI::DisableMotion(void) {
	movement.bEnabled = false;
	movement.bMoving = false;
}

void UnitAI::EnableMotion(void) {
	movement.bEnabled = true;
}

bool UnitAI::IsOnGround(void) const {
	return onGround;
}

void UnitAI::SetSpeed(float fSpeed) {
	movement.fSpeed = fSpeed;
}

void UnitAI::ApplyForce(WVector3 vec, float fDeltaTime) {
	/*APPHANDLE->core->PhysicsDevice->GetWorld()->lock();
	hkVector4 v;
	v(0) = vec.x;
	v(1) = vec.y;
	v(2) = vec.z;
	characterRigidBody->getRigidBody()->applyForce(fDeltaTime, v);
	APPHANDLE->core->PhysicsDevice->GetWorld()->unlock();*/
}

void UnitAI::SetLinearVelocity(WVector3 vec, float fDeltaTime) {
	/*APPHANDLE->core->PhysicsDevice->GetWorld()->lock();
	hkVector4 v;
	v(0) = vec.x;
	v(1) = vec.y;
	v(2) = vec.z;
	characterRigidBody->setLinearVelocity(v, fDeltaTime);
	APPHANDLE->core->PhysicsDevice->GetWorld()->unlock();*/
}

PlayerAI::PlayerAI(float fHeight, float fRadius) : UnitAI(fHeight, fRadius) {
	fYaw = 0.0f;
	fPitch = 30.0f;
}

PlayerAI::~PlayerAI(void) {
}

void PlayerAI::Step(Object* obj, float fDeltaTime) {
	WCamera* cam = APPHANDLE->CameraManager->GetDefaultCamera();

	float fSensitivity = 15.0f; //1-20:high-low
	static bool bMouseHidden = false;
	static int lx, ly;
	if (!bMouseHidden) {
		ShowCursor(FALSE);
		bMouseHidden = true;

		lx = APPHANDLE->InputComponent->MouseX(MOUSEPOS_DESKTOP, 0);
		ly = APPHANDLE->InputComponent->MouseY(MOUSEPOS_DESKTOP, 0);

		POINT pt = { 640 / 2, 480 / 2 };
		SetCursorPos(pt.x, pt.y);
	}

	int mx = APPHANDLE->InputComponent->MouseX(MOUSEPOS_VIEWPORT, 0);
	int my = APPHANDLE->InputComponent->MouseY(MOUSEPOS_VIEWPORT, 0);

	int dx = mx - 640 / 2;
	int dy = my - 480 / 2;

	fYaw += (float)dx / fSensitivity;
	fPitch += (float)dy / fSensitivity;

	if (dx || dy) {
		POINT pt = { 640 / 2, 480 / 2 };
		SetCursorPos(pt.x, pt.y);
	}

	if (fPitch < -30)
		fPitch = -30;
	if (fPitch > 90)
		fPitch = 90;

	WOrientation orientation;
	orientation.SetPosition(obj->GetOrientation()->GetPosition() + WVector3(0, 10.0f, 0));
	orientation.SetAngle(0, 0, 0);
	orientation.Yaw(fYaw);
	orientation.Pitch(fPitch);
	orientation.Move(-20);
	orientation.Pitch(-fPitch / 4);
	float fDist = WVec3Length(cam->GetPosition() - orientation.GetPosition());
	float fMoveSpeed = fDist * fDist * 0.01f + 1.0f * fDeltaTime * 500.0f;
	if (fDist > fMoveSpeed) {
		cam->Point(orientation.GetPosition());
		cam->Move(fMoveSpeed);
	} else
		cam->SetPosition(orientation.GetPosition());
	cam->SetToRotation(&orientation);
	movement.fAng = fYaw;
	//((Player*)obj)->SetLookVector ( hxVector2 ( cam->GetLVector().x, cam->GetLVector().z ) );

	UnitAI::Apply(obj, fDeltaTime);
}

AI* PlayerAI::Clone(void) const {
	return new PlayerAI(fH, fR);
}

CreatureAI::CreatureAI(float fHeight, float fRadius) : UnitAI(fHeight, fRadius) {
	attribs.fHitRange = 20.0f;
	attribs.fAtenntionRange = 250.0f;
	attribs.target = nullptr;
}

CreatureAI::~CreatureAI(void) {
}

void CreatureAI::Step(Object* obj, float fDeltaTime) {
	Object* player = UnitManager::GetPlayer();
	Unit* unit = (Unit*)obj;
	if (!attribs.target) {
		WOrientation* player_orientatiion = player->GetOrientation();
		if (WVec3LengthSq(player_orientatiion->GetPosition() - obj->GetOrientation()->GetPosition()) <
			attribs.fAtenntionRange * attribs.fAtenntionRange)
			attribs.target = player;
		movement.forward = 0;
	} else {
		WOrientation* target_orientation = attribs.target->GetOrientation();
		WVector3 l = target_orientation->GetPosition() - unit->GetOrientation()->GetPosition();

		float fAng;
		WVector3 _2dLook(l.x, 0.0f, l.z);
		float dot = WVec3Dot(WVector3(0, 0, 1), _2dLook); //get angle between the vectors
		float u = 1.0f;
		float v = sqrt(float(_2dLook.x*_2dLook.x + _2dLook.y*_2dLook.y + _2dLook.z*_2dLook.z));
		if (v == 0)
			fAng = 0.0f;
		else {
			fAng = acos(float(dot / (u*v)));
			if (l.x < 0.0f) //flip the angle if the object is in opposite direction
				fAng = -fAng;
		}
		movement.fAng = W_RADTODEG(fAng);

		if (WVec3LengthSq(l) > attribs.fHitRange * attribs.fHitRange)
			movement.forward = 1;
		else
			movement.forward = 0;
	}
	UnitAI::Apply(obj, fDeltaTime);
}

AI* CreatureAI::Clone(void) const {
	return new CreatureAI(fH, fR);
}

