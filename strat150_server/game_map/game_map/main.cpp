#include "GameMap.h"

int main(){
	GameMap map;
	map.constructMap("./map.json");
	map.drawMap();
	return 0;
}