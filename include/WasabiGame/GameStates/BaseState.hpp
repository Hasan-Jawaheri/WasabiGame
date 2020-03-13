#pragma once

#include "Common.hpp"


namespace WasabiGame {
		
	class BaseGameState : public WGameState {

	public:
		BaseGameState(Wasabi* app);
		virtual ~BaseGameState();

		virtual void OnKeyDown(uint32_t c) override;
		virtual void OnKeyUp(uint32_t c) override;
		virtual void OnMouseDown(W_MOUSEBUTTON button, double mx, double my) override;
		virtual void OnMouseUp(W_MOUSEBUTTON button, double mx, double my) override;
		virtual void OnMouseMove(double mx, double my) override;
		virtual void OnInput(uint32_t c) override;
	};

};
