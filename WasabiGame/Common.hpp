#pragma once

#include <Wasabi.h>
#include "Main.hpp"
//#include "Entities//Object.hpp"  @TODO: CHANGE HERE
//#include "Resource Manager//Resource Manager.hpp" @TODO: CHANGE HERE

#define APPNAME "RPG"

#define FONT_DEFAULT 0
#define FONT_CALIBRI_16 1

extern class Application* APPHANDLE;
#define CURGAMETIME (APPHANDLE->fCurGameTime)

struct STATS_STRUCT {
	//passive stats
	float moveSpeed;

	//abilities
	float health;
	float maxHealth;
	float mana;
	float maxMana;
	float energy;
	float maxEnergy;

	//offensive stats
	float attackDmg;
	float magicDmg;
	float armorPen;
	float magicPen;

	//defensive stats
	float fireResist;
	float frostResist;
	float natureResist;
	float armor;
	float shield;

	STATS_STRUCT() { ZeroMemory(this, sizeof STATS_STRUCT); }
};

struct DAMAGE_STRUCT {
	float fBaseAttackDmg;
	float fBaseFireDmg;
	float fBaseFrostDmg;
	float fBaseNatureDmg;

	float attackDmgMul;
	float fireDmgMul;
	float frostDmgMul;
	float natureDmgMul;
};

struct ANIMATIONRANGE {
	char name[16];
	UINT params[3];
};
