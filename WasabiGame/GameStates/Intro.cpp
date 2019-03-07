#include "Intro.hpp"
#include "Menu.hpp"

Intro::Intro(Wasabi* app) : WGameState(app) {
}

Intro::~Intro() {
}

void Intro::Load() {
	m_app->Renderer->GetDefaultRenderTarget()->SetClearColor(WColor(0, 0, 0));

	const char* logoFilenames[NUMLOGOS] = {
		"Media\\havok_logo.jpg",
		"Media\\HasX11 Engine.png"
	};

	// load images using the above filenames
	for (unsigned int i = 0; i < NUMLOGOS; i++) {
		m_logos[i] = new WImage(m_app); // allocate a new image
		m_logos[i]->Load(logoFilenames[i]); // load the image from file
	}

	// setup the sprite to display the current logo
	m_cur_logo = new WSprite(m_app);
	m_cur_logo->Load();

	// now set the initial values to start showing the logos
	m_cur_logo_index = 0; // start with the first image, index 0
	m_fade_in = true;
	m_cur_logo_alpha = -0.4f; // fade in - start with -0.4 alpha to idle for a while
}

void Intro::Update(float fDeltaTime) {
	// We allow the alpha to range from 0 to 1.5 although the maximum alpha (fully opaque
	// image) is 1.0 because we want the image to stay at full alpha for a while
	// and to be specific it will remain at full alpha for (1.5-1)/100 seconds.
	// But when we set the alpha value to the sprite we cap it at 1.0 so the range
	// 1.0 to 1.5 is always given to the sprite as 255, but we just use it for the delay
	// Fading at the rate of INTROSPEED alpha per second
	const float fMinAlpha = 0.0f;
	const float fMaxAlpha = 1.0f;
	const float fExcessAlpha = 1.5f;
	if (m_fade_in) { // fading in
		m_cur_logo_alpha += INTROSPEED * fDeltaTime;
		if (m_cur_logo_alpha > fExcessAlpha) { //excceeded maximum alpha, now fade out
			m_cur_logo_alpha = fMaxAlpha; //start fading out from 255
			m_fade_in = false; //start fading out
		}
	} else { //fading out
		m_cur_logo_alpha -= INTROSPEED * fDeltaTime;
		if (m_cur_logo_alpha < fMinAlpha) { //less than the minimum, now fade in a new image
			m_cur_logo_alpha = fMinAlpha;
			m_fade_in = true;
			m_cur_logo_index++; //jump to the new logo
			if (m_cur_logo_index >= NUMLOGOS) { //no more logos! switch to the game state!
				m_app->SwitchState(new Menu(m_app));
				return; //dont execute any further, we're done.
			}
		}
	}

	m_cur_logo->SetImage(m_logos[m_cur_logo_index]);
	m_cur_logo->SetAlpha(m_cur_logo_alpha);

	//position the sprite in the center of the screen
	int window_width = m_app->WindowComponent->GetWindowWidth();
	int window_height = m_app->WindowComponent->GetWindowHeight();
	float image_width = (float)m_logos[m_cur_logo_index]->GetWidth();
	float image_height = (float)m_logos[m_cur_logo_index]->GetHeight();
	float xpos = window_width / 2.0f - image_width / 2.0f;
	float ypos = window_height / 2.0f - image_height / 2.0f;
	m_cur_logo->SetPosition(xpos, ypos);
	m_cur_logo->SetSize(image_width, image_height);
}

void Intro::Cleanup() {
	//freeing resources with HX_SAFE_REMOVEREF
	for (unsigned int i = 0; i < NUMLOGOS; i++)
		W_SAFE_REMOVEREF(m_logos[i]);

	W_SAFE_REMOVEREF(m_cur_logo);
}
