#include "WasabiGame/UI/UI.hpp"
#include "WasabiGame/Main.hpp"
#include "WasabiGame/UI/GeneralControls/ErrorBox.hpp"


WasabiGame::UIElement::UIElement(std::shared_ptr<WasabiGame::UserInterface> ui) {
	m_UI = ui;
	m_parent = nullptr;
	m_sprite = nullptr;
	m_material = nullptr;
	m_is_loaded = false;
	m_alpha = 1.0f;
}

WasabiGame::UIElement::~UIElement() {
	W_SAFE_REMOVEREF(m_sprite);
}

void WasabiGame::UIElement::SetFade(float fFade) {
	m_alpha = fFade;
	if (m_material)
		m_material->SetVariable("alpha", m_alpha);
}

void WasabiGame::UIElement::OnResize(uint32_t width, uint32_t height) {
	if (m_material)
		m_material->SetVariable("spriteSize", WVector2((float)width, (float)height));
}

void WasabiGame::UIElement::SetPosition(float x, float y) {
	if (m_sprite)
		m_sprite->SetPosition(WVector2(x, y));
}

void WasabiGame::UIElement::SetSize(float sizeX, float sizeY) {
	if (m_sprite)
		m_sprite->SetSize(WVector2(sizeX, sizeY));
	if (m_material)
		m_material->SetVariable("spriteSize", WVector2(sizeX, sizeY));
}

std::shared_ptr<WasabiGame::UIElement> WasabiGame::UIElement::GetParent() const {
	return m_parent;
}

void WasabiGame::UIElement::SetParent(std::shared_ptr<UIElement> p) {
	m_parent = p;
}

std::shared_ptr<WasabiGame::UIElement> WasabiGame::UIElement::GetChild(uint32_t index) const {
	if (index >= m_children.size())
		return nullptr;

	return m_children[index];
}

void WasabiGame::UIElement::AddChild(std::shared_ptr<UIElement> child) {
	m_children.push_back(child);
}

void WasabiGame::UIElement::RemoveChild(std::shared_ptr<UIElement> child) {
	for (uint32_t i = 0; i < m_children.size(); i++)
		if (m_children[i] == child)
			m_children.erase(m_children.begin() + i);
}

void WasabiGame::UIElement::RemoveChild(uint32_t index) {
	if (index < m_children.size())
		m_children.erase(m_children.begin() + index);
}

uint32_t WasabiGame::UIElement::GetNumChildren() const {
	return m_children.size();
}


WasabiGame::UserInterface::UserInterface(std::shared_ptr<WasabiBaseGame> app) {
	m_app = app;
	focus = nullptr;
}

WasabiGame::UserInterface::~UserInterface() {
	Cleanup();
}

std::weak_ptr<WasabiGame::WasabiBaseGame> WasabiGame::UserInterface::GetApp() {
	return m_app;
}

WError WasabiGame::UserInterface::Init() {
	std::shared_ptr<WasabiBaseGame> app = m_app.lock();
	WError err = app->TextComponent->CreateTextFont(FONT_CALIBRI_16, "Calibri");
	if (!err) {
		MessageBoxA(nullptr, err.AsString().c_str(), APPNAME, MB_ICONERROR | MB_OK);
		return err;
	}

	return WError(W_SUCCEEDED);
}

std::shared_ptr<WasabiGame::UIElement> WasabiGame::UserInterface::PrintError(std::string error_message) {
	std::shared_ptr<UserInterface> UI = m_app.lock()->UI;
	std::shared_ptr<ErrorBox> errBox = std::make_shared<ErrorBox>(UI, error_message);
	std::shared_ptr<MenuButton> errorButton = std::make_shared<ErrorButton>(UI, "Ok");
	AddUIElement(std::static_pointer_cast<UIElement>(errBox), nullptr);
	AddUIElement(std::static_pointer_cast<UIElement>(errorButton), std::static_pointer_cast<UIElement>(errBox));
	Load();
	SetFocus(std::static_pointer_cast<UIElement>(errorButton));
	return std::static_pointer_cast<UIElement>(errorButton);
}

void WasabiGame::UserInterface::AddUIElement(std::shared_ptr<UIElement> element, std::shared_ptr<UIElement> m_parent) {
	element->SetParent(m_parent);
	if (m_parent)
		m_parent->AddChild(element);
	UI.insert(std::make_pair(element, true));

	if (UI.size() == 1)
		focus = element;
}

void WasabiGame::UserInterface::RemoveUIElement(std::shared_ptr<UIElement> element) {
	auto it = UI.find(element);
	if (it != UI.end()) {
		if (element == focus) {
			focus = nullptr;
			for (auto it : UI) {
				std::shared_ptr<UIElement> curElem = it.first;
				if (curElem != element && curElem->OnFocus()) {
					focus = curElem;
					break;
				}
			}
		}

		UI.erase(it);
		for (auto child : element->m_children)
			RemoveUIElement(child);
	}
}

void WasabiGame::UserInterface::Load() {
	for (auto it : UI) {
		std::shared_ptr<UIElement> curElem = it.first;
		if (!curElem->m_is_loaded)
			curElem->Load();
		curElem->m_is_loaded = true;
	}
}

void WasabiGame::UserInterface::Cleanup() {
	while (UI.size())
		RemoveUIElement(UI.begin()->first);
	focus = nullptr;
}

void WasabiGame::UserInterface::Update(float fDeltaTime) {
	//check for update disablers
	for (auto it : UI) {
		std::shared_ptr<UIElement> curElem = it.first;
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

	std::vector<std::shared_ptr<UIElement>> elementsToDelete;
	for (auto it : UI) {
		std::shared_ptr<UIElement> curElem = it.first;
		if (!curElem->Update(fDeltaTime)) {
			elementsToDelete.push_back(curElem);
		}
	}

	for (auto it : elementsToDelete)
		RemoveUIElement(it);
}

void WasabiGame::UserInterface::SetFade(float fFade) {
	for (auto it : UI)
		it.first->SetFade(fFade);
}

void WasabiGame::UserInterface::OnResize(uint32_t width, uint32_t height) {
	for (auto it : UI)
		it.first->OnResize(width, height);
}

void WasabiGame::UserInterface::SetFocus(std::shared_ptr<UIElement> element) {
	focus = element;
}

std::shared_ptr<WasabiGame::UIElement> WasabiGame::UserInterface::GetFocus() {
	return focus;
}

std::shared_ptr<WasabiGame::UIElement> WasabiGame::UserInterface::GetElementAt(int mx, int my) {
	vector<std::shared_ptr<UIElement>> possibilities;
	for (auto it : UI) {
		std::shared_ptr<UIElement> curElem = it.first;
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
		for (uint32_t i = 1; i < possibilities.size(); i++)
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
