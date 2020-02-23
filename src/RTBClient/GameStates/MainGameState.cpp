#include "RTBClient/Main.hpp"
#include "RTBClient/GameStates/MainGameState.hpp"

#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"
#include "RollTheBall/AI/PlayerAI.hpp"
#include "RollTheBall/AI/RTBAI.hpp"

#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"

#include "WasabiGame/UI/InputControls/MenuButton.hpp"
#include "WasabiGame/UI/GeneralControls/ErrorBox.hpp"


RTBClient::MainGameState::MainGameState(Wasabi* app) : WasabiGame::BaseGameState(app) {
	m_input = nullptr;
	m_player = nullptr;
}

RTBClient::MainGameState ::~MainGameState() {
}

void RTBClient::MainGameState::Load() {
	// Setup and load the user interface
	((RTBClient::ClientApplication*)m_app)->UI->AddUIElement(std::dynamic_pointer_cast<WasabiGame::UIElement>(m_input = std::make_shared<GameInputHandler>(this)), nullptr);
	std::shared_ptr<WasabiGame::UIElement> ok = std::make_shared<WasabiGame::MenuButton>(((RTBClient::ClientApplication*)m_app)->UI, "sure");
	std::shared_ptr<WasabiGame::UIElement> err = std::make_shared<WasabiGame::ErrorBox>(((RTBClient::ClientApplication*)m_app)->UI, "what??");
	((RTBClient::ClientApplication*)m_app)->UI->AddUIElement(err, std::dynamic_pointer_cast<WasabiGame::UIElement>(m_input));
	((RTBClient::ClientApplication*)m_app)->UI->AddUIElement(ok, err);
	((RTBClient::ClientApplication*)m_app)->UI->Load();

	// Load the map
	((RTBClient::ClientApplication*)m_app)->Maps->SetMap(RollTheBall::MAP_TEST);

	((RTBClient::ClientApplication*)m_app)->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOAD_UNIT, [this](WasabiGame::NetworkUpdate& update) {
		uint32_t unitId, unitType;
		WVector3 spawnPos;
		RollTheBall::UpdateBuilders::ReadLoadUnitPacket(update, &unitType, &unitId, &spawnPos);
		std::shared_ptr<WasabiGame::Unit> unit = ((RTBClient::ClientApplication*)this->m_app)->Units->LoadUnit(unitType, unitId, spawnPos);
		if (unitType == RollTheBall::UNIT_PLAYER && !this->m_player)
			this->m_player = std::dynamic_pointer_cast<RollTheBall::Player>(unit);
	});

	((RTBClient::ClientApplication*)m_app)->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_UNLOAD_UNIT, [this](WasabiGame::NetworkUpdate& update) {
		uint32_t unitId;
		RollTheBall::UpdateBuilders::ReadUnloadUnitPacket(update, &unitId);
		if (m_player && unitId == m_player->GetId())
			this->m_player = nullptr;
		((RTBClient::ClientApplication*)this->m_app)->Units->DestroyUnit(unitId);
	});

	((RTBClient::ClientApplication*)m_app)->Networking->RegisterNetworkUpdateCallback(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_UNIT_PROPS, [this](WasabiGame::NetworkUpdate& update) {
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
	});

	// Login to server
	((RTBClient::ClientApplication*)m_app)->Networking->Login();
}

void RTBClient::MainGameState::Update(float fDeltaTime) {
}

void RTBClient::MainGameState::Cleanup() {
	((RTBClient::ClientApplication*)m_app)->Maps->SetMap(RollTheBall::MAP_NONE);
}

RTBClient::GameInputHandler::GameInputHandler(RTBClient::MainGameState* gameState) : WasabiGame::UIElement(((RTBClient::ClientApplication*)gameState->m_app)->UI) {
	m_game = gameState;
}

bool RTBClient::GameInputHandler::OnEnter() {
	/*
	if (((RTBClient::ClientApplication*)m_app)->UI->GetFocus() != (UIElement*)m_game->m_ui.chatEdit) {
		m_game->ui.chatEdit->OnFocus();
		((RTBClient::ClientApplication*)m_app)->UI->SetFocus(m_game->m_ui.chatEdit);
	} else {
		((RTBClient::ClientApplication*)m_app)->UI->SetFocus(this);
		char str[512];
		m_game->ui.chatEdit->GetText(str, 512);
		if (strlen(str)) {
			Unit* u = UnitManager::LoadUnit(1);
			if (u)
				u->GetAI()->SetPosition(m_game->m_player->GetOriDev()->GetPosition());
		}
		m_game->m_ui.chatEdit->SetText("");
	} @TODO: CHANGE HERE*/
	return true;
}

void RTBClient::GameInputHandler::OnKeydown(short key) {
	/*
	switch (key) {
	case 'W':
		m_game->m_player->GetAI()->Forward(true);
		break;
	case 'S':
		m_game->m_player->GetAI()->Backward(true);
		break;
	case 'A':
		m_game->m_player->GetAI()->Left(true);
		break;
	case 'D':
		m_game->m_player->GetAI()->Right(true);
		break;
	case ' ':
		m_game->m_player->GetAI()->Jump(true);
		break;
	case '1':
		m_game->m_ui.stanceBar->SetStance(1);
		break;
	case '2':
		m_game->m_ui.stanceBar->SetStance(2);
		break;
	case '3':
		m_game->m_ui.stanceBar->SetStance(3);
		break;
	case '	':
		m_game->m_target = UnitManager::FindTarget(WVector2(m_game->m_ui.targetCursor->GetPositionX() + 128,
			m_game->m_ui.targetCursor->GetPositionY() + 128), 200);
		break;
	}
	@TODO: CHANGE HERE*/
}

void RTBClient::GameInputHandler::OnKeyup(short key) {
	/*
	char actionKeys[10] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
	switch (key) {
	case 'W':
		m_game->m_player->GetAI()->Forward(false);
		break;
	case 'S':
		m_game->m_player->GetAI()->Backward(false);
		break;
	case 'A':
		m_game->m_player->GetAI()->Left(false);
		break;
	case 'D':
		m_game->m_player->GetAI()->Right(false);
		break;
	case ' ':
		m_game->m_player->GetAI()->Jump(false);
		break;
	case '1':
		if (m_game->m_ui.stanceBar->GetCurrentStance() == 1)
			m_game->m_ui.stanceBar->SetStance(0);
		break;
	case '2':
		if (m_game->m_ui.stanceBar->GetCurrentStance() == 2)
			m_game->m_ui.stanceBar->SetStance(0);
		break;
	case '3':
		if (m_game->m_ui.stanceBar->GetCurrentStance() == 3)
			m_game->m_ui.stanceBar->SetStance(0);
		break;
	default:
		break;
	}
	@TODO: CHANGE HERE*/
}

void RTBClient::GameInputHandler::OnMouseButton(int mx, int my, bool bDown) {
	/*
	if (bDown) {
		if (WImage* img = m_game->m_ui.stanceBar->GetCurrentImage()) {
			if (ComboNode* n = m_game->m_player->GetComboSystem()->AdvanceCombo(m_game->m_ui.stanceBar->GetCurrentStance() - 1, 0, m_game->m_player, m_game->m_target)) {
				m_game->m_player->curStats.mana -= n->manaCost;
				m_game->m_player->curStats.energy -= n->energyCost;
				m_game->m_ui.castBar->Cast(img, m_game->m_player->GetComboSystem()->GetCurrentSpell()->GetTotalCastTime());
			}
		}
	}
	@TODO: CHANGE HERE*/
}

void RTBClient::GameInputHandler::OnMouseButton2(int mx, int my, bool bDown) {
	if (bDown)
		(std::dynamic_pointer_cast<RollTheBall::PlayerAI>(m_game->m_player->GetAI()))->BeginDragCamera();
	else
		(std::dynamic_pointer_cast<RollTheBall::PlayerAI>(m_game->m_player->GetAI()))->EndDragCamera();
	/*
	if (bDown) {
		if (WImage* img = m_game->m_ui.stanceBar->GetCurrentImage()) {
			if (ComboNode* n = m_game->m_player->GetComboSystem()->AdvanceCombo(m_game->m_ui.stanceBar->GetCurrentStance() - 1, 1, m_game->m_player, m_game->m_target)) {
				m_game->player->curStats.mana -= n->manaCost;
				m_game->player->curStats.energy -= n->energyCost;
				m_game->ui.castBar->Cast(img, m_game->m_player->GetComboSystem()->GetCurrentSpell()->GetTotalCastTime());
			}
		}
	}
	@TODO: CHANGE HERE*/
}

bool RTBClient::GameInputHandler::OnEscape() {
	m_game->m_app->SwitchState(nullptr);
	return false;
}
