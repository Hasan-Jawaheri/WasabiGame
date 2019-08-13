#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Maps/Test.hpp"

template<typename T> Map* MapGen() { return new T(); };

void SetupRTBMaps(MapLoader* loader) {
	loader->ResetMaps();
	loader->RegisterMap(MAP_TEST, MapGen<TestMap>);
}
