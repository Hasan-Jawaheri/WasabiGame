#pragma once

#include "..//Common.hpp"
#include "MapLoader.hpp"

class Map {
public:

	struct MAP_DESCRIPTION {
		std::string mapFilename;
	};

	virtual MAP_DESCRIPTION GetDescription() const = 0;
	virtual void OnLoaded() {}
	virtual void Update(float fTimeDelta) {}
	virtual void Cleanup() {}
};
