#pragma once

#include "..//UI.hpp"

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

class MenuButton : public UIElement {
	std::string m_text;
	float m_alpha_offset;
	bool m_is_clicked;

public:
	MenuButton(std::string text);
	~MenuButton();

	virtual void Load(Wasabi* const app);
	virtual void SetFade(float fFade);
	virtual void OnResize(UINT width, UINT height);
	virtual bool Update(float fDeltaTime);

	virtual bool OnInput() { return false; }
	virtual bool OnTab() { return false; }
	virtual bool OnEnter() { return true; }
	virtual bool OnEscape() { return false; }
	virtual void OnMouseButton(int mx, int my, bool bDown);
	virtual bool OnFocus() { return false; } //cannot gain focus

	virtual void OnClick() {};
	bool IsClicked() { if (m_is_clicked) { m_is_clicked = !m_is_clicked; return !m_is_clicked; } return m_is_clicked; }
};

