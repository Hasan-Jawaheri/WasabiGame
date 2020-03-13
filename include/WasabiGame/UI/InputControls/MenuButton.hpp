#pragma once

#include "WasabiGame/UI/UI.hpp"


namespace WasabiGame {

	class UserInterface;

	class MenuButton : public UIElement {
		std::string m_text;
		float m_alphaOffset;
		bool m_isClicked;

	public:
		MenuButton(std::shared_ptr<UserInterface> ui, std::string text);
		virtual ~MenuButton();

		virtual void Load() override;
		virtual void SetFade(float fFade) override;
		virtual void OnResize(uint32_t width, uint32_t height) override;
		virtual bool Update(float fDeltaTime) override;

		virtual bool OnInput() override { return false; }
		virtual bool OnTab() override { return false; }
		virtual bool OnEnter() override { return true; }
		virtual bool OnEscape() override { return false; }
		virtual void OnMouseButton(double mx, double my, bool bDown) override;
		virtual bool OnFocus() override { return false; } //cannot gain focus

		virtual void OnClick() {}
		bool IsClicked() { if (m_isClicked) { m_isClicked = !m_isClicked; return !m_isClicked; } return m_isClicked; }
	};

};
