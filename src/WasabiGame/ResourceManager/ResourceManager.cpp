#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include <Wasabi/Physics/Bullet/WBulletRigidBody.hpp>

#include <filesystem>
typedef std::filesystem::path stdpath;

ResourceManager::ResourceManager(Wasabi* app) {
	m_app = app;
	m_mediaFolder = "";
	m_mapResources = {};
	m_generalResources = {};
}

void ResourceManager::MAP_RESOURCES::Cleanup() {
	for (auto asset : loadedAssets) {
		W_SAFE_REMOVEREF(asset.obj);
		W_SAFE_REMOVEREF(asset.rb);
	}
	for (auto light : loadedLights)
		W_SAFE_REMOVEREF(light);
	loadedAssets.clear();
	loadedLights.clear();

	if (mapFile) {
		mapFile->Close();
		W_SAFE_DELETE(mapFile);
	}
}

void ResourceManager::GENERAL_RESOURCES::Cleanup() {
	for (auto asset : loadedAssets) {
		W_SAFE_REMOVEREF(asset.second->obj);
		W_SAFE_REMOVEREF(asset.second->rb);
		W_SAFE_REMOVEREF(asset.second->skeleton);
	}
	loadedAssets.clear();

	if (assetsFile) {
		assetsFile->Close();
		W_SAFE_DELETE(assetsFile);
	}
}

WError ResourceManager::Init(std::string mediaFolder) {
	m_mediaFolder = mediaFolder;

	m_generalResources.Cleanup();
	m_generalResources.assetsFile = new WFile(m_app);
	WError err = m_generalResources.assetsFile->Open((stdpath(m_mediaFolder) / "resources.WSBI").string());
	if (!err) {
		m_app->WindowAndInputComponent->ShowErrorMessage("Failed to load resources: " + err.AsString());
		Cleanup();
	}

	return err;
}

void ResourceManager::Update(float fDeltaTime) {
	{
		std::lock_guard lockGuard(m_modelsToFreeMutex);
		for (auto it = m_modelsToFree.begin(); it != m_modelsToFree.end(); it++) {
			W_SAFE_REMOVEREF((*it)->obj);
			W_SAFE_REMOVEREF((*it)->rb);
		}
		m_modelsToFree.clear();
	}
}

void ResourceManager::Cleanup() {
	m_mapResources.Cleanup();
	m_generalResources.Cleanup();
}

void ResourceManager::LoadMapFile(std::string mapFilename) {
	m_mapResources.Cleanup();

	if (mapFilename != "") {
		std::string fullMapFilename = (stdpath(m_mediaFolder) / "Maps" / (mapFilename + ".WSBI")).string();
		m_mapResources.mapFile = new WFile(m_app);
		WError err = m_mapResources.mapFile->Open(fullMapFilename);
		if (!err) {
			m_app->WindowAndInputComponent->ShowErrorMessage("Failed to load map: " + err.AsString());
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
				asset.rb = m_app->PhysicsComponent->CreateRigidBody();
				err = asset.rb->Create(W_RIGID_BODY_CREATE_INFO::ForComplexObject(asset.obj));
				if (err) {
					asset.rb->SetFriction(1.0f);
					asset.rb->BindObject(asset.obj, asset.obj);
				}
				m_mapResources.loadedAssets.push_back(asset);
			} else if (type == WLight::_GetTypeName()) {
				WLight* light = nullptr;
				WError err = m_mapResources.mapFile->LoadAsset<WLight>(name, &light, WLight::LoadArgs());
				if (err)
					m_mapResources.loadedLights.push_back(light);
			}
		}
	}
}

LOADED_MODEL* ResourceManager::LoadUnitModel(std::string unitName) {
	LOADED_MODEL* asset = new LOADED_MODEL();
	std::string suffix = "-" + std::to_string((size_t)asset);
	asset->name = unitName + suffix;

	WError err = m_generalResources.assetsFile->LoadAsset<WObject>(unitName, &asset->obj, WObject::LoadArgs(), suffix);
	if (!err) {
		delete asset;
		return nullptr;
	}

	m_generalResources.loadedAssets.insert(std::make_pair(asset->name, asset));

	err = m_generalResources.assetsFile->LoadAsset<WBulletRigidBody>(unitName + "-rigidbody", (WBulletRigidBody**)&asset->rb, WBulletRigidBody::LoadArgs(), suffix);
	if (err == W_SUCCEEDED) {
		asset->rb->BindObject(asset->obj, asset->obj);
	}

	err = m_generalResources.assetsFile->LoadAsset<WSkeleton>(unitName + "-skeleton", &asset->skeleton, WSkeleton::LoadArgs(), suffix);
	if (err == W_SUCCEEDED) {
		asset->obj->SetAnimation(asset->skeleton);
		asset->skeleton->Play();
		asset->skeleton->SetPlaySpeed(10);
		asset->skeleton->RemoveReference();
	}

	return asset;
}

void ResourceManager::DestroyUnitModel(LOADED_MODEL* model) {
	auto it = m_generalResources.loadedAssets.find(model->name);
	m_generalResources.loadedAssets.erase(it);
	{
		std::lock_guard lockGuard(m_modelsToFreeMutex);
		m_modelsToFree.push_back(model);
	}
}
