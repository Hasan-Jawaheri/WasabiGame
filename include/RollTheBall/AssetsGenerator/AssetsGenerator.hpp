#pragma once

#include <Wasabi/Wasabi.h>
#include <Wasabi/Physics/Bullet/WBulletPhysics.h>

#include <filesystem>
typedef std::experimental::filesystem::path stdpath;

#include "RollTheBall/Maps/Test.hpp"

class WasabiGenerator : public Wasabi {
public:
	WasabiGenerator() : Wasabi() {}

	virtual WError Setup() {
		SetEngineParam<const char*>("appName", "AssetGenerator");
		SetEngineParam<int>("enableVulkanValidation", 0);

		WError err = StartEngine(1, 1);
		if (!err) {
			WindowAndInputComponent->ShowErrorMessage("Failed to start the asset generator: " + err.AsString());
			return err;
		}

		return W_SUCCEEDED;
	}

	virtual bool Loop(float fDeltaTime) {
		return false;
	}

	virtual void Cleanup() {}

	WPhysicsComponent* CreatePhysicsComponent() {
		WBulletPhysics* physics = new WBulletPhysics(this);
		WError werr = physics->Initialize();
		if (!werr)
			W_SAFE_DELETE(physics);
		return physics;
	}
};

class AssetGenerator {
	Wasabi* m_app;
	WFile* m_resourcesFile;
	std::string m_outputFolder;

public:
	AssetGenerator(std::string outputFolder = "Media/") {
		m_app = new WasabiGenerator();
		m_resourcesFile = nullptr;
		m_outputFolder = outputFolder;
	}

	~AssetGenerator() {
		if (m_resourcesFile) {
			m_resourcesFile->Close();
			delete m_resourcesFile;
		}
		W_SAFE_DELETE(m_app);
	}

	bool CreateAndOpenFile(WFile* file, std::string filename) {
		// empty out the file
		std::fstream f;
		f.open(filename, ios::out);
		f.close();

		// open it
		return file->Open(filename);
	}

	bool GeneratePlayer() {
		WGeometry* ball = new WGeometry(m_app);
		ball->SetName("player-geometry");
		ball->CreateSphere(1.0f, 16, 16);
		WObject* ballObj = m_app->ObjectManager->CreateObject();
		ballObj->SetName("player");
		ballObj->SetGeometry(ball);
		ballObj->GetMaterial()->SetVariableColor("color", WColor(0.8f, 0.6f, 0.55f, 1.0f));
		WRigidBody* rb = m_app->PhysicsComponent->CreateRigidBody();
		W_RIGID_BODY_CREATE_INFO rbCreateInfo = W_RIGID_BODY_CREATE_INFO::ForSphere(1.0f, 20.0f);
		rb->Create(rbCreateInfo, true);
		rb->SetName("player-rigidbody");

		rb->SetLinearDamping(0.8f);
		rb->SetAngularDamping(0.6f);
		rb->SetFriction(1.0f);
		rb->SetBouncingPower(0.2f);

		if (!m_resourcesFile->SaveAsset(ballObj))
			return false;

		if (!m_resourcesFile->SaveAsset(rb))
			return false;

		return true;
	}

	bool GenerateUnits() {
		WGeometry* ball = new WGeometry(m_app);
		ball->SetName("small-ball-geometry");
		ball->CreateSphere(0.7f, 16, 16);
		WObject* ballObj = m_app->ObjectManager->CreateObject();
		ballObj->SetName("small-ball");
		ballObj->SetGeometry(ball);
		ballObj->GetMaterial()->SetVariableColor("color", WColor(0.8f, 0.6f, 0.55f, 1.0f));
		WRigidBody* rb = m_app->PhysicsComponent->CreateRigidBody();
		W_RIGID_BODY_CREATE_INFO rbCreateInfo = W_RIGID_BODY_CREATE_INFO::ForSphere(0.7f, 20.0f);
		rb->Create(rbCreateInfo, true);
		rb->SetName("small-ball-rigidbody");

		rb->SetLinearDamping(0.8f);
		rb->SetAngularDamping(0.6f);
		rb->SetFriction(1.0f);
		rb->SetBouncingPower(0.2f);

		if (!m_resourcesFile->SaveAsset(ballObj))
			return false;

		if (!m_resourcesFile->SaveAsset(rb))
			return false;

		return true;
	}

	bool GenerateMap(std::string mapName, std::function<bool(Wasabi*, WFile*)> generator) {
		WFile mapFile(m_app);
		if (!CreateAndOpenFile(&mapFile, (stdpath(m_outputFolder) / stdpath("Maps") / stdpath(mapName + ".WSBI")).string()))
			return false;

		bool b = generator(m_app, &mapFile);
		mapFile.Close();
		return b;
	}

	bool Generate() {
		if (!m_app->Setup())
			return false;

		m_resourcesFile = new WFile(m_app);

		return
			CreateAndOpenFile(m_resourcesFile, (stdpath(m_outputFolder) / stdpath("resources.WSBI")).string()) &&
			GeneratePlayer() &&
			GenerateUnits() &&
			GenerateMap("test", TestMap::Generate);
	}
};
