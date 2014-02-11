#include <vector>
#include <map>
#include <fstream>
#include "Tile.h"
#include "Jzon.h"
#include "support.h"
using namespace Jzon;
//using namespace std;

#ifndef NULL
#define NULL 0
#endif


class GameMap{
private:
	string title, description, rules;
	int mapWidth, mapHeight;
	Tile* defaultTile;
	vector< vector<Tile* > > tiles;
	map<string, Tile* > tileSet;
	map<string, char> ch;

public:
	/*	Constructors.	*/
	GameMap()
		{ defaultTile = new Tile(); }
	GameMap(string filename)
		{ defaultTile = new Tile(); constructMap(filename); }

    ~GameMap()
    {
        while(tileSet.size() > 0)
        {
            delete tileSet.begin()->second;
            tileSet.erase(tileSet.begin());
        }
    }

    string getTitle()
    { return title; }

    string getDescription()
    { return description; }

    string getDefaultRules()
    { return rules; }

    bool inBounds(int x, int y)
    {return !(x < 0 || y < 0 || x >= mapWidth || y >= mapHeight);}

	/*	Opens map JSON file and constructs the map.	*/
	bool constructMap(string filename){
		Object gameMap;
		try
		{
			if(!FileReader::ReadFile(filename, gameMap))
				return false;
		}
		catch(...)
		{
			return false;
		}
		title		= gameMap.Get("Title").ToString();
		description	= gameMap.Get("Description").ToString();
		mapWidth	= gameMap.Get("Dimensions").Get(0).ToInt();
		mapHeight	= gameMap.Get("Dimensions").Get(1).ToInt();
		rules		= gameMap.Get("DefaultRules").ToString();

		tiles = vector< vector<Tile*> >(mapWidth, vector<Tile*>(mapWidth));

		string type = gameMap.Get("DefaultTile").ToString();
		defaultTile->setTerrainType(type);
		tileSet[type] = defaultTile;

		Array rect = gameMap.Get("Rectangles").AsArray();
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

		return true;
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
	vector < vector<Tile* > > getTiles()
		{ return tiles; }
	map <string, Tile* > getTileSet()
		{ return tileSet; }

};
