#include <vector>
#include <hash_map>
#include <fstream>
#include "Tile.h"
#include "Jzon.h"
using namespace Jzon;

#define NULL 0

class GameMap{
public:
	/*	Constructors.	*/
	GameMap()
		{ defaultTile = new Tile(); }
	GameMap(string filename)
		{ defaultTile = new Tile(); constructMap(filename); }

	/*	Opens map JSON file and constructs the map.	*/
	void constructMap(string filename){
		Object map;
		FileReader::ReadFile(filename, map);

		title		= map.Get("Title").ToString();
		description	= map.Get("Description").ToString();
		mapWidth	= map.Get("Dimensions").Get(0).ToInt();
		mapHeight	= map.Get("Dimensions").Get(1).ToInt();
		rules		= map.Get("DefaultRules").ToString();

		tiles = vector< vector<Tile*> >(mapWidth, vector<Tile*>(mapWidth));

		string type = map.Get("DefaultTile").ToString();
		defaultTile->setTerrainType(type);
		tileSet[type] = defaultTile;

		Array rect = map.Get("Rectangles").AsArray();
		int x, y, w, h, i, j;
		for(Array::iterator it = rect.begin(); it != rect.end(); ++it){
			x = (*it).Get("Bounds").Get(0).ToInt();
			y = (*it).Get("Bounds").Get(1).ToInt();
			w = (*it).Get("Bounds").Get(2).ToInt();
			h = (*it).Get("Bounds").Get(3).ToInt();

			type = (*it).Get("Type").ToString();
			if(tileSet.find(type) == tileSet.end())
				tileSet[type] = new Tile(type);

			for(i = 0; i < w; i++)
				for(j = 0; j < h; j++){
					if(x + i >= mapWidth || y + j >= mapHeight)
						continue;
					tiles[x + i][y + j] = tileSet[type];
				}
		}
	}
	/*	Writes current map to TXT file.	*/
	void drawMap(){
		ofstream file;
		file.open("psuedo_map.txt");
		for(int i = mapHeight - 1; i > -1; i--)
			for(int j = 0; j < mapWidth; j++){
				file << (tiles[j][i] != NULL ? tiles[j][i]->getTerrainType() : defaultTile->getTerrainType())[0];
				if(j == mapHeight - 1)
					file << '\n';
			}
		file.close();
	}
	/*	Makes all Tile pointers NULL.	*/
	void clearMap(){
		for(int i = 0; i < mapWidth; i++)
			for(int j = 0; j < mapHeight; j++)
				tiles[i][j] = NULL;
	}
	int getWidth()
		{ return mapWidth; }
	int getHeight()
		{ return mapHeight; }
	string getDefaultTileType()
		{ return defaultTile->getTerrainType(); }
	vector< vector<Tile*> > getTiles()
		{ return tiles; }
	hash_map<string, Tile*> getTileSet()
		{ return tileSet; }
private:
	string title, description, rules;
	int mapWidth, mapHeight;
	Tile* defaultTile;
	vector< vector<Tile*> > tiles;
	hash_map<string, Tile*> tileSet;
	hash_map<string, char> ch;
};