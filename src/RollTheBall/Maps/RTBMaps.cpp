#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Maps/Test.hpp"


template<typename T> std::shared_ptr<WasabiGame::Map> MapGen() { return std::make_shared<T>(); };

void RollTheBall::SetupRTBMaps(std::shared_ptr<WasabiGame::MapLoader> loader) {
	loader->ResetMaps();
	loader->RegisterMap(MAP_TEST, MapGen<TestMap>);
}
