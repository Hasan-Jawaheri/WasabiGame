#pragma once

#include "..//Common.hpp"

#define NUMLOGOS 2
#define INTROSPEED 1.5f

class Intro : public WGameState {

	WImage* m_logos[NUMLOGOS];		//an array of images to be displayed
	WSprite* m_cur_logo;			//a sprite that displays the currently shown logo
	unsigned int m_cur_logo_index;	//index of the current logo
	float m_cur_logo_alpha;			//alpha value of the current logo
	bool m_fade_in;					//fading in or out?

public:
	Intro(Wasabi* app);
	~Intro();

	void Load();
	void Update(float fDeltaTime);
	void Cleanup();
};
