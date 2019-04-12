#include "ErrorBox.hpp"

ErrorBox::ErrorBox(std::string error) {
	m_error_msg = error;
	m_is_click = false;
}

void ErrorBox::Load(Wasabi* const app) {
	m_sprite = app->SpriteManager->CreateSprite();
	m_material = CreateSpriteMaterial<ErrorBoxPS>(app, m_sprite);
	m_sprite->SetPriority(0);

	SetSize(400, 200);
	OnResize(app->WindowAndInputComponent->GetWindowWidth(), app->WindowAndInputComponent->GetWindowHeight());

	SetFade(1.0f);
}

void ErrorBox::OnResize(UINT width, UINT height) {
	UIElement::OnResize(width, height);
	SetPosition((float)width / 2.0f - 200.0f, (float)height / 2.0f - 100.0f);
}

bool ErrorBox::Update(float fDeltaTime) {
	float x = m_sprite->GetPosition().x;
	float y = m_sprite->GetPosition().y;
	if (GetNumChildren())
		GetChild(0)->SetPosition(x + m_sprite->GetSize().x / 2 - 150, y + m_sprite->GetSize().y / 2 + 50);

	m_sprite->GetAppPtr()->TextComponent->RenderText(m_error_msg, x + 20.0f, y + 20.0f, 16.0f, FONT_CALIBRI_16, WColor(1.0f, 0, 0, 1.0f));
	return !m_is_click;
}
