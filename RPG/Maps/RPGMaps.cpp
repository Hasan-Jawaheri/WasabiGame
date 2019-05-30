#include "RPGMaps.hpp"
#include "../../WasabiGame/Maps/MapLoader.hpp"

#include "ICC.hpp"

template<typename T> Map* MapGen() { return new T(); };

void SetupRPGMaps() {
	MapLoader::ResetMaps();
	MapLoader::RegisterMap(MAP_ICC, MapGen<ICCMap>);
}
