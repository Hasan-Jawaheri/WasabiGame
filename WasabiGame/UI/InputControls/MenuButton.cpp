#include "MenuButton.hpp"

MenuButton::MenuButton(std::string text) {
	m_text = text;
	m_is_clicked = false;
}

MenuButton::~MenuButton() {
}

void MenuButton::Load(Wasabi* const app) {
	m_material = CreateSpriteMaterial<MenuButtonPS>(app);
	m_sprite = new WSprite(app);
	m_sprite->SetMaterial(m_material);
	m_sprite->SetAlpha(0.0f);
	m_sprite->SetPriority(0);
	m_sprite->SetSize(300, 40);

	SetFade(1.0f);
	m_alpha_offset = 0.5f;
}

void MenuButton::SetFade(float fFade) {
	m_sprite->SetAlpha(max((fFade - m_alpha_offset), 0));
	m_alpha = fFade;
}

void MenuButton::OnResize(UINT width, UINT height) {
}

bool MenuButton::Update(float fDeltaTime) {
	Wasabi* app = m_sprite->GetAppPtr();

	if (app->InputComponent->MouseX() > m_sprite->GetPositionX() && app->InputComponent->MouseY() > m_sprite->GetPositionY() &&
		app->InputComponent->MouseX() < m_sprite->GetPositionX() + m_sprite->GetSizeX() &&
		app->InputComponent->MouseY() < m_sprite->GetPositionY() + m_sprite->GetSizeY()) {
		if (m_alpha_offset > 0.15f)
			m_alpha_offset -= 5.0f * fDeltaTime;
		if (app->InputComponent->MouseClick(MOUSE_LEFT)) {
			if (m_alpha_offset > 0.0f)
				m_alpha_offset -= 5.0f * fDeltaTime;
		} else if (m_alpha_offset < 0.15f)
			m_alpha_offset += 5.0f * fDeltaTime;
		if (m_alpha_offset < 0.0f)
			m_alpha_offset = 0.0f;
		if (m_alpha_offset > 0.3f)
			m_alpha_offset = 0.3f;
	} else if (m_alpha_offset < 0.3f)
		m_alpha_offset += m_alpha_offset / 15.0f;

	m_sprite->SetAlpha(max(m_alpha - m_alpha_offset, 0));

	WColor c(1, 1, 1, 1);
	float text_width = app->TextComponent->GetTextWidth(m_text, 16.0f, FONT_CALIBRI_16);
	float text_height = 16.0f;
	app->TextComponent->RenderText(
		m_text,
		m_sprite->GetPositionX() + m_sprite->GetSizeX() / 2.0f - text_width/2.0f,
		m_sprite->GetPositionY() + m_sprite->GetSizeY() / 2.0f - text_height / 2.0f,
		16.0f, FONT_CALIBRI_16, WColor(c.r, c.g, c.b, max(m_alpha - m_alpha_offset, 0))
	);

	return true;
}

void MenuButton::OnMouseButton(int mx, int my, bool bDown) {
	if (!bDown) {
		m_is_clicked = true;
		if (GetParent())
			GetParent()->OnEnter(); //send ENTER message to parent, the parent should do the OnClick of this
		else
			OnClick();
	}
}
