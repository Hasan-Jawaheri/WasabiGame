#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/Main.hpp"
#include "WasabiGame/UI/UI.hpp"


WasabiGame::BaseGameState::BaseGameState(Wasabi* app) : WGameState(app) {
}

WasabiGame::BaseGameState::~BaseGameState() {
}

void WasabiGame::BaseGameState::OnKeyDown(char c) {
	if (((WasabiBaseGame*)m_app)->UI->GetFocus()) {
		if (((WasabiBaseGame*)m_app)->UI->GetFocus()->OnInput())
			((WasabiBaseGame*)m_app)->UI->GetFocus()->OnKeydown(c);
		else //if it does not accept input, see if it's parent does
			if (((WasabiBaseGame*)m_app)->UI->GetFocus()->GetParent())
				if (((WasabiBaseGame*)m_app)->UI->GetFocus()->GetParent()->OnInput())
					((WasabiBaseGame*)m_app)->UI->GetFocus()->GetParent()->OnKeydown(c);
	}
}

void WasabiGame::BaseGameState::OnKeyUp(char c) {
	if (((WasabiBaseGame*)m_app)->UI->GetFocus()) {
		if (((WasabiBaseGame*)m_app)->UI->GetFocus()->OnInput())
			((WasabiBaseGame*)m_app)->UI->GetFocus()->OnKeyup(c);
		else //if it does not accept input, see if it's parent does
			if (((WasabiBaseGame*)m_app)->UI->GetFocus()->GetParent())
				if (((WasabiBaseGame*)m_app)->UI->GetFocus()->GetParent()->OnInput())
					((WasabiBaseGame*)m_app)->UI->GetFocus()->GetParent()->OnKeyup(c);
	}
}

void WasabiGame::BaseGameState::OnMouseDown(W_MOUSEBUTTON button, int mx, int my) {
	if (button == MOUSE_LEFT) {
		std::shared_ptr<UIElement> targetElement = ((WasabiBaseGame*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement) {
			targetElement->OnMouseButton(mx, my, true);
			if (targetElement->GetSizeX() != 0 && targetElement->GetSizeY() != 0) {
				bool bGiveFocus = true;
				if (((WasabiBaseGame*)m_app)->UI->GetFocus())
					if (!((WasabiBaseGame*)m_app)->UI->GetFocus()->OnLoseFocus()) {
						bool bIsChild = false;
						for (UINT i = 0; i < ((WasabiBaseGame*)m_app)->UI->GetFocus()->GetNumChildren(); i++)
							if (((WasabiBaseGame*)m_app)->UI->GetFocus()->GetChild(i) == targetElement)
								bIsChild = true;
						if (!bIsChild)
							bGiveFocus = false;
					}

				if (targetElement->OnFocus() && bGiveFocus)
					((WasabiBaseGame*)m_app)->UI->SetFocus(targetElement);
			}
		}
	} else if (button == MOUSE_RIGHT) {
		std::shared_ptr<UIElement> targetElement = ((WasabiBaseGame*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement)
			targetElement->OnMouseButton2(mx, my, true);
	}
}

void WasabiGame::BaseGameState::OnMouseUp(W_MOUSEBUTTON button, int mx, int my) {
	if (button == MOUSE_LEFT) {
		std::shared_ptr<UIElement> targetElement = ((WasabiBaseGame*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement) {
			bool bProcceed = true;
			if (((WasabiBaseGame*)m_app)->UI->GetFocus())
				if (!((WasabiBaseGame*)m_app)->UI->GetFocus()->OnLoseFocus()) {
					bool bIsChild = false;
					for (UINT i = 0; i < ((WasabiBaseGame*)m_app)->UI->GetFocus()->GetNumChildren(); i++)
						if (((WasabiBaseGame*)m_app)->UI->GetFocus()->GetChild(i) == targetElement)
							bIsChild = true;
					if (!bIsChild)
						bProcceed = false;
				}

			if (bProcceed)
				targetElement->OnMouseButton(mx, my, false);
		}
	} else if (button == MOUSE_RIGHT) {
		std::shared_ptr<UIElement> targetElement = ((WasabiBaseGame*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement)
			targetElement->OnMouseButton2(mx, my, false);
	}
}

void WasabiGame::BaseGameState::OnMouseMove(int mx, int my) {
	std::shared_ptr<UIElement> targetElement = ((WasabiBaseGame*)m_app)->UI->GetElementAt(mx, my);
	if (targetElement)
		targetElement->OnMouseMove(mx, my);
}

void WasabiGame::BaseGameState::OnInput(char c) {
	if (((WasabiBaseGame*)m_app)->UI->GetFocus()) {
		// give input to the focus or the first ancestor that accepts input
		std::shared_ptr<UIElement> target_element = ((WasabiBaseGame*)m_app)->UI->GetFocus();
		do {
			if (target_element->OnInput())
				break;
			else // element doesn't accept input, try its parent
				target_element = target_element->GetParent();
		} while (target_element);

		if (target_element) {
			if (c == VK_RETURN) {
				// propagate the return to ancestors as long as OnEnter returns true
				std::shared_ptr<UIElement> cur_target_element = target_element;
				do {
					if (cur_target_element->OnEnter())
						cur_target_element = cur_target_element->GetParent();
					else
						cur_target_element = nullptr;
				} while (cur_target_element);
			} else if (c == VK_ESCAPE) {
				// propagate the escape to ancestors as long as OnEscape returns true
				std::shared_ptr<UIElement> cur_target_element = target_element;
				do {
					if (cur_target_element->OnEscape())
						cur_target_element = cur_target_element->GetParent();
					else
						cur_target_element = nullptr;
				} while (cur_target_element);
			} else if (c == VK_TAB) {
				// switch the focus to a sibling
				if (target_element->OnTab()) {
					std::shared_ptr<UIElement> parent = target_element->GetParent();
					if (parent) {
						UINT numChildren = parent->GetNumChildren();
						UINT focusIndex = -1;
						for (UINT i = 0; i <= numChildren; i++) {
							if (i == numChildren)
								i = 0;
							std::shared_ptr<UIElement> cur_child = parent->GetChild(i);

							if (focusIndex != -1) {
								if (cur_child->OnTab()) {
									if (cur_child->OnFocus())
										((WasabiBaseGame*)m_app)->UI->SetFocus(cur_child);
									break;
								}
							}

							if (cur_child == ((WasabiBaseGame*)m_app)->UI->GetFocus())
								focusIndex = i;
						}
					}
				}
			} else if (c == VK_BACK) {
				// remove from buffer
				target_element->GetInputBuffer() = target_element->GetInputBuffer().substr(0, target_element->GetInputBuffer().length() - 1);
			} else {
				// append to buffer
				target_element->GetInputBuffer() += c;
			}
		}
	}
}
