#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <set>
#include <sstream>
#include "simulator.h"
#include "GameMap.h"
#include "orders.h"
#include <cmath>
#include <algorithm>
#include <iostream>

/*!
	\class World
	\brief World container
	\author Chris Livingston/Jonathon Meza/Phil Harmston
*/
class World
{
private:
	map<string, Player*> players;
	map<string, Actor*> actors;
	map<string, Unit*> units;
	map<string, Event*> events;
	map<string, Equipment*> equipments;
	map< int, Unit*> unitLocations;

	set<string> factions;
	map<int, string> tickLogs;

	int nextTick;
	int currentTick;
	double worldTime;

	void update(double deltaTime);
	void updateUnits(double deltaTime);
	void updateEquipment(double deltaTime);
	void updatePlayers(double deltaTime);
	void assignEvents();

	int getCoordIndex( double x, double y);

	Properties *settings;

    set<Actor*> allocatedActors; //memory management
    map<string, set<Actor*> > typeActors;   //All actors sorted by type
	string infoBuffer; //Serialization buffer
	void updateInfoBuffer(time_t complete, time_t next);

	string serializeInfo();
	string serializeEvents();
	string serializePlayers(bool privacy);
	string serializeUnits();
	string serializeEquipment();
	string serializeDiplomacy(set<Player*> pl);
public:
	RulesContainer rules;
	GameMap gameMap;

	map<string, Player*> getPlayers() {return players;}
    Player* addPlayer(string userID, string factionID);
    bool addOrders(Orders* orders);
    void loadOrders(OrdersLoader* loader);
    Unit* createUnit(string unitType, Player* owner);
    Equipment* createEquipment(string equipType, Player *owner);
    void removeActor(Actor *actor);

	int advanceTick(time_t complete, time_t next);

    Unit* getUnitAt(Vector2<double> pos);
	Unit* getUnitAt( double x, double y);
	bool moveUnit( Unit* mUnit, double x, double y);

    World(Properties &properties);
    ~World();
    string dumpLog(int tick);

    void refreshPlayerVisibility();
	string getTileTypeAt(double x, double y);
	string masterSerialize(Player* currplayer);
	bool moveUnit(Unit *mUnit, Vector2<double> newPos);
    Vector2<int> getNearestOpen(Vector2<double> point);
    void doCombat(Unit *attacker, Unit *defender);

	void addEvent(Event* eve);
    Unit* getUnitTarget(Unit *unit);

	void log();
    static stringstream currentLog;
    set<Actor*> findActors(Player *owner, set<Actor*> exclude, string type);
	bool isVisible(Event* ev, Player* pl);

};


#endif
