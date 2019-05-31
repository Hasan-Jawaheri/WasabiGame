#pragma once

#include <Wasabi.h>

bool GenerateTestMap(Wasabi* app, WFile* mapFile) {
	srand(3);

	std::vector<WObject*> boxes;
	std::vector<WLight*> lights;

	// Create the plain
	WObject* plain = app->ObjectManager->CreateObject();
	WGeometry* plainGeometry = new WGeometry(app);
	plainGeometry->CreatePlain(50.0f, 0, 0);
	plain->SetGeometry(plainGeometry);
	plainGeometry->RemoveReference();
	plain->GetMaterial()->SetVariableColor("color", WColor(0.4f, 0.4f, 0.4f));
	plain->GetMaterial()->SetVariableInt("isTextured", 0);

	// Create the boxes
	WGeometry* boxGeometry = new WGeometry(app);
	boxGeometry->CreateCube(2.0f);
	for (int i = 0; i < 40; i++) {
		float x = 20.0f * (float)(rand() % 10000) / 10000.0f - 10.0f;
		float y = 2.0f * (float)(rand() % 10000) / 10000.0f - 0.0f;
		float z = 20.0f * (float)(rand() % 10000) / 10000.0f - 10.0f;
		WObject* box = app->ObjectManager->CreateObject();
		box->SetGeometry(boxGeometry);
		box->SetPosition(x, y, z);
		boxes.push_back(box);
		box->GetMaterial()->SetVariableColor("color", WColor(0.7f, 0.7f, 0.7f));
		box->GetMaterial()->SetVariableInt("isTextured", 0);
	}
	boxGeometry->RemoveReference();

	// hide default light
	app->LightManager->GetDefaultLight()->Hide();

	int maxLights = min(app->engineParams.find("maxLights") != app->engineParams.end() ? (int)app->engineParams["maxLights"] : INT_MAX, 8);
	WColor colors[] = {
		WColor(1, 0, 0),
		WColor(0, 1, 0),
		WColor(0, 0, 1),
		WColor(1, 1, 0),
		WColor(0, 1, 1),
		WColor(1, 0, 1),
		WColor(1, 1, 1),
	};

	for (int i = 0; i < maxLights / 2; i++) {
		float x = 20.0f * (float)(rand() % 10000) / 10000.0f - 10.0f;
		float z = 20.0f * (float)(rand() % 10000) / 10000.0f - 10.0f;

		WLight* l = new WPointLight(app);
		l->SetRange(5.0f);
		l->SetPosition(x, 3.0, z);
		l->SetColor(colors[rand() % (sizeof(colors) / sizeof(WColor))]);
		lights.push_back(l);
	}

	for (int i = 0; i < maxLights / 2; i++) {
		float x = 20.0f * (float)(rand() % 10000) / 10000.0f + 10.0f;
		float z = (10.0f + 15.0f * (float)(rand() % 10000) / 10000.0f) * (rand() % 2 == 0 ? 1 : -1);

		WLight* l = new WSpotLight(app);
		l->SetIntensity(5.0f);
		l->SetRange(30.0f);
		l->SetPosition(x, 4.0, z);
		l->Point(WVector3(x, 0.0, z) * 0.5f);
		l->SetColor(colors[rand() % (sizeof(colors) / sizeof(WColor))]);
		lights.push_back(l);
	}

	plain->SetName("test-plain");
	mapFile->SaveAsset(plain);
	W_SAFE_REMOVEREF(plain);

	for (uint i = 0; i < boxes.size(); i++) {
		boxes[i]->SetName("test-box-" + std::to_string(i));
		mapFile->SaveAsset(boxes[i]);
		W_SAFE_REMOVEREF(boxes[i]);
	}

	for (uint i = 0; i < lights.size(); i++) {
		lights[i]->SetName("test-light-" + std::to_string(i));
		mapFile->SaveAsset(lights[i]);
		W_SAFE_REMOVEREF(lights[i]);
	}

	return true;
}
