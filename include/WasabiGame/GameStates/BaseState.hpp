#pragma once

#include "Common.hpp"


namespace WasabiGame {
		
	class BaseGameState : public WGameState {

	public:
		BaseGameState(Wasabi* app);
		virtual ~BaseGameState();

		virtual void OnKeyDown(char c);
		virtual void OnKeyUp(char c);
		virtual void OnMouseDown(W_MOUSEBUTTON button, int mx, int my);
		virtual void OnMouseUp(W_MOUSEBUTTON button, int mx, int my);
		virtual void OnMouseMove(int mx, int my);
		virtual void OnInput(char c);
	};

};
