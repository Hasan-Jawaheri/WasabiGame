#pragma once

#include "Common.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"

#include <atomic>


namespace WasabiGame {

	class WasabiBaseGame;
	class ResourceManager;
	class UnitsManager;
	class AI;
	struct LOADED_MODEL;

	class Unit {
		friend class UnitsManager;

		struct LoadInfo {
			std::string modelName;
			WVector3 spawnPos;
		} m_loadInfo;
		std::atomic<bool> m_canLoad;

		LOADED_MODEL* m_model;
		std::shared_ptr<AI> m_AI;
		uint m_id;
		uint m_type;

	protected:
		std::weak_ptr<WasabiBaseGame> m_app;
		std::shared_ptr<ResourceManager> m_resourceManager;
		std::shared_ptr<UnitsManager> m_unitsManager;

		Unit(std::shared_ptr<WasabiBaseGame> app, std::shared_ptr<ResourceManager> resourceManager, std::shared_ptr<UnitsManager> unitsManager);
		virtual ~Unit();

		virtual void Update(float fDeltaTime);
		virtual void OnLoaded();

	public:

		uint GetId() const;
		uint GetType() const;
		std::weak_ptr<WasabiBaseGame> GetApp() const;

		WOrientation* O() const;
		WRigidBody* RB() const;

		template<typename T> void SetAI() { m_AI = std::make_shared<T>(this); }
		std::shared_ptr<AI> GetAI() const;
	};

};
