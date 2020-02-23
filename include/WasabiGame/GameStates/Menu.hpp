#pragma once

#include "Common.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"


namespace WasabiGame {

	class MenuGameState : public BaseGameState {
		std::function<BaseGameState* ()> m_nextStateGenerator;

	public:
		MenuGameState(Wasabi* app, std::function<BaseGameState* ()> nextStateGenerator);
		~MenuGameState();

		void Load();
		void Update(float fDeltaTime);
		void Cleanup();
	};

};
