#include "Game.hpp"
#include "../UI/GeneralControls/ErrorBox.hpp"
/*#include "..//Maps//Map.hpp"
#include "..//Entities//Player.hpp"
#include "..//Spells//Warrior Spells//Warrior Spells.hpp"
#include "..//UI//Game Controls//Cast Bar.hpp"
#include "..//UI//Game Controls//Health Bar.hpp"
#include "..//UI//Game Controls//Stance Bar.hpp"
#include "..//UI//Game Controls//Target Cursor.hpp"
#include "..//UI//Game Controls//Chat.hpp"
#include "..//Quests//Quests.hpp" @TODO: CHANGE HERE*/

Game::Game(Wasabi* app) : WGameState(app) {
	m_player = nullptr;
	m_target = nullptr;
}

Game::~Game() {
}

void Game::Load() {
	/*//
	// Load the worlds essential resources and initialize the unit manager
	//
	AIFactory::Init();
	UnitManager::Initialize();

	//
	// Initialize spells and buffs
	//
	SpellManager::LoadSpellResources();
	BuffManager::LoadBuffResources();

	//
	// Load the player
	//
	m_player = new Player();
	UnitManager::LoadUnit(0, m_player);
	UnitManager::SetPlayer(player);
	player->SetAI(new PlayerAI(8.0f, 2.5f));
	m_target = nullptr;

	//
	// Load the quest system and progress
	//
	QuestManager::Initialize();
	
	@TODO: CHANGE HERE
	*/

	//
	// Setup and load the user interface
	//
	UserInterface::AddUIElement(m_input = new GameInputHandler(this), nullptr);
	UserInterface::AddUIElement(new ErrorBox("DAFAQ"), m_input);
	/*UserInterface::AddUIElement(m_ui.castBar = new CastBar(), m_input);
	UserInterface::AddUIElement(m_ui.playerHealth = new HealthBar(true), m_input);
	UserInterface::AddUIElement(m_ui.targetHealth = new HealthBar(false), m_input);
	UserInterface::AddUIElement(m_ui.stanceBar = new StanceBar(), m_input);
	UserInterface::AddUIElement(m_ui.targetCursor = new TargetCursor(), m_input);
	UserInterface::AddUIElement(m_ui.chatEdit = new ChatEditBox(), m_input); @TODO: CHANGE HERE*/
	UserInterface::Load(m_app);

	/*m_ui.playerHealth->Open();
	m_ui.targetHealth->Open();
	m_ui.stanceBar->Open();
	m_ui.chatEdit->SetFade(1.0f);
	m_ui.chatEdit->SetSize(400, 20);

	//
	// Load the map
	//
	Map::Set(1);
	
	@TODO: CHANGE HERE
	*/
}

void Game::Update(float fDeltaTime) {
	/*int offset = 220;
	float sw = m_app->WindowComponent->GetWindowWidth();
	float sh = m_app->WindowComponent->GetWindowHeight();
	m_ui.castBar->SetPosition(sw / 2 - m_ui.castBar->GetSizeX() / 2, sh - offset);
	int cbx = m_ui.playerHealth->GetSizeX();
	int cby = m_ui.castBar->GetPositionY();
	m_ui.targetHealth->SetPosition(sw / 2 + 38, cby + 64 - (m_ui.playerHealth->GetSizeY() / 2) + 14);
	m_ui.playerHealth->SetPosition(sw / 2 - 38 - cbx, cby + 64 - (m_ui.playerHealth->GetSizeY() / 2) + 14);
	m_ui.stanceBar->SetPosition(sw / 2, cby + 100 - 3);
	m_ui.targetCursor->SetPosition(sw / 2 - 128, sh / 2 - 128 - 50);
	m_ui.chatEdit->SetPosition(10, sh - 30);

	if (m_player) {
		m_ui.playerHealth->SetHealth(m_player->curStats.health * 100.0f / m_player->curStats.maxHealth);
		m_ui.playerHealth->SetPower(m_player->curStats.mana * 100.0f / m_player->curStats.maxMana);

		/ **************************************************************** /
		WOrientation* orientation = m_player->GetOriDev();
		if (orientation) {
			char text[256];
			WVector3 pos = orientation->GetPosition();
			WVector3 ang = orientation->GetAngle();
			sprintf_s(text, 256, "(%.2f,%.2f,%.2f) - (%.2f,%.2f,%.2f)", pos.x, pos.y, pos.z, ang.x, ang.y, ang.z);
			m_app->TextComponent->RenderText(text, 5, 100, 16, FONT_CALIBRI_16, WColor(0.4, 0.4, 1));
		}
		/ **************************************************************** /
	}
	if (m_target && UnitManager::UnitExists(m_target)) {
		m_ui.targetHealth->SetHealth(m_target->curStats.health * 100.0f / m_target->curStats.maxHealth);
		m_ui.targetHealth->SetPower(m_target->curStats.mana * 100.0f / m_target->curStats.maxMana);
		m_ui.targetHealth->Open();
	} else {
		m_ui.targetHealth->Close();
		m_target = nullptr;
	}

	Map::Get()->Update(fDeltaTime);
	UnitManager::Update(fDeltaTime);
	if (!UnitManager::UnitExists(m_target))
		target = nullptr; //if target dies, make it null
	BuffManager::Update(fDeltaTime);
	SpellManager::Update(fDeltaTime);
	
	@TODO: CHANGE HERE
	*/
}

void Game::Cleanup() {
	/*BuffManager::Cleanup();
	SpellManager::Cleanup();
	QuestManager::Cleanup();
	W_SAFE_DELETE(m_player);
	UnitManager::SetPlayer(nullptr);
	UnitManager::Cleanup();
	AIFactory::Cleanup();
	
	@TODO: CHANGE HERE
	*/
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
