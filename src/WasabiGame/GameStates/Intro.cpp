#include "WasabiGame/GameStates/Intro.hpp"
#include "WasabiGame/Main.hpp"


WasabiGame::IntroGameState::IntroGameState(Wasabi* app, std::vector<std::string> images, std::function<BaseGameState* ()> nextStateGenerator) : BaseGameState(app) {
	m_logoFiles = images;
	m_nextStateGenerator = nextStateGenerator;
}

WasabiGame::IntroGameState::~IntroGameState() {
}

void WasabiGame::IntroGameState::Load() {
	m_app->Renderer->GetRenderTarget()->SetClearColor(WColor(0, 0, 0));

	// load images using the above filenames
	m_logos.resize(m_logoFiles.size());
	for (unsigned int i = 0; i < m_logos.size(); i++) {
		m_logos[i] = new WImage(m_app); // allocate a new image
		m_logos[i]->Load(m_logoFiles[i]); // load the image from file
	}

	// setup the sprite to display the current logo
	m_cur_logo = m_app->SpriteManager->CreateSprite();

	// now set the initial values to start showing the logos
	m_cur_logo_index = 0; // start with the first image, index 0
	m_fade_in = true;
	m_cur_logo_alpha = -0.4f; // fade in - start with -0.4 alpha to idle for a while
}

void WasabiGame::IntroGameState::Update(float fDeltaTime) {
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
		m_cur_logo_alpha += INTRO_SPEED * fDeltaTime;
		if (m_cur_logo_alpha > fExcessAlpha) { //excceeded maximum alpha, now fade out
			m_cur_logo_alpha = fMaxAlpha; //start fading out from 255
			m_fade_in = false; //start fading out
		}
	} else { //fading out
		m_cur_logo_alpha -= INTRO_SPEED * fDeltaTime;
		if (m_cur_logo_alpha < fMinAlpha) { //less than the minimum, now fade in a new image
			m_cur_logo_alpha = fMinAlpha;
			m_fade_in = true;
			m_cur_logo_index++; //jump to the new logo
			if (m_cur_logo_index >= m_logos.size()) { //no more logos! switch to the game state!
				m_app->SwitchState(m_nextStateGenerator());
				return; //dont execute any further, we're done.
			}
		}
	}

	m_cur_logo->GetMaterials().SetTexture("diffuseTexture", m_logos[m_cur_logo_index]);
	m_cur_logo->GetMaterials().SetVariable("alpha", m_cur_logo_alpha);

	//position the sprite in the center of the screen
	int window_width = m_app->WindowAndInputComponent->GetWindowWidth();
	int window_height = m_app->WindowAndInputComponent->GetWindowHeight();
	float image_width = (float)m_logos[m_cur_logo_index]->GetWidth();
	float image_height = (float)m_logos[m_cur_logo_index]->GetHeight();
	float xpos = window_width / 2.0f - image_width / 2.0f;
	float ypos = window_height / 2.0f - image_height / 2.0f;
	m_cur_logo->SetPosition(WVector2(xpos, ypos));
	m_cur_logo->SetSize(WVector2(image_width, image_height));
}

void WasabiGame::IntroGameState::Cleanup() {
	//freeing resources with HX_SAFE_REMOVEREF
	for (unsigned int i = 0; i < m_logos.size(); i++)
		W_SAFE_REMOVEREF(m_logos[i]);
	m_logos.clear();

	W_SAFE_REMOVEREF(m_cur_logo);
}
