#pragma once

#include "Common.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/UI/UI.hpp"
#include "RollTheBall/Units/Player.hpp"


namespace RTBClient {

	class MainGameState;

	class GameInputHandler : public WasabiGame::UIElement {
		MainGameState* m_game;

	public:
		GameInputHandler(MainGameState* gameState);
		virtual void OnMouseButton(int mx, int my, bool bDown);
		virtual void OnMouseButton2(int mx, int my, bool bDown);
		virtual bool OnFocus() { return false; }
		virtual bool OnInput() { return true; }
		virtual bool OnEnter();
		virtual void OnKeydown(short key);
		virtual void OnKeyup(short key);
		virtual bool OnEscape();
		virtual int GetPosZ() const { return 0; }
	};

	class MainGameState : public WasabiGame::BaseGameState {

		friend class GameInputHandler;

		std::shared_ptr<RollTheBall::Player> m_player;

		std::shared_ptr<GameInputHandler> m_input;

	public:
		MainGameState(Wasabi* app);
		~MainGameState();

		void Load();
		void Update(float fDeltaTime);
		void Cleanup();
	};

};
