#include "Unit.hpp"
#include "../ResourceManager/ResourceManager.hpp"

const char* boneN[] = {
	"Bip001 Head",
	"Bip001 Neck",
	"Bip001 L Clavicle",
	"",
	"Bip001 L Forearm",
	"Bip001 L Forearm",
	"Bip001 Spine",
	"",
	"Bip001 L Calf",
	"Bip001 L Finger0Nub",
	"Bip001 R Finger0Nub",
	"",
	"",
	"Bip001 R Hand",
	"Bip001 L Hand",
	"Bip001 R Hand",
	""
};
const char* boneNExtra[] = {
	"",
	"",
	"Bip001 R Clavicle",
	"",
	"Bip001 R Forearm",
	"Bip001 R Forearm",
	"Bip001 Pelvis",
	"",
	"Bip001 R Calf",
	"Bip001 R Finger0Nub",
	"Bip001 L Finger0Nub",
	"",
	"",
	"Bip001 L Hand",
	"Bip001 R Hand",
	"Bip001 L Hand",
	""
};

WVector3 GetBindingPosition(WOrientation* orientation) {
	return WVec3TransformCoord(WVector3(), orientation->GetBindingMatrix());
}

vector<Unit*> UnitManager::units;
FILE* UnitManager::unitsFile = nullptr;
FILE* UnitManager::equipmentFile = nullptr;
UnitManager::UNITSFILEHEADER UnitManager::unitsFileHeader;
Player* UnitManager::player = nullptr;

void UnitManager::Initialize() {
	fopen_s(&unitsFile, "Media\\Data\\units", "rb");
	if (unitsFile) {
		fread(&unitsFileHeader.numUnits, 4, 1, unitsFile);
		UINT* positions = new UINT[unitsFileHeader.numUnits];
		fread(positions, 4, unitsFileHeader.numUnits, unitsFile);
		for (UINT i = 0; i < unitsFileHeader.numUnits; i++)
			unitsFileHeader.unitPos.push_back(positions[i]);
		delete[] positions;
	}
	fopen_s(&equipmentFile, "Media\\Data\\equipment", "rb");
}

void UnitManager::Update(float fDeltaTime) {
	for (int i = 0; i < units.size(); i++) {
		if (!units[i]->Update(fDeltaTime)) {
			delete units[i];
			i--;
		}
	}
}

void UnitManager::Cleanup() {
	if (unitsFile)
		fclose(unitsFile);
}

Unit* UnitManager::LoadUnit(UINT unitID, Unit* premade) {
	if (premade)
		unitID = premade->__ID;

	if (unitsFile && unitID < unitsFileHeader.unitPos.size()) {
		UNITDATA data;
		fseek(unitsFile, unitsFileHeader.unitPos[unitID], 0);
		UINT k = fread((char*)&data, UNITDATASIZENOVECTOR, 1, unitsFile);
		UINT numAnim;
		fread((char*)&numAnim, sizeof UINT, 1, unitsFile);
		if (numAnim) {
			ANIMATIONRANGE* ranges = new ANIMATIONRANGE[numAnim];
			fread(ranges, numAnim * sizeof ANIMATIONRANGE, 1, unitsFile);
			for (UINT i = 0; i < numAnim; i++)
				data.animationData.push_back(ranges[i]);
			delete[] ranges;
		}

		Unit* u;
		if (premade)
			u = premade;
		else
			u = new Unit(data.modelID, data.skeletonID);
		u->Load();
		u->baseStats.maxHealth = data.health;
		u->baseStats.health = data.health;
		u->baseStats.mana = data.mana;
		u->baseStats.maxMana = data.mana;
		memcpy(&u->curStats, &u->baseStats, sizeof STATS_STRUCT);
		u->Scale(data.scale);
		u->ClearAnimationRange();
		for (UINT i = 0; i < data.animationData.size(); i++)
			u->AddAnimationRange(data.animationData[i]);
		u->SetAI((UnitAI*)AIFactory::CreateAI(data.AIName));
		return u;
	}
	return nullptr;
}

bool UnitManager::UnitExists(Unit* unit) {
	for (UINT i = 0; i < units.size(); i++)
		if (units[i] == unit)
			return true;
	return false;
}

void UnitManager::DealDamage(Unit* from, Unit* to, DAMAGE_STRUCT dmg) {
	if (!to)
		return;

	float fAtkDmg = dmg.fBaseAttackDmg;
	float fFireDmg = dmg.fBaseFireDmg;
	float fFrostDmg = dmg.fBaseFrostDmg;
	float fNatureDmg = dmg.fBaseNatureDmg;
	if (from) {
		fAtkDmg += dmg.attackDmgMul * from->curStats.attackDmg;
		fFireDmg += dmg.fireDmgMul * from->curStats.magicDmg;
		fFrostDmg += dmg.frostDmgMul * from->curStats.magicDmg;
		fNatureDmg += dmg.natureDmgMul * from->curStats.magicDmg;
	}
	float fArmor = to->curStats.armor;
	float fFireR = to->curStats.fireResist;
	float fFrostR = to->curStats.frostResist;
	float fNatureR = to->curStats.natureResist;
	if (from) {
		fArmor = max(0, fArmor - from->curStats.armorPen);
		fFireR = max(0, fFireR - from->curStats.magicPen);
		fFrostR = max(0, fFrostR - from->curStats.magicPen);
		fNatureR = max(0, fNatureR - from->curStats.magicPen);
	}
	float fFinalDmg = max(0, fAtkDmg - fArmor) + max(0, fFireDmg - fFireR) +
		max(0, fFrostDmg - fFrostR) + max(0, fNatureDmg - fNatureR);
	to->curStats.health = max(0, to->curStats.health - fFinalDmg);

	if (to->curStats.health < 0.002f) {
		//kill 'to'
		to->OnDeath();
	}
}

void UnitManager::DealDamage(Unit* from, float fRange, DAMAGE_STRUCT dmg) {
	if (!from)
		return;

	float fRangeSq = fRange * fRange;
	for (UINT i = 0; i < units.size(); i++) {
		if (units[i] != from && WVec3LengthSq(from->GetOrientation()->GetPosition() -
			units[i]->GetOrientation()->GetPosition()) < fRangeSq) {
			DealDamage(from, units[i], dmg);
		}
	}
}

void UnitManager::DealDamage(Unit* from, WVector3 pos, float fRange, DAMAGE_STRUCT dmg) {
	float fRangeSq = fRange * fRange;
	for (UINT i = 0; i < units.size(); i++) {
		if (units[i] != from && WVec3LengthSq
		(pos - units[i]->GetOrientation()->GetPosition()) < fRangeSq) {
			DealDamage(from, units[i], dmg);
		}
	}
}

Unit* UnitManager::FindTarget(WVector2 point, float maxDist) {
	Unit* closest = nullptr;
	int cx = 2000, cy = 2000;
	for (UINT i = 1; i < units.size(); i++) { //unit 0 is player, ignore
		int x, y;
		units[i]->GetOrientation()->Fly(10); //origin is lower than body center, attempt to balance...
		WUtil::Point3DToScreen2D(APPHANDLE, units[i]->GetOrientation()->GetPosition(), &x, &y);
		units[i]->GetOrientation()->Fly(-10);
		x -= point.x;
		y -= point.y;
		if (x * x + y * y < cx * cx + cy * cy) {
			closest = units[i];
			cx = x;
			cy = y;
		}
	}
	if (cx * cx + cy * cy < maxDist * maxDist)
		return closest;
	return nullptr;
}

Player* UnitManager::GetPlayer() {
	return player;
}

void UnitManager::SetPlayer(Player* p) {
	player = p;
}

EquippedItem::EquippedItem(Unit* o, EQUIPMENTDATA d) : Object(0, TYPE_EQUIPMENT) { //always ID 0
	data = d;
	owner = o;
	objExtra = nullptr;
}

EquippedItem::~EquippedItem() {
	Destroy();
}

void EquippedItem::Load() {
	obj = ResourceManager::AllocModel(data.modelID[0]).obj;
	if (data.modelID[1] != -1) {
		objExtra = ResourceManager::AllocModel(data.modelID[1]).obj;
	}
}

bool EquippedItem::Update(float fDeltaTime) {
	if (owner && UnitManager::UnitExists(owner) && obj) {
		GetOrientation()->SetPosition(owner->GetOrientation()->GetPosition());
		GetOrientation()->SetToRotation(owner->GetOrientation());
		if (data.modelID[1] != -1) {
			objExtra->SetPosition(owner->GetOrientation()->GetPosition());
			objExtra->SetToRotation(owner->GetOrientation());
		}
		return true;
	} else
		return false;
}

void EquippedItem::Destroy() {
	ResourceManager::ReleaseModel(obj, data.modelID[0]);
	if (objExtra)
		ResourceManager::ReleaseModel(objExtra, data.modelID[1]);
	objExtra = obj = nullptr;
}

bool EquippedItem::HasModel() const {
	return obj;
}

Unit::Unit(UINT mID, UINT sID) : Object(0, TYPE_UNIT) {
	modelID = mID;
	skeletonID = sID;
	baseStats = STATS_STRUCT();
	baseStats.health = baseStats.maxHealth = 100.0f;
	baseStats.energy = baseStats.maxEnergy = 100.0f;
	baseStats.mana = baseStats.maxMana = 100.0f;
	baseStats.moveSpeed = SPEED;
	memcpy(&curStats, &baseStats, sizeof STATS_STRUCT);

	for (UINT i = 0; i < NUM_ITEM_SLOTS; i++)
		equipment.itemSlots[i] = nullptr;

	bAIEnabled = true;

	bDead = false;

	anim = nullptr;
	comboSys = nullptr;

	UnitManager::units.push_back(this);
}

Unit::~Unit() {
	for (UINT i = 0; i < UnitManager::units.size(); i++) {
		if (UnitManager::units[i] == this) {
			UnitManager::units.erase(UnitManager::units.begin() + i);
			break;
		}
	}
	Destroy();
}

void Unit::Load() {
	ALLOCATED_UNIT u = ResourceManager::AllocUnit(modelID, skeletonID);
	obj = u.obj;
	anim = u.skeleton;
	__ID = u.ID;

	ai = nullptr;
	InitAnimation();
	comboSys = new ComboSystem();
}

void Unit::Destroy() {
	//equipment will destroy itself with no  owner
	for (UINT i = 0; i < NUM_ITEM_SLOTS; i++)
		Unequip((ITEMSLOT)i);
	W_SAFE_DELETE(ai);

	for (UINT i = 0; i < UnitManager::units.size(); i++) {
		if (UnitManager::units[i] == this) {
			UnitManager::units.erase(UnitManager::units.begin() + i);
			break;
		}
	}

	ALLOCATED_UNIT u;
	u.obj = obj;
	u.skeleton = anim;
	u.ID = __ID;
	ResourceManager::ReleaseUnit(u, modelID, skeletonID);
	obj = nullptr;
	anim = nullptr;
}

bool Unit::Update(float fDeltaTime) {
	UpdateAnimation();

	if (ai && bAIEnabled) {
		ai->SetSpeed(baseStats.moveSpeed / 162.1621621621622);
		ai->Step(this, fDeltaTime);
	}

	baseStats.health = curStats.health;
	baseStats.mana = curStats.mana;
	baseStats.energy = curStats.energy;
	memcpy(&curStats, &baseStats, sizeof STATS_STRUCT);

	//update equipped items
	for (UINT i = 0; i < NUM_ITEM_SLOTS; i++)
		if (equipment.itemSlots[i])
			equipment.itemSlots[i]->Update(fDeltaTime);

	return !bDead;
}

ANIMATIONRANGE Unit::SetAnimationState(ANIMSTATE st, UINT i1, UINT i2, float fSpeed,
	ANIMPART body, LPCSTR name, bool bForceLoop) {
	if (anim) {
		bool bLoop = true;
		char animName[64];
		switch (st) {
		case IDLE: strcpy_s(animName, 64, "idle"); break;
		case STARTED_MOVING: strcpy_s(animName, 64, "walk"); bLoop = false; break;
		case MOVING: strcpy_s(animName, 64, "walk"); break;
		case CASTING: strcpy_s(animName, 64, name); bLoop = bForceLoop; break;
		}
#ifdef _DEBUG
		cout << animName << ": " << fSpeed << ", " << name << ", " <<
			((body & LOWER_BODY) ? "L" : "-") << "|" <<
			((body & UPPER_BODY) ? "U" : "-") << "\n";
#endif
		for (UINT i = 0; i < animationRanges.size(); i++) {
			if (strcmp(animationRanges[i].name, animName) == 0) {
				if (body & LOWER_BODY) {
					anim->SetPlayingBounds(animationRanges[i].params[i1], animationRanges[i].params[i2], 0);
					anim->SetCurrentFrame(animationRanges[i].params[i1], 0);
					anim->SetPlaySpeed(fSpeed, 0);
					if (bLoop)
						anim->Loop(0);
					else
						anim->Play(0);
					animStateL = st;
				}
				if (body & UPPER_BODY) {
					anim->SetPlayingBounds(animationRanges[i].params[i1], animationRanges[i].params[i2], 1);
					anim->SetCurrentFrame(animationRanges[i].params[i1], 1);
					anim->SetPlaySpeed(fSpeed, 1);
					if (bLoop)
						anim->Loop(1);
					else
						anim->Play(1);
					animStateH = st;
				}
				return animationRanges[i];
			}
		}
	}
	return ANIMATIONRANGE(); //clear warning
}

void Unit::AddAnimationRange(ANIMATIONRANGE range) {
	animationRanges.push_back(range);
}

void Unit::ClearAnimationRange() {
	animationRanges.clear();
}

void Unit::StartMoving() {
	if (animStateH == CASTING)
		SetAnimationState(STARTED_MOVING, 0, 2, SPEED / 10.0f, LOWER_BODY);
	else
		SetAnimationState(STARTED_MOVING, 0, 2);
}

void Unit::StopMoving() {
	if (animStateH == CASTING)
		SetAnimationState(IDLE, 0, 1, SMSPEED, LOWER_BODY);
	else
		SetAnimationState(IDLE, 0, 1, SMSPEED);
}

void Unit::SetAngle(float fAng) {
	WMatrix ml = WRotationMatrixY(W_DEGTORAD(fAng));
	WVector3 l = WVec3TransformNormal(WVector3(0, 0, 1), ml);
	WVector3 r = WVec3TransformNormal(WVector3(1, 0, 0), ml);
	obj->SetULRVectors(WVector3(0, 1, 0), l, r);
}

void Unit::OnDeath() {
	bDead = true;
}

void Unit::InitAnimation() {
	SetAnimationState(IDLE, 0, 1, SMSPEED);
}

void Unit::UpdateAnimation() {
	if (anim) {
		/*if ( animStateL == STARTED_MOVING && !anim->Playing ( 0 ) )
			SetAnimationState ( MOVING, 1, 2, SPEED, LOWER_BODY );
		if ( animStateH == STARTED_MOVING && !anim->Playing ( 1 ) )
			SetAnimationState ( MOVING, 1, 2, SPEED, UPPER_BODY );*/
		if (animStateL == STARTED_MOVING) {
			for (UINT i = 0; i < animationRanges.size(); i++) {
				if (strcmp(animationRanges[i].name, "walk") == 0) {
					baseStats.moveSpeed = (anim->GetTime(0) / (float)animationRanges[i].params[1]) * SPEED;
					if (anim->GetTime(0) > animationRanges[i].params[1]) {
						anim->SetPlayingBounds(animationRanges[i].params[1], animationRanges[i].params[2], 0);
						anim->SetPlaySpeed(SPEED, 0);
						anim->Loop(0);
						animStateL = MOVING;
						baseStats.moveSpeed = SPEED;
					}
				}
			}
		}
	}
}

bool Unit::StartSpellAnimation(LPCSTR animName, float fSpeed, bool bForceWholeBody,
	ANIMATIONRANGE* range, UINT l1, UINT l2, bool bLoop) {
	if (anim) {
		ANIMPART body = (ANIMPART)(bForceWholeBody ? (int)(LOWER_BODY | UPPER_BODY) :
			(int)(UPPER_BODY | ((animStateL == MOVING || animStateL == STARTED_MOVING) ? UPPER_BODY : LOWER_BODY)));
		ANIMATIONRANGE r = SetAnimationState(CASTING, l1, l2, fSpeed, body, animName, bLoop);
		if (range)
			*range = r;
		return body & LOWER_BODY;
		/*
		for ( UINT i = 0; i < animationRanges.size ( ); i++ ) {
			if ( strcmp ( animationRanges[i].name, animName ) == 0 ) {
				bool bMovedLowerBody = false;
				if ( !anim->Playing ( 0 ) || bForceWholeBody ) {
					anim->SetPlayingBounds ( animationRanges[i].params[0], animationRanges[i].params[1], 0 );
					anim->SetCurrentFrame ( animationRanges[i].params[0], 0 );
					anim->SetPlaySpeed ( fSpeed, 0 );
					anim->Play ( 0 );
					bMovedLowerBody = true;
				}
				anim->SetPlayingBounds ( animationRanges[i].params[0], animationRanges[i].params[1], 1 );
				anim->SetCurrentFrame ( animationRanges[i].params[0], 1 );
				anim->SetPlaySpeed ( fSpeed, 1 );
				anim->Play ( 1 );
				bSpellAnim = true;
				if ( range )
					*range = animationRanges[i];
				return bMovedLowerBody;
			}
		}*/
	}

	return false;
}

void Unit::StopSpellAnimation() {
	if (anim) {
		ANIMSTATE st = animStateH == CASTING ? IDLE : animStateL;
		if (st == IDLE) {
			if (animStateL != CASTING) {
				if (animStateL == IDLE)
					SetAnimationState(IDLE, 0, 1, SMSPEED, UPPER_BODY);
				else
					SetAnimationState(animStateL, 1, 2, SPEED, UPPER_BODY);
			} else
				SetAnimationState(st, 0, 1, SMSPEED);
		} else {
			if (animStateL != CASTING)
				SetAnimationState(st, 0, 1, SPEED, UPPER_BODY);
			else
				SetAnimationState(st);
		}
	}
}
WSkeleton* Unit::GetAnimation() const {
	return anim;
}

ComboSystem* Unit::GetComboSystem() const {
	return comboSys;
}

void Unit::Equip(ITEMSLOT slot, UINT itemID) {
	Unequip(slot);

	EQUIPMENTDATA data;
	fseek(UnitManager::equipmentFile, itemID * sizeof EQUIPMENTDATA, SEEK_SET);
	fread(&data, sizeof EQUIPMENTDATA, 1, UnitManager::equipmentFile);

	equipment.itemSlots[slot] = new EquippedItem(this, data);
	EquippedItem* item = equipment.itemSlots[slot];
	item->Load();

	if (anim && item->HasModel()) {
		if (data.type == 0) {
			WBone* bone = anim->GetBone(0, boneN[slot]);
			if (bone) {
				anim->BindToBone(item->obj, bone->GetIndex());
				if (item->objExtra) {
					bone = anim->GetBone(0, boneNExtra[slot]);
					if (bone) {
						anim->BindToBone(item->objExtra, bone->GetIndex());
					}
				}
			}
		} else {
			item->obj->SetAnimation(anim);
			if (item->objExtra)
				item->objExtra->SetAnimation(anim);
		}
	}
}

void Unit::Unequip(ITEMSLOT slot) {
	EquippedItem* item = equipment.itemSlots[slot];
	if (anim && item && item->HasModel()) {
		if (item->data.type == 0) {
			WBone* bone = anim->GetBone(0, boneN[slot]);
			if (bone) {
				anim->UnbindFromBone(item->obj, bone->GetIndex());
				if (item->objExtra) {
					bone = anim->GetBone(0, boneNExtra[slot]);
					if (bone) {
						anim->UnbindFromBone(item->objExtra, bone->GetIndex());
					}
				}
			}
		} else {
			item->obj->SetAnimation(nullptr);
			if (item->objExtra)
				item->objExtra->SetAnimation(nullptr);
		}
	}

	if (equipment.itemSlots[slot]) {
		W_SAFE_DELETE(equipment.itemSlots[slot]);
	}
}

EquippedItem* Unit::GetEquippedItem(ITEMSLOT slot) {
	return equipment.itemSlots[slot];
}

void Unit::ApplyBuff(class Buff* buff) {
	//make sure we do this only in apply and remove because we don't want to increase the "current"
	//ability every frame, that would make the unit invincible
	STATS_STRUCT s = buff->GetStats();
	if (s.maxHealth > 0.001f) {
		baseStats.maxHealth += s.maxHealth;
		baseStats.health += s.maxHealth;
	}
	if (s.maxMana > 0.001f) {
		baseStats.maxMana += s.maxMana;
		baseStats.mana += s.maxMana;
	}
	if (s.maxEnergy > 0.001f) {
		baseStats.maxEnergy += s.maxEnergy;
		baseStats.energy += s.maxEnergy;
	}

	ApplyPerFrameStats(s);
}

void Unit::RemoveBuff(class Buff* buff) {
	//make sure we do this only in apply and remove because we don't want to increase the "current"
	//ability every frame, that would make the unit invincible
	STATS_STRUCT s = buff->GetStats();
	if (s.maxHealth > 0.001f) {
		baseStats.maxHealth -= s.maxHealth;
		baseStats.health -= s.maxHealth;
	}
	if (s.maxMana > 0.001f) {
		baseStats.maxMana -= s.maxMana;
		baseStats.mana -= s.maxMana;
	}
	if (s.maxEnergy > 0.001f) {
		baseStats.maxEnergy -= s.maxEnergy;
		baseStats.energy -= s.maxEnergy;
	}
	baseStats.health = max(baseStats.health, 1);
	baseStats.mana = max(baseStats.mana, 1);
	baseStats.energy = max(baseStats.energy, 1);
}

void Unit::ApplyPerFrameStats(STATS_STRUCT stats) {
	for (UINT offset = 6 * sizeof(float); offset < sizeof STATS_STRUCT; offset += 4) {
		float fCurVal, fAppliedVal, fFinalVal;
		memcpy(&fCurVal, (char*)(&curStats) + offset, sizeof(float));
		memcpy(&fAppliedVal, (char*)(&stats) + offset, sizeof(float));
		fFinalVal = fCurVal + fAppliedVal;
		memcpy((char*)(&curStats) + offset, &fFinalVal, sizeof(float));
	}
}

void Unit::Scale(float fPerc) {
	if (obj) {
		obj->Scale(fPerc, fPerc, fPerc);
		if (anim)
			anim->SetBindingScale(fPerc);
		for (UINT i = 0; i < NUM_ITEM_SLOTS; i++) {
			if (equipment.itemSlots[i]) {
				if (equipment.itemSlots[i]->obj && equipment.itemSlots[i]->data.type == 1) {
					equipment.itemSlots[i]->obj->Scale(fPerc, fPerc, fPerc);
					if (equipment.itemSlots[i]->objExtra)
						equipment.itemSlots[i]->objExtra->Scale(fPerc, fPerc, fPerc);
				}
			}
		}
	}
}

void Unit::SetAI(UnitAI* a) {
	ai = a;
}

UnitAI* Unit::GetAI() const {
	return ai;
}

void Unit::EnableAI() {
	bAIEnabled = true;
}

void Unit::DisableAI() {
	bAIEnabled = false;
}

void Unit::DisableMotion() {
	StopMoving();
	ai->DisableMotion();
}

void Unit::EnableMotion() {
	ai->EnableMotion();
}

Creature::Creature(UINT ID) : Unit(ID, ID) {
}

Creature::~Creature() {
}
