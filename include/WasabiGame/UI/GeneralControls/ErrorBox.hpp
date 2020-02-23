#pragma once

#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/UI/InputControls/MenuButton.hpp"


namespace WasabiGame {

	class ErrorBox : public UIElement {
		std::string m_errorMsg;
		bool m_isClick;

	public:
		ErrorBox(std::shared_ptr<UserInterface> ui, std::string error);
		virtual ~ErrorBox() {}

		virtual void Load() override;
		virtual void OnResize(uint32_t width, uint32_t height) override;
		virtual bool Update(float fDeltaTime) override;

		virtual bool OnInput() override { return false; }
		virtual bool OnTab() override { return false; }
		virtual bool OnEnter() override { m_isClick = true; return false; }
		virtual bool OnEscape() override { m_isClick = true; return false; }
		virtual bool IsPassword() override { return false; }
		virtual bool OnFocus() override { return true; }
		virtual bool OnLoseFocus() override { return false; } //never lose focus
		virtual bool OnDisableAllUpdates() override { return true; } //disable updating all other elements

		//in order not to allow the UI manager to pick it for mouse events
		virtual int GetPosZ() const override { return 10; }
		virtual float GetSizeX() const override { return 0.0f; }
		virtual float GetSizeY() const override { return 0.0f; }
	};

	class ErrorButton : public MenuButton {
	public:
		ErrorButton(std::shared_ptr<UserInterface> ui, std::string text) : MenuButton(ui, text) {};
		virtual int GetPosZ() const override { return 10; }
	};
};
