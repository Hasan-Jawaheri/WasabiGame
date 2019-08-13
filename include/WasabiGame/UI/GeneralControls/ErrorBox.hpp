#pragma once

#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/UI/InputControls/MenuButton.hpp"

class ErrorBox : public UIElement {
	std::string m_error_msg;
	bool m_is_click;

public:
	ErrorBox(UserInterface* ui, std::string error);
	~ErrorBox() {}

	virtual void Load(Wasabi* const app);
	virtual void OnResize(UINT width, UINT height);
	virtual bool Update(float fDeltaTime);

	virtual bool OnInput() { return false; }
	virtual bool OnTab() { return false; }
	virtual bool OnEnter() { m_is_click = true; return false; }
	virtual bool OnEscape() { m_is_click = true; return false; }
	virtual bool IsPassword() { return false; }
	virtual bool OnFocus() { return true; }
	virtual bool OnLoseFocus() { return false; } //never lose focus
	virtual bool OnDisableAllUpdates() { return true; } //disable updating all other elements

	//in order not to allow the UI manager to pick it for mouse events
	virtual int GetPosZ() const { return 10; }
	virtual float GetSizeX() const { return 0.0f; }
	virtual float GetSizeY() const { return 0.0f; }
};

class ErrorButton : public MenuButton {
public:
	ErrorButton(UserInterface* ui, std::string text) : MenuButton(ui, text) {};
	virtual int GetPosZ() const { return 10; }
};
