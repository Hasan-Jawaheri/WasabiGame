#include "ResourceManager.hpp"
#include "../Entities/Unit.hpp"

//hxFile* ResourceManager::dMapFile = nullptr;
FILE* ResourceManager::modelsFile = nullptr;
UINT ResourceManager::curObjID = INT_MAX;
UINT ResourceManager::curUnitAnimID = INT_MAX;

//
// Initialization and clean
//
void ResourceManager::Init(Wasabi* app) {
	dMapFile = new WFile(app);
	dMapFile->Initialize("media/Data/assets", nullptr, true);

	fopen_s(&modelsFile, "media/Data/models", "rb");

	/*// default texture
	hxImage* defImg;
	(defImg = AllocImage ( "default", true ))->
		Load ( "Media\\Units\\checker.bmp" );
	// default mesh
	AllocMesh ( "default", true )->
		CreateCube ( 10.0f );
	// default animation
	AllocSkeleton ( "default", true )->
		LoadFromHXS ( "Media\\Units\\Character.HXS" );
	//default material
	AllocGBufferMaterial ( "default", true )->
		Texture ( defImg );*/
}

void ResourceManager::Cleanup(void) {
	if (dMapFile) {
		dMapFile->Close();
		W_SAFE_REMOVEREF(dMapFile);
	}
	if (modelsFile) {
		fclose(modelsFile);
		modelsFile = nullptr;
	}
}

WGeometry* ResourceManager::_allocGeometry(UINT ID) {
	WGeometry* geometry = APPHANDLE->GeometryManager->GetEntity(ID);
	if (!geometry) {
		// load from file
	} else
		geometry->AddReference();
	return geometry;
}

WImage* ResourceManager::_allocImage(UINT ID) {
	WImage* image = APPHANDLE->ImageManager->GetEntity(ID);
	if (!image) {
		// load from file
	} else
		image->AddReference();
	return image;
}

WEffect* ResourceManager::_allocEffect(UINT ID) {
	WEffect* effect = APPHANDLE->EffectManager->GetEntity(ID);
	if (!effect) {
		// load from file
	} else
		effect->AddReference();
	return effect;
}

WMaterial* ResourceManager::_allocMaterial(UINT ID) {
	WMaterial* material = APPHANDLE->MaterialManager->GetEntity(ID);
	if (!material) {
		// load from file
	} else
		material->AddReference();

	WMaterial* new_material = new WMaterial(APPHANDLE);
	new_material->CopyFrom(material);
	return new_material;
}

/*hxRigidBody* ResourceManager::_allocRigidBody(UINT ID) {
	Entry<hxRigidBody>* me = _rigidBodies_.lookupEntry(index);
	if (!me) {
		//no yet loaded, load it
		dMapFile->LoadRigidBodyByIndex(index);

		Entry<hxRigidBody> entry;
		entry.refCount = 1;
		entry.index = index;
		entry.resource = reinterpret_cast<hxRigidBody*> (dMapFile->GetLastLoaded());

		_rigidBodies_.addEntry(entry);

		return entry.resource;
	} else
		me->refCount++;

	return me->resource;
}*/

WSkeleton* ResourceManager::_allocSkeleton(UINT ID) {
	WSkeleton* skeleton = (WSkeleton*)APPHANDLE->AnimationManager->GetEntity(ID);
	if (!skeleton) {
		// load from file
	} else
		skeleton->AddReference();

	WSkeleton* new_skeleton = new WSkeleton(APPHANDLE);
	new_skeleton->UseAnimationFrames(skeleton);

	if (new_skeleton->GetBone(0, "backbone")) {
		new_skeleton->AddSubAnimation();
		new_skeleton->SetSubAnimationBaseBone(1, new_skeleton->GetBone(0, "backbone")->GetIndex(), 0);
	}

	return new_skeleton;
}

void ResourceManager::_releaseGeometry(UINT ID) {
	WGeometry* geometry = APPHANDLE->GeometryManager->GetEntity(ID);
	W_SAFE_REMOVEREF(geometry);
}

void ResourceManager::_releaseImage(UINT ID) {
	WImage* image = APPHANDLE->ImageManager->GetEntity(ID);
	W_SAFE_REMOVEREF(image);
}

void ResourceManager::_releaseEffect(UINT ID) {
	WEffect* effect = APPHANDLE->EffectManager->GetEntity(ID);
	W_SAFE_REMOVEREF(effect);
}

void ResourceManager::_releaseMaterial(WMaterial* mat, UINT ID) {
	W_SAFE_REMOVEREF(mat);
	WMaterial* material = APPHANDLE->MaterialManager->GetEntity(ID);
	W_SAFE_REMOVEREF(material);
}

/*void ResourceManager::_releaseRigidBody(UINT ID) {
	Entry<hxRigidBody>* e = _rigidBodies_.lookupEntry(index);
	if (e)
		if (e->refCount > 0)
			e->refCount--;
}*/

void ResourceManager::_releaseSkeleton(WSkeleton* sk, UINT ID) {
	W_SAFE_REMOVEREF(sk);
	WSkeleton* skeleton = (WSkeleton*)APPHANDLE->AnimationManager->GetEntity(ID);
	W_SAFE_REMOVEREF(skeleton);
}

MODEL ResourceManager::GetModelData(UINT modelID) {
	fseek(modelsFile, 4 + modelID * sizeof MODEL, SEEK_SET);
	MODEL model;
	fread(&model, sizeof MODEL, 1, modelsFile);
	return model;
}

ALLOCATED_MODEL ResourceManager::AllocModel(UINT modelID) {
	MODEL model = GetModelData(modelID);
	WObject* obj = new WObject(APPHANDLE, curObjID++);
	WGeometry* geometry = nullptr;
	WImage* tex = nullptr;
	WEffect* fx = nullptr;
	WMaterial* mat = nullptr;
	//hxRigidBody* rb = nullptr;
	if (model.geometries[0] != -1)
		geometry = _allocGeometry(model.geometries[0]);
	if (model.textures[0] != -1)
		tex = _allocImage(model.textures[0]);
	if (model.effect != -1)
		fx = _allocEffect(model.effect);
	if (model.material != -1)
		mat = _allocMaterial(model.material);
	//if (model.rigidBody != -1)
	//	rb = _allocRigidBody(model.rigidBody);

	mat->SetTexture(0, tex);
	obj->SetGeometry(geometry);
	obj->SetMaterial(mat);
	//if (rb)
	//	rb->BindObject(obj);
	obj->SetName(model.name);

	ALLOCATED_MODEL ret;
	ret.obj = obj;
	//ret.rb = rb;
	return ret;
}

void ResourceManager::ReleaseModel(WObject* m, UINT modelID) {
	if (m) {
		MODEL model = GetModelData(modelID);
		if (model.material != 0 && model.material != -1)
			_releaseMaterial(m->GetMaterial(), model.material);
		else {
			WMaterial* mat = m->GetMaterial();
			W_SAFE_REMOVEREF(mat);
		}
		if (model.geometries[0] != 0 && model.geometries[0] != -1)
			_releaseGeometry(model.geometries[0]);
		if (model.textures[0] != 0 && model.textures[0] != -1)
			_releaseImage(model.textures[0]);
		if (model.effect != 0 && model.effect != -1)
			_releaseEffect(model.effect);
		//if (model.rigidBody != 0 && model.rigidBody != -1)
		//	_releaseRigidBody(model.rigidBody);

		W_SAFE_REMOVEREF(m);
	}
}

ALLOCATED_UNIT ResourceManager::AllocUnit(UINT modelID, UINT skeletonID) {
	ALLOCATED_UNIT u;
	ZeroMemory(&u, sizeof ALLOCATED_UNIT);

	u.obj = AllocModel(modelID).obj;
	u.ID = u.obj->GetID();

	u.skeleton = _allocSkeleton(skeletonID);
	u.obj->SetAnimation(u.skeleton);

	return u;
}

void ResourceManager::ReleaseUnit(ALLOCATED_UNIT unit, UINT modelID, UINT skeletonID) {
	_releaseSkeleton(unit.skeleton, skeletonID);
	ReleaseModel(unit.obj, modelID);
}

