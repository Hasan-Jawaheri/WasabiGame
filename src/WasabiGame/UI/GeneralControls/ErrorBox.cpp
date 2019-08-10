#include "WasabiGame/UI/GeneralControls/ErrorBox.hpp"

class ErrorBoxPS : public WShader {
public:
	ErrorBoxPS(class Wasabi* const app) : WShader(app) {}

	virtual void Load(bool bSaveData = false) {
		m_desc.type = W_FRAGMENT_SHADER;
		m_desc.bound_resources = {
			W_BOUND_RESOURCE(W_TYPE_UBO, 0, "uboPerSprite", {
				W_SHADER_VARIABLE_INFO(W_TYPE_FLOAT, "alpha"),
				W_SHADER_VARIABLE_INFO(W_TYPE_VEC_2, "spriteSize"),
			}),
		};
		vector<byte> code = {
			#include "ErrorBox.frag.glsl.spv"
		};
		LoadCodeSPIRV((char*)code.data(), code.size(), bSaveData);
	}
};

ErrorBox::ErrorBox(std::string error) {
	m_error_msg = error;
	m_is_click = false;
}

void ErrorBox::Load(Wasabi* const app) {
	m_sprite = app->SpriteManager->CreateSprite();
	m_material = CreateSpriteMaterial<ErrorBoxPS>(app, m_sprite);
	m_sprite->SetPriority(0);

	OnResize(app->WindowAndInputComponent->GetWindowWidth(), app->WindowAndInputComponent->GetWindowHeight());

	SetFade(1.0f);
}

void ErrorBox::OnResize(UINT width, UINT height) {
	UIElement::OnResize(width, height);
	float boxHeight = 200.0f + 50.0f * (float)(GetNumChildren() - 1);
	SetPosition((float)width / 2.0f - 200.0f, (float)height / 2.0f - (boxHeight/2.0f));
	SetSize(400, boxHeight);
}

bool ErrorBox::Update(float fDeltaTime) {
	float x = m_sprite->GetPosition().x;
	float y = m_sprite->GetPosition().y;
	float boxHeight = 200.0f + 50.0f * (float)(GetNumChildren() - 1);
	uint numChildren = GetNumChildren();
	for (uint i = 0; i < numChildren; i++)
		GetChild(i)->SetPosition(x + m_sprite->GetSize().x / 2 - 150, y + m_sprite->GetSize().y - ((numChildren - i) * 50));

	m_sprite->GetAppPtr()->TextComponent->RenderText(m_error_msg, x + 20.0f, y + 20.0f, 16.0f, FONT_CALIBRI_16, WColor(1.0f, 0, 0, 1.0f));
	return !m_is_click;
}
