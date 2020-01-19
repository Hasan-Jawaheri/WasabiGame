#pragma once

#include "Common.hpp"
#include "WasabiGame/GameStates/BaseState.hpp"


namespace WasabiGame {

	const float INTRO_SPEED = 1.5f;

	class IntroGameState : public BaseGameState {
		std::vector<std::string> m_logoFiles;
		std::vector<WImage*> m_logos;	// an array of images to be displayed
		WSprite* m_cur_logo;			// a sprite that displays the currently shown logo
		unsigned int m_cur_logo_index;	// index of the current logo
		float m_cur_logo_alpha;			// alpha value of the current logo
		bool m_fade_in;					// fading in or out?
		std::function<BaseGameState* ()> m_nextStateGenerator;

	public:
		IntroGameState(Wasabi* app, std::vector<std::string> images, std::function<BaseGameState*()> nextStateGenerator);
		~IntroGameState();

		void Load();
		void Update(float fDeltaTime);
		void Cleanup();
	};

};
