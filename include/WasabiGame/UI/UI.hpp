#pragma once

#include "Common.hpp"


namespace WasabiGame {

	class WasabiBaseGame;
	class UserInterface;
		
	class UIElement {
		friend class UserInterface;

		std::shared_ptr<UIElement> m_parent;
		std::vector<std::shared_ptr<UIElement>> m_children;
		bool m_is_loaded;

	protected:
		std::shared_ptr<UserInterface> m_UI;

		WSprite* m_sprite;
		WMaterial* m_material;
		float m_alpha;
		std::string m_input_buffer;

	public:
		UIElement(std::shared_ptr<UserInterface> ui);
		virtual ~UIElement();

		std::shared_ptr<UIElement> GetParent() const;
		void SetParent(std::shared_ptr<UIElement> p);
		std::shared_ptr<UIElement> GetChild(uint32_t index) const;
		void AddChild(std::shared_ptr<UIElement> child);
		void RemoveChild(std::shared_ptr<UIElement> child);
		void RemoveChild(uint32_t index);
		uint32_t GetNumChildren() const;

		virtual void Load() {}
		virtual void SetFade(float fFade);
		virtual void OnResize(uint32_t width, uint32_t height);
		virtual bool Update(float fDeltaTime) { return true; };

		virtual bool OnInput() { return true; }
		virtual void OnKeydown(uint32_t key) {}
		virtual void OnKeyup(uint32_t key) {}
		virtual void OnMouseMove(double mx, double my) {}
		virtual void OnMouseButton(double mx, double my, bool bDown) {}
		virtual void OnMouseButton2(double mx, double my, bool bDown) {}
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
		std::weak_ptr<WasabiBaseGame> m_app;

		std::unordered_map<std::shared_ptr<UIElement>, bool> UI;
		std::shared_ptr<UIElement> focus;

	public:
		UserInterface(std::shared_ptr<WasabiBaseGame> app);
		~UserInterface();

		std::weak_ptr<WasabiBaseGame> GetApp();

		WError Init();
		void Update(float fDeltaTime);
		void Cleanup();
		void Load();
		void SetFade(float fFade);
		void OnResize(uint32_t width, uint32_t height);
		std::shared_ptr<UIElement> PrintError(std::string errorMessage);
		void AddUIElement(std::shared_ptr<UIElement> element, std::shared_ptr<UIElement> parent);
		void RemoveUIElement(std::shared_ptr<UIElement> element);
		void SetFocus(std::shared_ptr<UIElement> element);
		std::shared_ptr<UIElement> GetFocus();
		std::shared_ptr<UIElement> GetElementAt(double mx, double my);
	};

	template<typename PSType>
	WMaterial* CreateSpriteMaterial(std::shared_ptr<WasabiBaseGame> app, WSprite*& sprite) {
		PSType* ps = new PSType(app.get());
		ps->Load();
		sprite->ClearEffects();

		WEffect* fx = app->SpriteManager->CreateSpriteEffect(app->Renderer->GetRenderTarget(), ps);
		sprite->AddEffect(fx);
		return sprite->GetMaterial(fx);
	}

};
