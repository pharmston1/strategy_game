
#ifndef SIMULATOR_H_INCLUDED
#define SIMULATOR_H_INCLUDED
#include <string>
#include <set>
#include <map>
//#include "Players.h"
#include "rules.h"
//#include "Actor.h"
//#include "Units.h"
//#include "Equipment.h"
#include "support.h"
#include "vmath.h"
#include "Paths.h"

using namespace Rules;
using namespace std;

class Equipment;
class Unit;
class Actor;
class Event;
class Player;

class Actor
{
	//friend class World;
public:
	Actor();
	~Actor();
	std::string getID();
	void setPlayer(Player *p);
	Player* getPlayer();
	void setType(RulesEntry *entry);
	string getType(){return defTraits->title;}

	double getDoubleTrait(string key);

	double getLineDoubleTrait(string key); //Parental line
	double getTotalDoubleTrait(string key); //All in network
	double getMaxDoubleTrait(string key); //Highest in network

	string getStringTrait(string key);
	vector<string> getStringArrayTrait(string key);
	vector<double> getDoubleArrayTrait(string key);

    //Modifies traits on a per-unit level
	void setTrait(string key, double value);
    void setTrait(string key, vector<double> value);
    void setTrait(string key, string value);
    void setTrait(string key, vector<string> value);

	bool attach_child( Actor*);
    bool attach_parent(Actor* newParent);
    bool isConnected(Actor* actor);
	set<Actor* >* getNetwork(set<Actor* >* items = NULL);
    Actor* getParent(){return parent;}

    void destroy(){destroyed = true;}
    bool isDestroyed() {return destroyed;}

	string serializeModTraits();
	set<string>* getAttached();
	Actor* get_owner();
protected:
    bool destroyed;
	Player* player;
	std::string id;
	static unsigned long currentID;

    Actor* parent;

	map<string, Actor* > *attached;
	RulesEntry *defTraits;  //Default (rules) traits
	RulesEntry *modTraits;  //Modified traits

    ///Generates an identifier for the actor
	virtual void genID(){
	stringstream str;
	str << "a" << currentID++;

	id = str.str();//rand() % 9999999 + 1000000;
    };



private:

};



class Unit : public Actor
{
	//friend class Update;
public:
	Unit();
	~Unit();
    void setPos(double x, double y);
    void setPos(Vector2<double> pos);

    Vector2<double> getPos();
	void pushWaypoint(Vector2<int> waypoint);
	void clearWaypoints();
	Vector2<int> popWaypoint();
	Vector2<int> getCurrentWaypoint();
	bool hasWaypoints();
	bool updateWaypoints();
	void pushTarget(Unit* target);
	Unit* getCurrentTarget();
	Unit* popTarget();
	void clearTargets();
    void setStance(string newStance)
    {stance = newStance;}

    bool onMap();
	//void move_unit(vector<int>);

    ///Marks this unit and all of its child equipment as destroyed
	void destroy()
	{
        destroyed = true;
        set<Actor*>* all = getNetwork(NULL);

        for(set<Actor*>::iterator iter = all->begin(); iter != all->end(); iter++)
            (*iter)->destroy();

        delete all;
	}

	string serialize();
private:

	Vector2<double> coord;	//[0]=x; [1]=y
	std::string stance;
	queue<Vector2<int> > nextcoord;
	queue<Unit*> targets;

protected:
    void genID()
    {
        stringstream str;
        str << "u" << currentID++;

        //Ensures unique IDs upon unit creation
        id = str.str();//rand() % 9999999 + 1000000;
    }
};

class Equipment : public Actor
{
    private:

    protected:
    void genID()
    {
        stringstream str;
        str << "e" << currentID++;

        //Ensures unique IDs upon unit creation
        id = str.str();//rand() % 9999999 + 1000000;
    }

    public:
		Equipment();
		string get_id();
		string serialize();

};


///Player class
class Player
{

private:
	string userID;
	int currbalance;
	string faction;
	int score;
	//vector< Actor*> inventory;

    Bitgrid* visibilityGrid;
    set<Actor*> ownedActors;
    set<Unit*> ownedUnits;
    set<Equipment*> ownedEquipment;
    set<Event*> events;
    set<Actor*> visibleActors;
	string* privBuffer;
	string* pubBuffer;

	set<Player*> allies;

public:

	float color[3];
	string nickname;
    ~Player();
	Player( string, int);
	string get_userid();
	int get_balance();
	bool has_enough_money( double);
	void purchase( double cost);
    void setFaction(string factionID);
    string getFaction();
    void update_visibility(int width, int height);
    void reset();
    void add_actor(Actor *actor);
    void add_event(Event *ev); ///Implement this
    bool isVisible(Unit*);
	bool isVisible(Vector2<int> v);
    string serialize( bool);
	void clear_buffers();
	bool isAlly(Player *other);
	void setAlly(Player *other, bool allied);
	void disown(Actor *actor);
	void claim(Actor *actor);
};

///Helper function - returns true if both players are allied
bool areAllies(Player *a, Player *b);

/*!
	\class Event
	\brief Event container
	\author Chris Livingston/Phil Harmston
*/

class Event
{
    private:
    map<string, Actor*> *affected;
    map<string, Actor*> *destroyed;
    map<string, Actor*> *damaged;
    Player *instigator;
	string id;
	string outcome;
    Vector2< int> origin;
	set< Player*> involved;
	string serialize_map(map<string, Actor*> *actor_map){
		stringstream str;
		str << "[ ";
		for(map<string, Actor*>::iterator it = actor_map->begin(); it != actor_map->end(); it++)
			str << "\"" + it->first + "\",";
		str.seekp((long)str.tellp() - 1);
		str << "],";
		return str.str();
	}

    string buffer;
    public:
	string getID()
	{
		return id;
	}

    ///Returns this event serialized as JSON
    string serialize();

    ///Returns true if the player is involved
    bool isInvolved(Player*);

    ///Setter functions
    void addAffected(Actor*);
    void addDestroyed(Actor*);
    void addDamaged(Actor*);
    void setInstigator(Player*);
	void setorigin( int x, int y);
	void setoutcome( string);
	Vector2< int> get_origin();

	static unsigned long curreventID;

	void genID()
	{
		stringstream str;
		str << 'ev' << curreventID++;

		id = str.str();
	}


    Event();
    ~Event();

};


#endif // SIMULATOR_H_INCLUDED
