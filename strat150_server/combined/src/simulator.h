
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

using namespace Rules;
using namespace std;

class Equipment;
class Unit;
class Actor;
class Event;
class Player;

class Actor
{
//	friend class Update;
public:
	Actor();
	~Actor();
	std::string getID();
	void setPlayer(Player *p);
	Player* getPlayer();
	void setType(RulesEntry *entry);

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

    Actor* getParent(){return parent;}
	//void move_unit(vector<int>);

protected:
	Player* player;
	std::string id;
	static unsigned long currentID;

    Actor* parent;
    set<Actor* >* getNetwork(set<Actor* >* items = NULL);
	map<string, Actor* > *attached;
	RulesEntry *defTraits;  //Default (rules) traits
	RulesEntry *modTraits;  //Modified traits

    ///Generates an identifier for the actor
	virtual void genID(){
	stringstream str;
	str << "a" << currentID++;

	id = str.str();//rand() % 9999999 + 1000000;
    }

private:

};

class Unit : public Actor
{
	friend class Update;
public:
	Unit();
	~Unit();
    void setPos(double x, double y);
    void setPos(Vector2<double> pos);

    Vector2<double> getPos();

    bool onMap();
	//void move_unit(vector<int>);

private:

	Vector2<double> coord;	//[0]=x; [1]=y
	std::string stance;
	queue< Vector2<int> > nextcoord;
	vector<string> targets;

protected:
    void genID()
    {
        stringstream str;
        str << "u" << currentID++;

        //Ensures unique IDs upon unit creation
        id = str.str();//rand() % 9999999 + 1000000;
    }
};

/*!
    \fn void Unit::setPos(double x, double y)
    \brief Sets unit coordinates.
    \param x X-coordinate
    \param y Y-coordinate
*/
void Unit::setPos(double x, double y)
{
	coord.x = x;
    coord.y = y;
}

void Unit::setPos(Vector2<double> pos)
{
	coord = pos;
}

///Returns true if the unit is on the map
bool Unit::onMap()
{
    return (coord.x >= 0 && coord.y >= 0);
}

///Gets the unit's current coordinates.


Vector2<double> Unit::getPos()
{
    return coord;
}

/*!
	\fn Unit::Unit()
	\brief constructor and assigns it a random number for id
*/
Unit::Unit()	// constructor
{
    stringstream str;
    str << "u" << currentID++;

    //Ensures unique IDs upon unit creation
	id = str.str();//rand() % 9999999 + 1000000;

	coord.x = -1;
	coord.y = -1;
}

Unit::~Unit()
{
    delete modTraits;
}

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
		Unit* get_owner_unit();

};


Equipment::Equipment()	// constructor
{
    stringstream str;
    str << "e" << currentID++;

    //Ensures unique IDs upon unit creation
	id = str.str();//rand() % 9999999 + 1000000;
}



unsigned long Actor::currentID = 0;

double Actor::getDoubleTrait(string key)
{
    if(modTraits->getDoubleKeys().count(key) != 0)
        return modTraits->getDouble(key); //Return modified trait first - if available

    if(defTraits != NULL && defTraits->getDoubleKeys().count(key) != 0)
        return defTraits->getDouble(key); //Return default trait next - if available

    return 0;   //Not found, default value
}

/*!
	\fn Actor::Actor()
*/
Actor::Actor()	// constructor
{
//  genID();
    parent = NULL;
	player = NULL;
	defTraits = NULL;
	modTraits = new RulesEntry();
	attached = new map<string, Actor*>();
}

Actor::~Actor()
{
    delete modTraits;
    delete attached;
}

/*!
    \fn void Actor::setType(RulesEntry *entry)
    \param entry Pointer to relevant RulesEntry.
*/
void Actor::setType(RulesEntry *entry)
{
    defTraits = entry;
}

/*!
    \fn void Actor::setPlayer(Player *p)
    \param p Pointer to owner player class.
 */
void Actor::setPlayer(Player *p)
{
    player = p;

    //Change ownership of all children
    map<string, Actor* >::iterator iter = attached->begin();
    while(iter != attached->end())
    {
        Actor *a = (*iter).second;

        //Check to avoid potential circular recursion
        if(a->getPlayer() != p)
            a->setPlayer(p);
    }
}

Player* Actor::getPlayer()
{
    return player;
}

/*!
	\fn unsigned long Actor::give_id()
	\brief returns the id
	\return unsigned long for the unit id
*/
std::string Actor::getID()	// gives the unit id as unsigned to the calling
{
	return id;
}



/*!
    \fn
*/
bool Actor::isConnected(Actor* actor)
{
    set<Actor* > *allAttached = getNetwork(NULL);

    if(allAttached->count(actor) == 0)
    {
        delete allAttached;
        return false;

    }

    delete allAttached;
    return true;
}

///Recursively builds a set of all items in this set.
set<Actor* >* Actor::getNetwork(set<Actor* >* items)
{
    if(items == NULL)
        items = new set<Actor* >();

    items->insert(this);

    if(parent != NULL && items->count(parent) == 0)
        parent->getNetwork(items);

    map<string, Actor* >::iterator iter = attached->begin();
    while(iter != attached->end())
    {
        Actor* child = (*iter).second;

        if(items->count(child) == 0)
            child->getNetwork(items);

        iter++;
    }

    return items;
}


///Returns the sum total of all attached items with matching key
double Actor::getTotalDoubleTrait(string key)
{
    set<Actor*> *net = this->getNetwork(NULL);

    double sum = 0;

    set<Actor*>::iterator iter = net->begin();

    while(iter != net->end())
    {
        sum += (*iter)->getDoubleTrait(key);
        iter++;
    }


    delete net;
    return sum;
}

///Returns the highest value of all attached items with matching key
double Actor::getMaxDoubleTrait(string key)
{
    set<Actor*> *net = this->getNetwork(NULL);

    double maxD = 0;

    set<Actor*>::iterator iter = net->begin();

    while(iter != net->end())
    {
        double v = (*iter)->getDoubleTrait(key);

        if(v > maxD)
            maxD = v;
        iter++;
    }


    delete net;
    return maxD;
}

///Returns the parent-line sum of attached items with matching key
double Actor::getLineDoubleTrait(string key)
{
    Actor *p = parent;

    double sum = 0;

    while(p != NULL)
    {
        sum += p->getDoubleTrait(key);
        p = p->parent;
    }

    return sum;
}


/*!
	\fn void Actor::attach_child( Actor* child)
	\brief attaches equipment to units or to other equipment
	\return True on success, false on failure
*/
bool Actor::attach_child( Actor* child)
{
    if(child == NULL)
        return false;

    child->parent = this;
	attached->insert( pair< string, Actor*>( child->getID(), child));

	return true;
}

/*!
    \fn void Actor::attach_parent(Actor* newParent)
    \brief Attaches a parent actor to this actor.
    \return True on success, false on failure.
*/
bool Actor::attach_parent(Actor* newParent)
{
    if(newParent == NULL)
        return false;

    parent->attach_child(this);

    return true;
}

class Player
{

private:
	string userID;
	int currbalance;
	string faction;
	//vector< Actor*> inventory;

    Bitgrid* visibilityGrid;
    set<Actor*> ownedActors;
    set<Unit*> ownedUnits;
    set<Equipment*> ownedEquipment;
    set<Event*> events;
    set<Actor*> visibleActors;

public:


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
    void add_event(Event *event); ///Implement this
    bool isVisible(Actor*);
    string serialize();

};

///Sets the player's faction - does no sanity checking!
void Player::setFaction(string factionID)
{
    faction = factionID;
}

string Player::getFaction()
{
    return faction;
}

/*!
	\fn Player::Player( string myname, int starting_balance)
	\brief constructor
	\param String of the unique id and the starting balance
*/
Player::Player( string myname, int starting_balance)
{
	userID = myname;
	currbalance = starting_balance;
	visibilityGrid = NULL;
}

///Initializes/updates the visibility grid
void Player::update_visibility(int width, int height)
{
    //Abort on obvious garbage values
    if(width <= 0 || height <= 0)
        return;

    if(visibilityGrid == NULL)
        visibilityGrid = new Bitgrid(width, height);

    visibilityGrid->clear();

    set<Unit*>::iterator unitIter = ownedUnits.begin();

    while(unitIter != ownedUnits.end())
    if((*unitIter)->onMap())
    {
        Vector2<int> pos = (*unitIter)->getPos();

        double radius = (*unitIter)->getDoubleTrait("SightRange");

        if(radius >= 1)
            visibilityGrid->setCircle(pos[0], pos[1], radius);

        unitIter++;
    }

}

///Adds an actor
void Player::add_actor(Actor* actor)
{
    ownedActors.insert(actor);

    Unit *u = dynamic_cast<Unit*>(actor);
    Equipment *e = dynamic_cast<Equipment*>(actor);

    if(u)
        ownedUnits.insert(u);

    if(e)
        ownedEquipment.insert(e);

}

///Destructor
Player::~Player()
{
    if(visibilityGrid)
        delete visibilityGrid;

    events.clear();
    visibleActors.clear();
}

/*!
	\fn string Player::get_name()
	\brief returns the name
	\return String the player name
*/

string Player::get_userid()
{
	return userID;
}

/*!
	\fn int Player::get_balance()
	\brief returns the player's balance
	\return integer value of the amount
*/

int Player::get_balance()
{
	return currbalance;
}

/*!
	\fn bool Player::has_enough_money( unsigned int cost)
	\brief checks if the player has enough money in the balance to buy a unit
	\return true if yes and false if no
*/

bool Player::has_enough_money( double cost)
{
	int tempbalance = currbalance;
	tempbalance -= cost;
	if( tempbalance >= 0)
		return 1;
	else
		return 0;
}


void Player::purchase( double cost)
{
	currbalance-=cost;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

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
    int xpos, ypos;
	set< Player*> involved;
	string serialize_map(map<string, Actor*> *actor_map){
		stringstream str;
		str << "[ ";
		for(map<string, Actor*>::iterator it = actor_map->begin(); it != actor_map->end(); it++)
			str << "\"" + it->first + "\",";
		long pos = (long)str.tellp();
		str.seekp(--pos);
		str << "],";
		return str.str();
	}

    public:

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

	static unsigned long curreventID;

	void genID()
	{
		stringstream str;
		str << 'e' << curreventID++;

		id = str.str();
	}


    Event();
    ~Event();

};

unsigned long Event::curreventID = 0;

///constructor
Event::Event()
{
	affected = NULL;
	destroyed = NULL;
	damaged = NULL;
	instigator = NULL;
	xpos = -1;
	ypos = -1;
	genID();
}

///destructor
Event::~Event()
{
	delete affected;
	delete damaged;
	delete damaged;
}

void Event::setInstigator(Player* player)
{
	if( player != NULL)
		instigator = player;
}

/*!
	\fn bool Event::isInvolved(Player* player)
	\brief checks if player is involved in an event
	\param player pointer to the player
	\author Jonathon Meza/Phil Harmston
*/

bool Event::isInvolved(Player* player)
{

	if(player == NULL)
		return false;
	return (involved.count( player) == 1);
}

/*!
	\fn void Event::addAffected( Actor* affectedActor)
	\brief adds actor to affected
	\param destroyedActor pointer to the affected actor
	\author Phil Harmston
*/

void Event::addAffected( Actor* affectedActor)
{
	if( affected == NULL)
		affected = new map< string, Actor*>;

	affected->insert( pair< string, Actor*>( affectedActor->getID(), affectedActor));
	involved.insert( affectedActor->getPlayer());
}

/*!
	\fn void Event::addDamaged( Actor* damagedActor)
	\brief adds actor to damaged
	\param destroyedActor pointer to the damaged actor
	\author Phil Harmston
*/

void Event::addDamaged( Actor* damagedActor)
{
	if( damaged == NULL)
		damaged = new map< string, Actor*>;

	damaged->insert( pair< string, Actor*>( damagedActor->getID(), damagedActor));
	involved.insert( damagedActor->getPlayer());
}

/*!
	\fn void Event::addDestroyed( Actor* destroyedActor)
	\brief adds actor to damaged
	\param destroyedActor pointer to the destroyed actor
	\author Phil Harmston
*/

void Event::addDestroyed( Actor* destroyedActor)
{
	if( destroyed == NULL)
		destroyed = new map< string, Actor*>;

	destroyed->insert( pair< string, Actor*>( destroyedActor->getID(), destroyedActor));
	involved.insert( destroyedActor->getPlayer());
}

/*!
	\fn void Event::addDestroyed( Actor* destroyedActor)
	\brief sets the origin of the event
	\param x position on the x-axis
	\param y position on the y-axis
	\author Phil Harmston
*/

void Event::setorigin( int x, int y)
{
	if( x > 0 && y > 0)
	{
		xpos = x;
		ypos = y;
	}
}

void Event::setoutcome( string text)
{
	outcome = text;
}

/*!
	\fn string Event::serialize()
	\brief serializes the events in json format
	\author Phil Harmston
*/

string Event::serialize()
{
	stringstream output;

	output << "\"" << id << "\":{";

	//destroyed
	if( destroyed != NULL)
		output << "\"destroyed\": " + serialize_map(destroyed);

	//damaged
	if( damaged != NULL)
		output << "\"damaged\": " + serialize_map(damaged);

	//affected
	if( affected != NULL)
		output << "\"affected\": " + serialize_map(affected);

	//actor
	if( instigator != NULL)
		output << "\"instigator\": \"" << instigator->get_userid() << "\",";

	//origin
	output << "\"origin\": \"" << xpos << " " << ypos << "\",";

	//alerts
	output << "\"alerts\":\"" << outcome << "\"}";

	return output.str();
}

class Simulator
{
    private:
    map<string, Actor*> actors;
    map<string, Unit*> units;
    map<string, Equipment*> equipment;
    map<string, Player*> players;
    map<string, Event*> events;

    public:

    void simulate();


};

#endif // SIMULATOR_H_INCLUDED
