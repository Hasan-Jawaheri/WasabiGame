#pragma once

#include "WasabiGame/Maps/Map.hpp"

class TestMap : public Map {
public:
	virtual MAP_DESCRIPTION GetDescription() const {
		MAP_DESCRIPTION desc = {};
		desc.mapFilename = "test";
		return desc;
	}

	virtual void Update(float fTimeDelta) {
	}
};
