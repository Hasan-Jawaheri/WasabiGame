#pragma once

#include "../../WasabiGame/Maps/Map.hpp"

class ICCMap : public Map {
public:
	virtual MAP_DESCRIPTION GetDescription() const {
		MAP_DESCRIPTION desc = {};
		desc.mapFilename = "icc";
		return desc;
	}

	virtual void Update(float fTimeDelta) {
	}
};
