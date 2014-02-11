/*
 *	Tile.h
 */
#include <string>
using namespace std;

class Tile{
public:
	Tile(){}
	Tile(string terrainType)
		{ this->terrainType = terrainType; }
	Tile(Tile* tile)
		{ this->terrainType = tile->getTerrainType(); }

	string getTerrainType()
		{ return terrainType; }
	void setTerrainType(string terrainType)
		{ this->terrainType = terrainType; }
private:
	string terrainType;
};