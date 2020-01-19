#include "WasabiGame/UI/GeneralControls/ErrorBox.hpp"
#include "WasabiGame/Main.hpp"


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

WasabiGame::ErrorBox::ErrorBox(std::shared_ptr<UserInterface> ui, std::string error) : UIElement(ui) {
	m_errorMsg = error;
	m_isClick = false;
}

void WasabiGame::ErrorBox::Load() {
	std::shared_ptr<WasabiBaseGame> app = m_UI.get()->GetApp().lock();

	m_sprite = app.get()->SpriteManager->CreateSprite();
	m_material = CreateSpriteMaterial<ErrorBoxPS>(app, m_sprite);
	m_sprite->SetPriority(0);

	OnResize(app.get()->WindowAndInputComponent->GetWindowWidth(), app.get()->WindowAndInputComponent->GetWindowHeight());

	SetFade(1.0f);
}

void WasabiGame::ErrorBox::OnResize(uint32_t width, uint32_t height) {
	UIElement::OnResize(width, height);
	float boxHeight = 200.0f + 50.0f * (float)(GetNumChildren() - 1);
	SetPosition((float)width / 2.0f - 200.0f, (float)height / 2.0f - (boxHeight/2.0f));
	SetSize(400, boxHeight);
}

bool WasabiGame::ErrorBox::Update(float fDeltaTime) {
	float x = m_sprite->GetPosition().x;
	float y = m_sprite->GetPosition().y;
	float boxHeight = 200.0f + 50.0f * (float)(GetNumChildren() - 1);
	uint32_t numChildren = GetNumChildren();
	for (uint32_t i = 0; i < numChildren; i++)
		GetChild(i)->SetPosition(x + m_sprite->GetSize().x / 2 - 150, y + m_sprite->GetSize().y - ((numChildren - i) * 50));

	m_sprite->GetAppPtr()->TextComponent->RenderText(m_errorMsg, x + 20.0f, y + 20.0f, 16.0f, FONT_CALIBRI_16, WColor(1.0f, 0, 0, 1.0f));
	return !m_isClick;
}
