#include "RTBClient/Main.hpp"
#include "RTBClient/GameStates/MainGameState.hpp"
#include "RTBClient/Networking/Networking.hpp"

#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/AI/PlayerAI.hpp"
#include "RollTheBall/GameModes/GameModes.hpp"

#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"

#include "WasabiGame/UI/InputControls/MenuButton.hpp"
#include "WasabiGame/UI/GeneralControls/ErrorBox.hpp"


RTBClient::MainGameState::MainGameState(Wasabi* app) : WasabiGame::BaseGameState(app) {
	m_input = nullptr;
	m_player = nullptr;
	m_currentGameMode = RollTheBall::RTB_GAME_MODE::GAME_MODE_NONE;
}

RTBClient::MainGameState ::~MainGameState() {
}

void RTBClient::MainGameState::Load() {
	// Setup and load the user interface
	((RTBClient::ClientApplication*)m_app)->UI->AddUIElement(std::dynamic_pointer_cast<WasabiGame::UIElement>(m_input = std::make_shared<GameInputHandler>(this)), nullptr);
	std::shared_ptr<WasabiGame::MenuButton> ok = std::make_shared<WasabiGame::MenuButton>(((RTBClient::ClientApplication*)m_app)->UI, "sure");
	std::shared_ptr<WasabiGame::ErrorBox> err = std::make_shared<WasabiGame::ErrorBox>(((RTBClient::ClientApplication*)m_app)->UI, "what??");
	((RTBClient::ClientApplication*)m_app)->UI->AddUIElement(err, std::dynamic_pointer_cast<WasabiGame::UIElement>(m_input));
	((RTBClient::ClientApplication*)m_app)->UI->AddUIElement(ok, err);
	((RTBClient::ClientApplication*)m_app)->UI->Load();

	m_input->DisableInput(); // disable input (loading)

	// Load the map
	((RTBClient::ClientApplication*)m_app)->Maps->SetMap(RollTheBall::MAP_TEST);

	((RTBClient::ClientApplication*)m_app)->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOAD_UNIT, [this](std::shared_ptr<WasabiGame::Selectable> s, WasabiGame::NetworkUpdate& update) {
		uint32_t unitId, unitType;
		WVector3 spawnPos;
		RollTheBall::UpdateBuilders::ReadLoadUnitPacket(update, &unitType, &unitId, &spawnPos);
		std::shared_ptr<WasabiGame::Unit> unit = ((RTBClient::ClientApplication*)this->m_app)->Units->LoadUnit(unitType, unitId, spawnPos);
		if (unitType == RollTheBall::UNIT_PLAYER && !this->m_player) {
			m_input->EnableInput(); // enable input now that player is loaded
			this->m_player = std::dynamic_pointer_cast<RollTheBall::Player>(unit);
		}
		return true;
	});

	((RTBClient::ClientApplication*)m_app)->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_UNLOAD_UNIT, [this](std::shared_ptr<WasabiGame::Selectable> s, WasabiGame::NetworkUpdate& update) {
		uint32_t unitId;
		RollTheBall::UpdateBuilders::ReadUnloadUnitPacket(update, &unitId);
		if (m_player && unitId == m_player->GetId())
			this->m_player = nullptr;
		((RTBClient::ClientApplication*)this->m_app)->Units->DestroyUnit(unitId);
		return true;
	});

	((RTBClient::ClientApplication*)m_app)->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS, [this](std::shared_ptr<WasabiGame::Selectable> s, WasabiGame::NetworkUpdate& update) {
		uint32_t unitId = -1;
		std::shared_ptr<WasabiGame::Unit> unit = nullptr;
		RollTheBall::UpdateBuilders::ReadSetUnitPropsPacket(update, &unitId, [this, &unitId, &unit](std::string prop, void* data, uint16_t size) {
			if (unitId == -1)
				return;
			if (!unit) {
				unit = ((RTBClient::ClientApplication*)this->m_app)->Units->GetUnit(unitId);
				if (!unit) {
					WasabiGame::NetworkUpdate whoisUpdate;
					RollTheBall::UpdateBuilders::WhoIsUnit(whoisUpdate, unitId);
					((RTBClient::ClientApplication*)this->m_app)->Networking->SendUpdate(whoisUpdate);
					unitId = -1;
					return;
				}
			}

			std::dynamic_pointer_cast<RollTheBall::RTBAI>(unit->GetAI())->OnNetworkUpdate(prop, data, size);
		});
		return true;
	});


	// Login to server
	std::static_pointer_cast<ClientNetworking>(((RTBClient::ClientApplication*)m_app)->Networking)->Login();
}

void RTBClient::MainGameState::Update(float fDeltaTime) {
	if (std::dynamic_pointer_cast<ClientNetworking>(((RTBClient::ClientApplication*)m_app)->Networking)->Status == RTBConnectionStatus::CONNECTION_CONNECTED && m_currentGameMode == RollTheBall::RTB_GAME_MODE::GAME_MODE_NONE) {
		m_currentGameMode = RollTheBall::RTB_GAME_MODE::GAME_MODE_ONE_VS_ONE;
		WasabiGame::NetworkUpdate gameModeUpdate;
		RollTheBall::UpdateBuilders::SelectGameMode(gameModeUpdate, RollTheBall::RTB_GAME_MODE::GAME_MODE_ONE_VS_ONE);
		((RTBClient::ClientApplication*)this->m_app)->Networking->SendUpdate(gameModeUpdate);
	}
}

void RTBClient::MainGameState::Cleanup() {
	((RTBClient::ClientApplication*)m_app)->Maps->SetMap(RollTheBall::MAP_NONE);
}

std::shared_ptr<RollTheBall::Player> RTBClient::MainGameState::GetPlayer() const {
	return m_player;
}

RTBClient::GameInputHandler::GameInputHandler(RTBClient::MainGameState* gameState) : WasabiGame::UIElement(((RTBClient::ClientApplication*)gameState->m_app)->UI) {
	m_game = gameState;
	m_draggingCamera = false;
	m_lastMouseX = m_lastMouseY = 0.0;
	m_inputEnabled = true;
}

RTBClient::GameInputHandler::~GameInputHandler() {
}

void RTBClient::GameInputHandler::EnableInput() {
	m_inputEnabled = true;
}

void RTBClient::GameInputHandler::DisableInput() {
	m_inputEnabled = false;
}

bool RTBClient::GameInputHandler::Update(float fDeltaTime) {
	Wasabi* app = m_game->m_app;

	std::shared_ptr<RollTheBall::PlayerAI> ai = std::dynamic_pointer_cast<RollTheBall::PlayerAI>(m_game->GetPlayer()->GetAI());
	float cameraDistance = ai->GetCameraDistance();
	float fMouseZ = (float)app->WindowAndInputComponent->MouseZ();
	cameraDistance += fMouseZ * (abs(cameraDistance) / 10.0f);
	app->WindowAndInputComponent->SetMouseZ(0);
	cameraDistance = fmin(-1.0f, cameraDistance);
	ai->SetCameraDistance(cameraDistance);

	return true;
}

bool RTBClient::GameInputHandler::OnEnter() {
	if (m_inputEnabled) {
	}
	return true;
}

void RTBClient::GameInputHandler::OnKeydown(uint32_t key) {
	if (m_inputEnabled) {
		RTBClient::ClientApplication* app = ((RTBClient::ClientApplication*)m_game->m_app);
		switch (key) {
		case W_KEY_W:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveForward(true);
			break;
		case W_KEY_S:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveBackward(true);
			break;
		case W_KEY_A:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveLeft(true);
			break;
		case W_KEY_D:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveRight(true);
			break;
		case W_KEY_SPACE:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveJump(true);
			break;
		}
	}
}

void RTBClient::GameInputHandler::OnKeyup(uint32_t key) {
	if (m_inputEnabled) {
		RTBClient::ClientApplication* app = ((RTBClient::ClientApplication*)m_game->m_app);
		switch (key) {
		case W_KEY_W:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveForward(false);
			break;
		case W_KEY_S:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveBackward(false);
			break;
		case W_KEY_A:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveLeft(false);
			break;
		case W_KEY_D:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveRight(false);
			break;
		case W_KEY_SPACE:
			std::dynamic_pointer_cast<RollTheBall::RTBAI>(m_game->m_player->GetAI())->SetMoveJump(false);
			break;
		}
	}
}

void RTBClient::GameInputHandler::OnMouseMove(double mx, double my) {
	if (m_inputEnabled) {
		if (m_draggingCamera) {
			RTBClient::ClientApplication* app = ((RTBClient::ClientApplication*)m_game->m_app);
			std::shared_ptr<RollTheBall::Player> player = m_game->GetPlayer();
			std::shared_ptr<RollTheBall::PlayerAI> ai = std::dynamic_pointer_cast<RollTheBall::PlayerAI>(player->GetAI());
			float yawAngle = ai->GetYawAngle();
			float cameraPitch = ai->GetCameraPitch();

			int dx = mx - m_lastMouseX;
			int dy = my - m_lastMouseY;
			m_lastMouseX = mx;
			m_lastMouseY = my;

			yawAngle += (float)dx / 2.0f;
			cameraPitch += (float)dy / 2.0f;

			ai->SetYawAngle(yawAngle);
			ai->SetCameraPitch(cameraPitch);
		}
	}
}

void RTBClient::GameInputHandler::OnMouseButton(double mx, double my, bool bDown) {
	if (m_inputEnabled) {
	}
}

void RTBClient::GameInputHandler::OnMouseButton2(double mx, double my, bool bDown) {
	if (m_inputEnabled) {
		Wasabi* app = m_game->m_app;
		m_draggingCamera = bDown;
		app->WindowAndInputComponent->SetCursorMotionMode(m_draggingCamera);
		m_lastMouseX = mx;
		m_lastMouseY = my;
	}
}

bool RTBClient::GameInputHandler::OnEscape() {
	if (m_inputEnabled) {
		m_game->m_app->SwitchState(nullptr);
	}
	return false;
}
