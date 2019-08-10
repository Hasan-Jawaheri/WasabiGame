#include "WasabiGame/UI/InputControls/MenuButton.hpp"

class MenuButtonPS : public WShader {
public:
	MenuButtonPS(class Wasabi* const app) : WShader(app) {}

	virtual void Load(bool bSaveData = false) {
		m_desc.type = W_FRAGMENT_SHADER;
		m_desc.bound_resources = {
			W_BOUND_RESOURCE(W_TYPE_UBO, 0, "uboPerSprite", {
				W_SHADER_VARIABLE_INFO(W_TYPE_FLOAT, "alpha"),
			}),
		};
		vector<byte> code = {
			#include "MenuButton.frag.glsl.spv"
		};
		LoadCodeSPIRV((char*)code.data(), code.size(), bSaveData);
	}
};

MenuButton::MenuButton(std::string text) {
	m_text = text;
	m_is_clicked = false;
}

MenuButton::~MenuButton() {
}

void MenuButton::Load(Wasabi* const app) {
	m_sprite = app->SpriteManager->CreateSprite();
	m_material = CreateSpriteMaterial<MenuButtonPS>(app, m_sprite);

	SetFade(0.0f);
	m_sprite->SetPriority(0);
	m_sprite->SetSize(WVector2(300.0f, 40.0f));

	SetFade(1.0f);
	m_alpha_offset = 0.5f;
}

void MenuButton::SetFade(float fFade) {
	m_alpha = fFade;
	if (m_material)
		m_material->SetVariableFloat("alpha", max((fFade - m_alpha_offset), 0));
}

void MenuButton::OnResize(UINT width, UINT height) {
}

bool MenuButton::Update(float fDeltaTime) {
	Wasabi* app = m_sprite->GetAppPtr();

	if (app->WindowAndInputComponent->MouseX() > m_sprite->GetPosition().x && app->WindowAndInputComponent->MouseY() > m_sprite->GetPosition().y &&
		app->WindowAndInputComponent->MouseX() < m_sprite->GetPosition().x + m_sprite->GetSize().x &&
		app->WindowAndInputComponent->MouseY() < m_sprite->GetPosition().y + m_sprite->GetSize().y) {
		if (m_alpha_offset > 0.15f)
			m_alpha_offset -= 5.0f * fDeltaTime;
		if (app->WindowAndInputComponent->MouseClick(MOUSE_LEFT)) {
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

	SetFade(m_alpha);

	WColor c(1, 1, 1, 1);
	float text_width = app->TextComponent->GetTextWidth(m_text, 16.0f, FONT_CALIBRI_16);
	float text_height = 16.0f;
	app->TextComponent->RenderText(
		m_text,
		m_sprite->GetPosition().x + m_sprite->GetSize().x / 2.0f - text_width/2.0f,
		m_sprite->GetPosition().y + m_sprite->GetSize().y / 2.0f - text_height / 2.0f,
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
