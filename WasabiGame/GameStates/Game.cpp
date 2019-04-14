#include "Game.hpp"
#include "../UI/GeneralControls/ErrorBox.hpp"
#include "../Maps/MapLoader.hpp"

Game::Game(Wasabi* app) : WGameState(app) {
	m_player = nullptr;
	m_target = nullptr;
}

Game::~Game() {
}

void Game::Load() {
	//
	// Setup and load the user interface
	//
	UserInterface::AddUIElement(m_input = new GameInputHandler(this), nullptr);
	UIElement* ok = new MenuButton("sure");
	UIElement* no = new MenuButton("nop");
	UIElement* err = new ErrorBox("DAFAQ??");
	UserInterface::AddUIElement(err, m_input);
	UserInterface::AddUIElement(no, err);
	UserInterface::AddUIElement(ok, err);
	UserInterface::Load(m_app);

	//
	// Load the map
	//
	MapLoader::SetMap(MAP_ICC);
}

void Game::Update(float fDeltaTime) {
}

void Game::Cleanup() {
}

GameInputHandler::GameInputHandler(class Game* g) {
	m_game = g;
}

bool GameInputHandler::OnEnter() {
	/*
	if (UserInterface::GetFocus() != (UIElement*)m_game->m_ui.chatEdit) {
		m_game->ui.chatEdit->OnFocus();
		UserInterface::SetFocus(m_game->m_ui.chatEdit);
	} else {
		UserInterface::SetFocus(this);
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

void GameInputHandler::OnKeydown(short key) {
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

void GameInputHandler::OnKeyup(short key) {
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

void GameInputHandler::OnMouseButton(int mx, int my, bool bDown) {
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

void GameInputHandler::OnMouseButton2(int mx, int my, bool bDown) {
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

bool GameInputHandler::OnEscape() {
	APPHANDLE->SwitchState(nullptr);
	return false;
}
