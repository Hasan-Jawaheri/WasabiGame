#include "ResourceManager.hpp"

ResourceManager::MAP_RESOURCES ResourceManager::m_mapResources = {};
ResourceManager::GENERAL_RESOURCES ResourceManager::m_generalResources = {};

void ResourceManager::MAP_RESOURCES::Cleanup() {
	for (auto asset : loadedAssets) {
		W_SAFE_REMOVEREF(asset.obj);
		W_SAFE_REMOVEREF(asset.rb);
	}
	loadedAssets.clear();

	if (mapFile) {
		mapFile->Close();
		W_SAFE_DELETE(mapFile);
	}
}

void ResourceManager::GENERAL_RESOURCES::Cleanup() {
	for (auto asset : loadedAssets) {
		W_SAFE_REMOVEREF(asset.first->obj);
		W_SAFE_REMOVEREF(asset.first->rb);
	}
	loadedAssets.clear();

	if (assetsFile) {
		assetsFile->Close();
		W_SAFE_DELETE(assetsFile);
	}
}

WError ResourceManager::Init() {
	m_generalResources.Cleanup();
	m_generalResources.assetsFile = new WFile(APPHANDLE);
	WError err = m_generalResources.assetsFile->Open("Media/resources.WSBI");
	if (!err) {
		APPHANDLE->WindowAndInputComponent->ShowErrorMessage("Failed to load resources: " + err.AsString());
		Cleanup();
	}

	return err;
}

void ResourceManager::Cleanup() {
	m_mapResources.Cleanup();
	m_generalResources.Cleanup();
}

void ResourceManager::LoadMapFile(std::string mapFilename) {
	m_mapResources.Cleanup();

	if (mapFilename != "") {
		std::string fullMapFilename = "Media/Maps/" + mapFilename + ".WSBI";
		m_mapResources.mapFile = new WFile(APPHANDLE);
		WError err = m_mapResources.mapFile->Open(fullMapFilename);
		if (!err) {
			APPHANDLE->WindowAndInputComponent->ShowErrorMessage("Failed to load map: " + err.AsString());
			m_mapResources.Cleanup();
			return;
		}

		uint numAssets = m_mapResources.mapFile->GetAssetsCount();
		for (uint i = 0; i < numAssets; i++) {
			std::pair<std::string, std::string> info = m_mapResources.mapFile->GetAssetInfo(i);
			std::string& name = info.first;
			std::string& type = info.second;
			if (type == WObject::_GetTypeName()) {
				LOADED_MODEL asset;
				WError err = m_mapResources.mapFile->LoadAsset<WObject>(name, &asset.obj, WObject::LoadArgs());
				asset.rb = APPHANDLE->PhysicsComponent->CreateRigidBody();
				err = asset.rb->Create(W_RIGID_BODY_CREATE_INFO::ForComplexObject(asset.obj));
				asset.rb->BindObject(asset.obj, asset.obj);
				m_mapResources.loadedAssets.push_back(asset);
			}
		}
	}
}

LOADED_MODEL* ResourceManager::LoadUnitModel(std::string unitName) {
	LOADED_MODEL* asset = new LOADED_MODEL();
	WError err = m_generalResources.assetsFile->LoadAsset<WObject>(unitName, &asset->obj, WObject::LoadArgs());
	if (!err) {
		delete asset;
		return nullptr;
	}

	asset->rb = APPHANDLE->PhysicsComponent->CreateRigidBody();
	err = asset->rb->Create(W_RIGID_BODY_CREATE_INFO::ForSphere(5.0f, 1.0f));
	asset->rb->BindObject(asset->obj, asset->obj);
	m_generalResources.loadedAssets.insert(std::make_pair(asset, unitName));

	WSkeleton* skeleton = nullptr;
	err = m_generalResources.assetsFile->LoadAsset<WSkeleton>(unitName + "-skeleton", &skeleton, WSkeleton::LoadArgs());
	if (err == W_SUCCEEDED) {
		asset->obj->SetAnimation(skeleton);
		skeleton->Play();
		skeleton->SetPlaySpeed(10);
		skeleton->RemoveReference();
	}

	return asset;
}

void ResourceManager::DestroyUnitModel(LOADED_MODEL* model) {
	auto it = m_generalResources.loadedAssets.find(model);
	W_SAFE_REMOVEREF(it->first->obj);
	W_SAFE_REMOVEREF(it->first->rb);
	m_generalResources.loadedAssets.erase(it);
}
