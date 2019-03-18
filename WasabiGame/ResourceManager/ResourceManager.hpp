#pragma once

#include "../Common.hpp"

struct MODEL {
	std::string name;
	UINT geometries[4];
	UINT textures[4];
	UINT effect;
	UINT material;
	UINT rigidBody;
};
struct ALLOCATED_MODEL {
	WObject* obj;
	//WRigidBody* rb;
};
struct ALLOCATED_UNIT {
	UINT ID;
	WObject* obj;
	WSkeleton* skeleton;
};

class ResourceManager {

	//
	// Loader data
	//
	//static WFile* dMapFile;
	static FILE* modelsFile;
	static UINT curObjID;
	static UINT curUnitAnimID;

	//
	// Loading utilities
	//
	static WGeometry* _allocGeometry(UINT ID);
	static WImage* _allocImage(UINT ID);
	static WEffect* _allocEffect(UINT ID);
	static WMaterial* _allocMaterial(UINT ID);
	static WSkeleton* _allocSkeleton(UINT ID);

	static void _releaseGeometry(UINT ID);
	static void _releaseImage(UINT ID);
	static void _releaseEffect(UINT ID);
	static void _releaseMaterial(WMaterial* mat, UINT ID);
	static void _releaseSkeleton(WSkeleton* sk, UINT ID);

public:
	static void Init(Wasabi* app);
	static void Cleanup(void);

	static MODEL GetModelData(UINT modelID);

	//
	// Game asset allocation
	//
	static ALLOCATED_MODEL AllocModel(UINT modelID);
	static void ReleaseModel(WObject* model, UINT modelID);
	static ALLOCATED_UNIT AllocUnit(UINT modelID, UINT skeletonID);
	static void ReleaseUnit(ALLOCATED_UNIT unit, UINT modelID, UINT skeletonID);
};


