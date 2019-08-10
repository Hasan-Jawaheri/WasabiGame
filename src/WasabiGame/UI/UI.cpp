#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/UI/GeneralControls/ErrorBox.hpp"

UIElement::UIElement() {
	m_sprite = nullptr;
	m_material = nullptr;
	m_is_loaded = false;
}

UIElement::~UIElement() {
	UserInterface::RemoveUIElement(this);

	W_SAFE_REMOVEREF(m_sprite);
}

void UIElement::SetFade(float fFade) {
	m_alpha = fFade;
	if (m_material)
		m_material->SetVariableFloat("alpha", m_alpha);
}

void UIElement::OnResize(UINT width, UINT height) {
	if (m_material)
		m_material->SetVariableVector2("spriteSize", WVector2((float)width, (float)height));
}

void UIElement::SetPosition(float x, float y) {
	if (m_sprite)
		m_sprite->SetPosition(WVector2(x, y));
}

void UIElement::SetSize(float sizeX, float sizeY) {
	if (m_sprite)
		m_sprite->SetSize(WVector2(sizeX, sizeY));
	if (m_material)
		m_material->SetVariableVector2("spriteSize", WVector2(sizeX, sizeY));
}

UIElement* UIElement::GetParent() const {
	return m_parent;
}

void UIElement::SetParent(UIElement* p) {
	m_parent = p;
}

UIElement* UIElement::GetChild(UINT index) const {
	if (index >= m_children.size())
		return nullptr;

	return m_children[index];
}

void UIElement::AddChild(UIElement* child) {
	m_children.push_back(child);
}

void UIElement::RemoveChild(UIElement* child) {
	for (UINT i = 0; i < m_children.size(); i++)
		if (m_children[i] == child)
			m_children.erase(m_children.begin() + i);
}

void UIElement::RemoveChild(UINT index) {
	if (index < m_children.size())
		m_children.erase(m_children.begin() + index);
}

UINT UIElement::GetNumChildren() const {
	return m_children.size();
}

std::unordered_map<UIElement*, bool> UserInterface::UI;
UIElement* UserInterface::focus = nullptr;

WError UserInterface::Init(Wasabi* app) {
	WError err = app->TextComponent->CreateTextFont(FONT_CALIBRI_16, "Calibri");
	if (!err) {
		MessageBoxA(nullptr, err.AsString().c_str(), APPNAME, MB_ICONERROR | MB_OK);
		return err;
	}

	return WError(W_SUCCEEDED);
}

UIElement* UserInterface::PrintError(Wasabi* const app, std::string error_message) {
	ErrorBox* errBox = new ErrorBox(error_message);
	MenuButton* errorButton = new ErrorButton("Ok");
	AddUIElement(errBox, nullptr);
	AddUIElement(errorButton, errBox);
	Load(app);
	SetFocus(errBox);
	return errBox;
}

void UserInterface::AddUIElement(UIElement* element, UIElement* m_parent) {
	element->SetParent(m_parent);
	if (m_parent)
		m_parent->AddChild(element);
	UI.insert(std::make_pair(element, true));

	if (UI.size() == 1)
		focus = element;
}

void UserInterface::RemoveUIElement(UIElement* element) {
	auto it = UI.find(element);
	if (it != UI.end()) {
		if (element == focus) {
			focus = nullptr;
			for (auto it : UI) {
				UIElement* curElem = it.first;
				if (curElem != element && curElem->OnFocus()) {
					focus = curElem;
					break;
				}
			}
		}

		UI.erase(it);
		for (auto child : element->m_children)
			RemoveUIElement(child);
		delete element;
	}
}

void UserInterface::Load(Wasabi* app) {
	for (auto it : UI) {
		UIElement* curElem = it.first;
		if (!curElem->m_is_loaded)
			curElem->Load(app);
		curElem->m_is_loaded = true;
	}
}

void UserInterface::Terminate() {
	while (UI.size())
		RemoveUIElement(UI.begin()->first);
	focus = nullptr;
}

void UserInterface::Update(float fDeltaTime) {
	//check for update disablers
	for (auto it : UI) {
		UIElement* curElem = it.first;
		if (curElem->OnDisableAllUpdates()) {
			if (!curElem->Update(fDeltaTime)) {
				RemoveUIElement(curElem);
			} else {
				//update children too
				for (auto child : curElem->m_children) {
					if (!child->Update(fDeltaTime)) {
						curElem->RemoveChild(child);
						RemoveUIElement(child);
					}
				}
			}
			return;
		}
	}

	std::vector<UIElement*> elementsToDelete;
	for (auto it : UI) {
		UIElement* curElem = it.first;
		if (!curElem->Update(fDeltaTime)) {
			elementsToDelete.push_back(curElem);
		}
	}

	for (auto it : elementsToDelete)
		RemoveUIElement(it);
}

void UserInterface::SetFade(float fFade) {
	for (auto it : UI)
		it.first->SetFade(fFade);
}

void UserInterface::OnResize(UINT width, UINT height) {
	for (auto it : UI)
		it.first->OnResize(width, height);
}

void UserInterface::SetFocus(UIElement* element) {
	focus = element;
}

UIElement* UserInterface::GetFocus() {
	return focus;
}

UIElement* UserInterface::GetElementAt(int mx, int my) {
	vector<UIElement*> possibilities;
	for (auto it : UI) {
		UIElement* curElem = it.first;
		float posX = curElem->GetPositionX();
		float posY = curElem->GetPositionY();
		float sX = curElem->GetSizeX();
		float sY = curElem->GetSizeY();
		if (mx > curElem->GetPositionX() && mx < curElem->GetPositionX() + curElem->GetSizeX() &&
			my > curElem->GetPositionY() && my < curElem->GetPositionY() + curElem->GetSizeY())
			possibilities.push_back(curElem);
	}
	if (possibilities.size()) {
		int lowestZ = possibilities[0]->GetPosZ();
		int index = 0;
		//return priority to the one without input support
		for (UINT i = 1; i < possibilities.size(); i++)
			if (lowestZ > possibilities[i]->GetPosZ() && possibilities[i]->GetPosZ() >= 0) {
				lowestZ = possibilities[i]->GetPosZ();
				index = i;
			}
		if (lowestZ >= 0)
			return possibilities[index];
	}

	for (auto it : UI) {
		if (it.first->GetSizeX() == 0 && it.first->GetSizeY() == 0)
			return it.first;
	}

	return nullptr;
}
