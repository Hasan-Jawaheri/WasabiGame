#pragma once

#include "../Common.hpp"
#include "Object.hpp"
#include "../Spells/Buff.hpp"
#include "../Spells/ComboSystem.hpp"
#include "../AI/AI.hpp"

#define SPEED 30.0f
#define SMSPEED 6.0f

class Unit;

struct EQUIPMENTDATA {
	char name[64];
	UINT modelID[2];
	UINT damage[2];
	UINT armor;
	char type, slot;
};

class UnitManager {
	friend class Unit;
	friend class EquippedItem;
	static vector<Unit*> units;
	static class Player* player;

	struct UNITSFILEHEADER {
		UINT numUnits;
		vector<UINT> unitPos;
	};
#define UNITDATASIZENOVECTOR 148
	struct UNITDATA {
		char name[64];
		char AIName[64];
		UINT modelID;
		UINT skeletonID;
		UINT health;
		UINT mana;
		float scale;
		vector<ANIMATIONRANGE> animationData;
	};
	static FILE* unitsFile;
	static UNITSFILEHEADER unitsFileHeader;

	static FILE* equipmentFile;

public:

	static void Initialize();
	static void Update(float fDeltaTime);
	static void Cleanup();
	static Unit* LoadUnit(UINT unitID, Unit* premade = nullptr);

	static bool UnitExists(Unit* unit);

	static void DealDamage(Unit* from, Unit* to, DAMAGE_STRUCT dmg);
	static void DealDamage(Unit* from, float fRange, DAMAGE_STRUCT dmg);
	static void DealDamage(Unit* from, WVector3 pos, float fRange, DAMAGE_STRUCT dmg);

	static Unit* FindTarget(WVector2 point, float maxDist = FLT_MAX);

	static class Player* GetPlayer();
	static void SetPlayer(class Player* p);
};

class EquippedItem : public Object {
	friend class Unit;
	Unit* owner;
	EQUIPMENTDATA data;
	WObject* objExtra; //some items require a pair of models

public:
	EquippedItem(Unit* o, EQUIPMENTDATA data);
	~EquippedItem();

	virtual void Load();
	virtual void Destroy();
	virtual bool Update(float fDeltaTime);

	bool HasModel() const;
};

class Unit : public Object {
	friend class UnitManager;
	friend class ResourceManager;

	struct EQUIPMENT_STRUCT {
		EquippedItem* itemSlots[NUM_ITEM_SLOTS];
	} equipment;

	struct INVENTORY_STRUCT {
	} inventory;

protected:
	UINT modelID;
	UINT skeletonID;

	WSkeleton* anim;

	UnitAI* ai;
	bool bAIEnabled;

	vector<ANIMATIONRANGE> animationRanges;
	enum ANIMSTATE { IDLE, STARTED_MOVING, MOVING, CASTING } animStateL, animStateH;
	enum ANIMPART { UPPER_BODY = 1, LOWER_BODY = 2 };
	bool bDead;
	ANIMATIONRANGE SetAnimationState(ANIMSTATE st, UINT i1 = 0, UINT i2 = 1, float fSpeed = SPEED,
		ANIMPART body = (ANIMPART)(UPPER_BODY | LOWER_BODY), LPCSTR name = "hit1", bool bForceLoop = false);

	class ComboSystem* comboSys;

public:
	STATS_STRUCT baseStats, curStats;

	Unit(UINT modelID, UINT skeletonID);
	virtual ~Unit();

	virtual void Load();
	virtual void Destroy();
	virtual bool Update(float fDeltaTime);

	//Inherit to bind items to bones. Call the parent function always.
	virtual void Equip(ITEMSLOT slot, UINT itemID);
	virtual void Unequip(ITEMSLOT slot);
	EquippedItem* GetEquippedItem(ITEMSLOT slot);

	void ApplyBuff(class Buff* buff);
	void RemoveBuff(class Buff* buff);
	void ApplyPerFrameStats(STATS_STRUCT stats);

	//combos
	class ComboSystem* GetComboSystem() const;

	//animation
	virtual void AddAnimationRange(ANIMATIONRANGE range);
	virtual void ClearAnimationRange();
	virtual void StartMoving();
	virtual void StopMoving();
	virtual void SetAngle(float fAng);
	virtual void OnDeath();
	virtual void InitAnimation();
	virtual void UpdateAnimation();
	virtual bool StartSpellAnimation(LPCSTR animName, float fSpeed,
		bool bForceLowerBody = false, ANIMATIONRANGE* range = nullptr,
		UINT l1 = 0, UINT l2 = 1, bool bLoop = false);
	virtual void StopSpellAnimation();
	WSkeleton* GetAnimation() const;

	void Scale(float fPerc);

	virtual void SetAI(UnitAI* a);
	virtual UnitAI* GetAI() const;
	void EnableAI();
	void DisableAI();
	void DisableMotion();
	void EnableMotion();
};

class Creature : public Unit {
protected:

public:
	Creature(UINT ID = 0);
	virtual ~Creature();
};

