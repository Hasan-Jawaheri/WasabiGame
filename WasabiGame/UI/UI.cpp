#include "UI.hpp"
#include "GeneralControls//ErrorBox.hpp"
//#include "GameControls//Action Bar.hpp"  @TODO: CHANGE HERE

UIElement::UIElement() {
	m_sprite = nullptr;
	m_material = nullptr;
	m_is_loaded = false;
}

UIElement::~UIElement() {
	W_SAFE_REMOVEREF(m_sprite);

	for (UINT i = 0; i < m_children.size(); i++)
		delete m_children[i];
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

vector<UIElement*> UserInterface::UI;
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
	UI.push_back(element);

	if (UI.size() == 1)
		focus = element;
}

void UserInterface::RemoveUIElement(UIElement* element) {
	for (UINT i = 0; i < UI.size(); i++) {
		if (UI[i] == element) {
			if (focus == element) {
				bool bFoundFocus = false;
				for (UINT n = 0; n < UI.size(); n++)
					if (UI[n] != focus && UI[n]->OnFocus()) {
						focus = UI[n];
						bFoundFocus = true;
						break;
					}
				if (!bFoundFocus)
					focus = nullptr;
			}
			UI.erase(UI.begin() + i);
			for (UINT n = 0; n < element->m_children.size(); n++)
				RemoveUIElement(element->m_children[n]);
		}
	}
}

void UserInterface::Load(Wasabi* app) {
	for (UINT i = 0; i < UI.size(); i++) {
		if (!UI[i]->m_is_loaded)
			UI[i]->Load(app);
		UI[i]->m_is_loaded = true;
	}
}

void UserInterface::Terminate() {
	for (UINT i = 0; i < UI.size(); i++) {
		while (UI[i]->GetNumChildren()) UI[i]->RemoveChild((UINT)0); //remove all children for safe deleting of the elements
		delete UI[i];
	}
	UI.clear();
	focus = nullptr;
}

void UserInterface::Update(float fDeltaTime) {
	//check for update disablers
	for (unsigned int i = 0; i < UI.size(); i++) {
		if (UI[i]->OnDisableAllUpdates()) {
			if (!UI[i]->Update(fDeltaTime)) {
				UIElement* element = UI[i];
				RemoveUIElement(element);
				delete element;
				i--;
			} else {
				//update children too
				for (UINT n = 0; n < UI[i]->GetNumChildren(); n++) {
					if (!UI[i]->GetChild(n)->Update(fDeltaTime)) {
						UIElement* element = UI[i]->GetChild(n);
						UI[i]->RemoveChild(n);
						RemoveUIElement(element);
						delete element;
						n--;
					}
				}
			}
			return;
		}
	}

	for (UINT i = 0; i < UI.size(); i++) {
		if (!UI[i]->Update(fDeltaTime)) {
			UIElement* element = UI[i];
			RemoveUIElement(element);
			delete element;
			i--;
		}
	}
}

void UserInterface::SetFade(float fFade) {
	for (UINT i = 0; i < UI.size(); i++)
		UI[i]->SetFade(fFade);
}

void UserInterface::OnResize(UINT width, UINT height) {
	for (UINT i = 0; i < UI.size(); i++)
		UI[i]->OnResize(width, height);
}

void UserInterface::SetFocus(UIElement* element) {
	focus = element;
}

UIElement* UserInterface::GetFocus() {
	return focus;
}

UIElement* UserInterface::GetElementAt(int mx, int my) {
	vector<UIElement*> possibilities;
	for (UINT i = 0; i < UI.size(); i++) {
		float posX = UI[i]->GetPositionX();
		float posY = UI[i]->GetPositionY();
		float sX = UI[i]->GetSizeX();
		float sY = UI[i]->GetSizeY();
		if (mx > UI[i]->GetPositionX() && mx < UI[i]->GetPositionX() + UI[i]->GetSizeX() &&
			my > UI[i]->GetPositionY() && my < UI[i]->GetPositionY() + UI[i]->GetSizeY())
			possibilities.push_back(UI[i]);
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

	for (UINT i = 0; i < UI.size(); i++) {
		if (UI[i]->GetSizeX() == 0 && UI[i]->GetSizeY() == 0)
			return UI[i];
	}

	return nullptr;
}
