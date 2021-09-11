#pragma once

#include "Common.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/UI/UI.hpp"
#include "RollTheBall/Units/Player.hpp"
#include "RollTheBall/GameModes/GameModes.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Networking/GameStateSyncProtocol.hpp"


namespace RTBClient {

	class MainGameState;

	class GameInputHandler : public WasabiGame::UIElement {
		MainGameState* m_game;

		bool m_inputEnabled;
		bool m_draggingCamera;
		double m_lastMouseX, m_lastMouseY;

	public:
		GameInputHandler(MainGameState* gameState);
		virtual ~GameInputHandler();

		virtual bool Update(float fDeltaTime) override;

		virtual void OnMouseMove(double mx, double my) override;
		virtual void OnMouseButton(double mx, double my, bool bDown) override;
		virtual void OnMouseButton2(double mx, double my, bool bDown) override;
		virtual bool OnFocus() override { return false; }
		virtual bool OnInput() override { return true; }
		virtual bool OnEnter() override;
		virtual void OnKeydown(uint32_t key) override;
		virtual void OnKeyup(uint32_t key) override;
		virtual bool OnEscape() override;
		virtual int GetPosZ() const override { return 0; }

		void EnableInput();
		void DisableInput();
	};

	class MainGameState : public WasabiGame::BaseGameState {

		friend class GameInputHandler;

		std::shared_ptr<RollTheBall::Player> m_player;
		std::shared_ptr<GameInputHandler> m_input;

		RollTheBall::MAP_NAME m_currentMap;
		RollTheBall::MAP_NAME m_mapToLoad;
		RollTheBall::RTB_GAME_MODE m_currentGameMode;

	public:
		MainGameState(Wasabi* app);
		virtual ~MainGameState();

		void Load();
		void Update(float fDeltaTime);
		void Cleanup();

		std::shared_ptr<RollTheBall::Player> GetPlayer() const;
	};

};
