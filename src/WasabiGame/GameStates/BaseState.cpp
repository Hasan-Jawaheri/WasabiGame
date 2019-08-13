#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/Main.hpp"
#include "WasabiGame/UI/UI.hpp"

BaseState::BaseState(Wasabi* app) : WGameState(app) {
}

BaseState::~BaseState() {
}

void BaseState::OnKeyDown(char c) {
	if (((WasabiRPG*)m_app)->UI->GetFocus()) {
		if (((WasabiRPG*)m_app)->UI->GetFocus()->OnInput())
			((WasabiRPG*)m_app)->UI->GetFocus()->OnKeydown(c);
		else //if it does not accept input, see if it's parent does
			if (((WasabiRPG*)m_app)->UI->GetFocus()->GetParent())
				if (((WasabiRPG*)m_app)->UI->GetFocus()->GetParent()->OnInput())
					((WasabiRPG*)m_app)->UI->GetFocus()->GetParent()->OnKeydown(c);
	}
}

void BaseState::OnKeyUp(char c) {
	if (((WasabiRPG*)m_app)->UI->GetFocus()) {
		if (((WasabiRPG*)m_app)->UI->GetFocus()->OnInput())
			((WasabiRPG*)m_app)->UI->GetFocus()->OnKeyup(c);
		else //if it does not accept input, see if it's parent does
			if (((WasabiRPG*)m_app)->UI->GetFocus()->GetParent())
				if (((WasabiRPG*)m_app)->UI->GetFocus()->GetParent()->OnInput())
					((WasabiRPG*)m_app)->UI->GetFocus()->GetParent()->OnKeyup(c);
	}
}

void BaseState::OnMouseDown(W_MOUSEBUTTON button, int mx, int my) {
	if (button == MOUSE_LEFT) {
		UIElement* targetElement = ((WasabiRPG*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement) {
			targetElement->OnMouseButton(mx, my, true);
			if (targetElement->GetSizeX() != 0 && targetElement->GetSizeY() != 0) {
				bool bGiveFocus = true;
				if (((WasabiRPG*)m_app)->UI->GetFocus())
					if (!((WasabiRPG*)m_app)->UI->GetFocus()->OnLoseFocus()) {
						bool bIsChild = false;
						for (UINT i = 0; i < ((WasabiRPG*)m_app)->UI->GetFocus()->GetNumChildren(); i++)
							if (((WasabiRPG*)m_app)->UI->GetFocus()->GetChild(i) == targetElement)
								bIsChild = true;
						if (!bIsChild)
							bGiveFocus = false;
					}

				if (targetElement->OnFocus() && bGiveFocus)
					((WasabiRPG*)m_app)->UI->SetFocus(targetElement);
			}
		}
	} else if (button == MOUSE_RIGHT) {
		UIElement* targetElement = ((WasabiRPG*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement)
			targetElement->OnMouseButton2(mx, my, true);
	}
}

void BaseState::OnMouseUp(W_MOUSEBUTTON button, int mx, int my) {
	if (button == MOUSE_LEFT) {
		UIElement* targetElement = ((WasabiRPG*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement) {
			bool bProcceed = true;
			if (((WasabiRPG*)m_app)->UI->GetFocus())
				if (!((WasabiRPG*)m_app)->UI->GetFocus()->OnLoseFocus()) {
					bool bIsChild = false;
					for (UINT i = 0; i < ((WasabiRPG*)m_app)->UI->GetFocus()->GetNumChildren(); i++)
						if (((WasabiRPG*)m_app)->UI->GetFocus()->GetChild(i) == targetElement)
							bIsChild = true;
					if (!bIsChild)
						bProcceed = false;
				}

			if (bProcceed)
				targetElement->OnMouseButton(mx, my, false);
		}
	} else if (button == MOUSE_RIGHT) {
		UIElement* targetElement = ((WasabiRPG*)m_app)->UI->GetElementAt(mx, my);
		if (targetElement)
			targetElement->OnMouseButton2(mx, my, false);
	}
}

void BaseState::OnMouseMove(int mx, int my) {
	UIElement* targetElement = ((WasabiRPG*)m_app)->UI->GetElementAt(mx, my);
	if (targetElement)
		targetElement->OnMouseMove(mx, my);
}

void BaseState::OnInput(char c) {
	if (((WasabiRPG*)m_app)->UI->GetFocus()) {
		// give input to the focus or the first ancestor that accepts input
		UIElement* target_element = ((WasabiRPG*)m_app)->UI->GetFocus();
		do {
			if (target_element->OnInput())
				break;
			else // element doesn't accept input, try its parent
				target_element = target_element->GetParent();
		} while (target_element);

		if (target_element) {
			if (c == VK_RETURN) {
				// propagate the return to ancestors as long as OnEnter returns true
				UIElement* cur_target_element = target_element;
				do {
					if (cur_target_element->OnEnter())
						cur_target_element = cur_target_element->GetParent();
					else
						cur_target_element = nullptr;
				} while (cur_target_element);
			} else if (c == VK_ESCAPE) {
				// propagate the escape to ancestors as long as OnEscape returns true
				UIElement* cur_target_element = target_element;
				do {
					if (cur_target_element->OnEscape())
						cur_target_element = cur_target_element->GetParent();
					else
						cur_target_element = nullptr;
				} while (cur_target_element);
			} else if (c == VK_TAB) {
				// switch the focus to a sibling
				if (target_element->OnTab()) {
					UIElement* parent = target_element->GetParent();
					if (parent) {
						UINT numChildren = parent->GetNumChildren();
						UINT focusIndex = -1;
						for (UINT i = 0; i <= numChildren; i++) {
							if (i == numChildren)
								i = 0;
							UIElement* cur_child = parent->GetChild(i);

							if (focusIndex != -1) {
								if (cur_child->OnTab()) {
									if (cur_child->OnFocus())
										((WasabiRPG*)m_app)->UI->SetFocus(cur_child);
									break;
								}
							}

							if (cur_child == ((WasabiRPG*)m_app)->UI->GetFocus())
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
