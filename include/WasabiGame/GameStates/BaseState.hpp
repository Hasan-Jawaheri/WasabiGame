#pragma once

#include "Common.hpp"
#include "WasabiGame/UI/UI.hpp"

class BaseState : public WGameState {

public:
	BaseState(Wasabi* app);
	virtual ~BaseState();

	virtual void OnKeyDown(char c);
	virtual void OnKeyUp(char c);
	virtual void OnMouseDown(W_MOUSEBUTTON button, int mx, int my);
	virtual void OnMouseUp(W_MOUSEBUTTON button, int mx, int my);
	virtual void OnMouseMove(int mx, int my);
	virtual void OnInput(char c);
};
