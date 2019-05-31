#pragma once

#include "..//Common.hpp"

class UIElement {
	friend class UserInterface;

	UIElement* m_parent;
	std::vector<UIElement*> m_children;
	bool m_is_loaded;

protected:
	WSprite* m_sprite;
	WMaterial* m_material;
	float m_alpha;
	std::string m_input_buffer;

public:
	UIElement();
	virtual ~UIElement();

	UIElement* GetParent() const;
	void SetParent(UIElement* p);
	UIElement* GetChild(UINT index) const;
	void AddChild(UIElement* child);
	void RemoveChild(UIElement* child);
	void RemoveChild(UINT index);
	UINT GetNumChildren() const;

	virtual void Load(Wasabi* const app) {}
	virtual void SetFade(float fFade);
	virtual void OnResize(UINT width, UINT height);
	virtual bool Update(float fDeltaTime) { return true; };

	virtual bool OnInput() { return true; }
	virtual void OnKeydown(short key) {}
	virtual void OnKeyup(short key) {}
	virtual void OnMouseMove(int mx, int my) {}
	virtual void OnMouseButton(int mx, int my, bool bDown) {}
	virtual void OnMouseButton2(int mx, int my, bool bDown) {}
	virtual bool OnTab() { return true; }
	virtual bool OnEnter() { return true; }
	virtual bool OnEscape() { return true; }
	virtual bool IsPassword() { return false; }
	virtual bool OnFocus() { return true; }
	virtual bool OnLoseFocus() { return true; }
	virtual bool OnDisableAllUpdates() { return false; }
	std::string& GetInputBuffer() { return m_input_buffer; }

	virtual int GetPosZ() const { return 1; }
	virtual void SetPosition(float x, float y);
	virtual float GetPositionX() const { if (m_sprite) return m_sprite->GetPosition().x; return 0; }
	virtual float GetPositionY() const { if (m_sprite) return m_sprite->GetPosition().y; return 0; }
	virtual void SetSize(float sizeX, float sizeY);
	virtual float GetSizeX() const { if (m_sprite) return m_sprite->GetSize().x; return 0; }
	virtual float GetSizeY() const { if (m_sprite) return m_sprite->GetSize().y; return 0; }
};

class UserInterface {
	static std::unordered_map<UIElement*, bool> UI;
	static UIElement* focus;

public:

	static WError Init(Wasabi* app);
	static void Update(float fDeltaTime);
	static void Terminate();
	static void Load(Wasabi* app);
	static void SetFade(float fFade);
	static void OnResize(UINT width, UINT height);
	static UIElement* PrintError(Wasabi* const app, std::string errorMessage);
	static void AddUIElement(UIElement* element, UIElement* parent);
	static void RemoveUIElement(UIElement* element);
	static void SetFocus(UIElement* element);
	static UIElement* GetFocus();
	static UIElement* GetElementAt(int mx, int my);
};

template<typename PSType>
WMaterial* CreateSpriteMaterial(Wasabi* app, WSprite*& sprite) {
	PSType* ps = new PSType(app);
	ps->Load();
	sprite->ClearEffects();
	sprite->AddEffect(app->SpriteManager->CreateSpriteEffect(app->Renderer->GetRenderTarget(), ps));
	return sprite->GetMaterial();
}
