#pragma once

#include "WasabiGame/Maps/Map.hpp"

#include <algorithm>


namespace RollTheBall {

	class TestMap : public WasabiGame::Map {
	public:
		virtual MAP_DESCRIPTION GetDescription() const {
			MAP_DESCRIPTION desc = {};
			desc.mapFilename = "test";
			return desc;
		}

		virtual void Update(float fTimeDelta) {
		}

		static bool Generate(Wasabi* app, WFile* mapFile) {
			std::srand(3);

			std::vector<WObject*> boxes;
			std::vector<WLight*> lights;

			// Create the plain
			WObject* plain = app->ObjectManager->CreateObject();
			WGeometry* plainGeometry = new WGeometry(app);
			plainGeometry->CreatePlain(50.0f, 0, 0);
			plain->SetGeometry(plainGeometry);
			plainGeometry->RemoveReference();
			plain->GetMaterials().SetVariable("color", WColor(0.4f, 0.4f, 0.4f));
			plain->GetMaterials().SetVariable("isTextured", 0);

			// Create the boxes
			WGeometry* boxGeometry = new WGeometry(app);
			boxGeometry->CreateCube(2.0f);
			for (int i = 0; i < 40; i++) {
				float x = 40.0f * (float)(rand() % 10000) / 10000.0f - 20.0f;
				float y = 3.0f * (float)(rand() % 10000) / 10000.0f - 0.0f;
				float z = 40.0f * (float)(rand() % 10000) / 10000.0f - 20.0f;
				WObject* box = app->ObjectManager->CreateObject();
				box->SetGeometry(boxGeometry);
				box->SetPosition(x, y, z);
				boxes.push_back(box);
				box->GetMaterials().SetVariable("color", WColor(0.7f, 0.7f, 0.7f));
				box->GetMaterials().SetVariable("isTextured", 0);
			}
			boxGeometry->RemoveReference();

			// hide default light
			app->LightManager->GetDefaultLight()->Hide();

			int maxLights = std::min(app->GetEngineParam<int>("maxLights", INT_MAX), 8);
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

			// save rigidbodies for server to load
			saveRigidBodyForObject(app, mapFile, plain);
			for (uint i = 0; i < boxes.size(); i++) {
				saveRigidBodyForObject(app, mapFile, boxes[i]);
			}

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

		static void saveRigidBodyForObject(Wasabi* app, WFile* mapFile, WObject* object) {
			WGeometry* geometry = object->GetGeometry();
			W_VERTEX_DESCRIPTION vertexDescription = geometry->GetVertexDescription();
			WVector3* vertexPositions = new WVector3[geometry->GetNumVertices()];
			char* vertices;
			uint32_t* indices;
			geometry->MapVertexBuffer((void**)&vertices, W_MAP_READ);
			geometry->MapIndexBuffer((void**)&indices, W_MAP_READ);

			// copy over the vertex positions only
			for (int i = 0; i < geometry->GetNumVertices(); i++) {
				memcpy(&vertexPositions[i], vertices + vertexDescription.GetSize() * i + vertexDescription.GetOffset(W_ATTRIBUTE_POSITION.name), sizeof(WVector3));
			}

			WRigidBody* rb = app->PhysicsComponent->CreateRigidBody();
			rb->Create(W_RIGID_BODY_CREATE_INFO::ForComplexRawGeometry(vertexPositions, geometry->GetNumVertices(), indices, geometry->GetNumIndices() / 3, true, object), true);
			rb->SetFriction(1.0f);
			rb->SetName(object->GetName() + "-rigidbody");
			mapFile->SaveAsset(rb);
			W_SAFE_REMOVEREF(rb);

			geometry->UnmapVertexBuffer(false);
			geometry->UnmapIndexBuffer();
			W_SAFE_DELETE_ARRAY(vertexPositions);
		}
	};

};
