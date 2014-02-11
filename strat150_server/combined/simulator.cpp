#include "simulator.h"

bool areAllies(Player *a, Player *b)
{
    return (a->isAlly(b) && b->isAlly(a));
}

unsigned long Event::curreventID = 0;
unsigned long Actor::currentID = 0;


///Pushes a target to the target queue
void Unit::pushTarget(Unit* target)
{
    targets.push(target);
}

///Retrieves the current target - use caution, may or may not be dead
Unit* Unit::getCurrentTarget()
{
    if(targets.size() == 0)
        return NULL;

    return targets.front();
}

///Pops off a target - use caution, may or may not be dead
Unit* Unit::popTarget()
{
    Unit* t = getCurrentTarget();
    if(t != NULL)
        targets.pop();
    return t;
}

///Empties the target queue
void Unit::clearTargets()
{
    targets = std::queue<Unit*>();
}

///Removes the given actor from this player's inventory
void Player::disown(Actor *actor)
{
    //Don't disown actors that aren't ours
    if(actor == NULL || ownedActors.count(actor) == 0)
        return;

    actor->setPlayer(NULL);
    Unit *u = dynamic_cast<Unit*>(actor); //Unit specific stuff
    Equipment *eq = dynamic_cast<Equipment*>(actor); //Equipment specific stuff
    if(u != NULL)
    {
        ownedUnits.erase(u);
    }

    if(eq != NULL)
    {
        ownedEquipment.erase(eq);
    }

    visibleActors.erase(actor);
    ownedActors.erase(actor);
}

///Claims the given actor - will release this actor from others owned by it
void Player::claim(Actor *actor)
{
    if(actor->getPlayer() != NULL)
        actor->getPlayer()->disown(actor);

    actor->setPlayer(this);
    Unit *u = dynamic_cast<Unit*>(actor); //Unit specific stuff
    Equipment *eq = dynamic_cast<Equipment*>(actor); //Equipment specific stuff
    if(u != NULL)
    {
        ownedUnits.insert(u);
    }

    if(eq != NULL)
    {
        ownedEquipment.insert(eq);
    }

    visibleActors.insert(actor);
    ownedActors.insert(actor);
}

///Returns true if this player considers the other an ally.
bool Player::isAlly(Player *other)
{
    if(other == this) //Always friends with ourselves
        return true;

    return (allies.count(other) != 0);
}

///Sets other player as an ally.  Must be mutual for units to not shoot at each other
void Player::setAlly(Player *other, bool allied)
{
    if(allied)
        allies.insert(other);
    else
        allies.erase(other);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
string Player::serialize( bool privacyFlag)	// if true doesnt show private data (ie balance)
{
	stringstream output;
	if( privBuffer == NULL)
	{
		output << "\"" << userID << "\":{";

		output << "\"score\": " << score << ",";

		output << "\"faction\": \"" << faction << "\",";

		output << "\"nickname\": \"" << nickname << "\",";

		output << "\"color\": [";

        output<< color[0] << ",";
        output<< color[1] << ",";
        output<< color[2];

		output << "]";

		pubBuffer = new string(output.str());
		pubBuffer->append("},");

		output << ",\"balance\": " << currbalance << "},";

		privBuffer = new string(output.str());
	}
	if( !privacyFlag)
		return *privBuffer;
	else
		return *pubBuffer;
}

void Player::clear_buffers()
{
	privBuffer->clear();
	pubBuffer->clear();
}

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
	score = 0;
	privBuffer = NULL;
	pubBuffer = NULL;

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
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

        double radius = (*unitIter)->getMaxDoubleTrait("SightRange");

        if(radius >= 1)
            visibilityGrid->setCircle(pos[0], pos[1], radius);

        unitIter++;
    }

}

bool Player::isVisible(Unit* other)
{
	set<Unit*>::iterator unitIter = ownedUnits.begin();
	for(unitIter; unitIter != ownedUnits.end(); unitIter++)
		if(visibilityGrid->get(other->getPos().x - (*unitIter)->getPos().x, other->getPos().y - (*unitIter)->getPos().y))
			return true;
	return false;
}

bool Player::isVisible(Vector2<int> v)
{
	set<Unit*>::iterator unitIter = ownedUnits.begin();
	for(unitIter; unitIter != ownedUnits.end(); unitIter++)
		if(visibilityGrid->get(v.x - (*unitIter)->getPos().x, v.y - (*unitIter)->getPos().y))
			return true;
	return false;
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

Equipment::Equipment()	// constructor
{
    stringstream str;
    str << "e" << currentID++;

    //Ensures unique IDs upon unit creation
	id = str.str();//rand() % 9999999 + 1000000;
}

string Equipment::serialize()
{
	stringstream str;

	str << "\"" << getID() << "\":{ ";
	str << "\"equipType\": \"" << getType() << "\",";

	//Actor* owner = get_owner();
	//str << "\"equipOwner\": \"" << ((owner == NULL) ? "" : owner->getID()) << "\",";
	str << "\"equipOwner\": \"" <<  this->player->get_userid() << "\",";

	if(getAttached()->size() > 0)
	{
        str << "\"equipment\": [ ";
        set<string>* children = getAttached();
        for(set<string>::iterator it = children->begin(); it != children->end(); it++)
            str << "\"" << *it << "\",";

        str.seekp((long)str.tellp() - 1);
        str << "],";
	}

	string modtraits = serializeModTraits();
	if(modtraits.empty())
		str.seekp((long)str.tellp() - 1);
	else
		str << modtraits;

	str << "}";

	return str.str();
}

Actor* Actor::get_owner()
{
	Actor* curr = this;
	while( curr->getParent() != NULL)
	{
		curr = curr->getParent();
	}
	if( curr == this)
		return NULL;
	return curr;
}

string Actor::getStringTrait( string key)
{
	string trait = modTraits->getString( key);
	if( trait.size() > 0)
		return trait;
	else
	{
		return defTraits->getString( key);
	}
}

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
    destroyed = false;
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

    if(items->count(this) != 0)
        return items;
	items->insert(this);
    if(parent != NULL)// && items->count(parent) == 0)
        parent->getNetwork(items);

    map<string, Actor* >::iterator iter = attached->begin();
    while(iter != attached->end())
    {
        Actor* child = (*iter).second;

        //if(items->count(child) == 0)
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
	sum = getDoubleTrait( key);

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

    newParent->attach_child(this);

    return true;
}

///Adds a modified trait - this will override any traits from rules for this instance.
void Actor::setTrait(string key, double value)
{
    modTraits->addDouble(key, value);
}

///Adds a modified trait - this will override any traits from rules for this instance.
void Actor::setTrait(string key, string value)
{
    modTraits->addString(key, value);
}

///Adds a modified trait - this will override any traits from rules for this instance.
void Actor::setTrait(string key, vector<string> value)
{
    modTraits->addStringArray(key, value);
}

///Adds a modified trait - this will override any traits from rules for this instance.
void Actor::setTrait(string key, vector<double> value)
{
    modTraits->addDoubleArray(key, value);
}

//    void setTrait(string key, vector<double> value);
 //   void setTrait(string key, string value);
  //  void setTrait(string key, vector<string> value);

///Returns the modified traits of this actor as a serialized fragment
string Actor::serializeModTraits()
{
    if(modTraits == NULL)
        return "";

    stringstream str;

    set<string> traits = modTraits->getDoubleKeys();
    set<string>::iterator iter;
    for(iter = traits.begin(); iter != traits.end(); iter++)
        str << "\"" << *iter << "\": " << modTraits->getDouble(*iter) << ", ";

    traits = modTraits->getStringKeys();
    for(iter = traits.begin(); iter != traits.end(); iter++)
        str << "\"" << *iter << "\": \"" << modTraits->getString(*iter) << "\", ";

    traits = modTraits->getStringArrayKeys();
    for(iter = traits.begin(); iter != traits.end(); iter++)
    {
        str << "\"" << *iter << "\": [";

        vector<string> as = modTraits->getStringArray(*iter);

        for(int i = 0; i < as.size(); i++)
        {
            str << "\"" << as[i] << "\"";

            if(i < as.size() - 1)
                str << ",";
        }
        str << "], ";
    }

    traits = modTraits->getDoubleArrayKeys();
    for(iter = traits.begin(); iter != traits.end(); iter++)
    {
        str << "\"" << *iter << "\": [";

        vector<double> as = modTraits->getDoubleArray(*iter);

        for(int i = 0; i < as.size(); i++)
        {
            str << " " << as[i];

            if(i < as.size() - 1)
                str << ",";
        }
        str << "], ";
    }

    string r = str.str();

    return r.substr(0, r.size()-2);

}

set<string>* Actor::getAttached()
{
	set<string>* children = new set<string>();

	map<string, Actor*>::iterator it = attached->begin();
	for(it; it != attached->end(); it++)
		children->insert(it->first);

	return children;
}


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

///Updates waypoints based on this unit's position
bool Unit::updateWaypoints()
{
    if(!hasWaypoints())
        return false;

    Vector2<int> nextPoint = getCurrentWaypoint();

    //If we are on the next waypoint, cycle waypoints
    if(nextPoint.x == (int)floor(coord.x) && nextPoint.y == (int)floor(coord.y))
    {
        popWaypoint();

        if(stance.compare(STANCE_PATROL) == 0 && nextcoord.size() > 1)
            pushWaypoint(nextPoint);

        return true;
    }

    return false;
}

///Returns true if the unit has any waypoints
bool Unit::hasWaypoints()
{
    return nextcoord.size() > 0;
}

///Gets this unit's current waypoint - does not pop it!
Vector2<int> Unit::getCurrentWaypoint()
{
    if(this->nextcoord.size() == 0)
        return coord;

    return nextcoord.front();
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

void Unit::pushWaypoint(Vector2<int> waypoint)
{
	nextcoord.push(waypoint);
}

void Unit::clearWaypoints()
{
	while(!nextcoord.empty())
		nextcoord.pop();
}

Vector2<int> Unit::popWaypoint()
{
	Vector2<int> fr = nextcoord.front();
	nextcoord.pop();
	return fr;
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

string Unit::serialize()
{
	stringstream str;

	str << "\"" << getID() << "\":{ ";
	str << "\"unitType\": \"" << getType() << "\",";
	str << "\"unitOwner\": \"" << getPlayer()->get_userid() << "\",";
	str << "\"pos\": \"" << (int)floor(getPos().x) << " " << (int)floor(getPos().y) << "\",";
	//str << "\"dir\":" << getDir();

	if(this->attached->size() > 0)
	{
        str << "\"equipment\": [ ";
        set<string>* children = getAttached();
        for(set<string>::iterator it = children->begin(); it != children->end(); it++)
            str << "\"" << *it << "\",";

        str.seekp((long)str.tellp() - 1);
        str << "],";
	}

	string modtraits = serializeModTraits();
	if(modtraits.empty())
		str.seekp((long)str.tellp() - 1);
	else
		str << modtraits;

	str << "}";

	return str.str();
}


///constructor
Event::Event()
{
    buffer = "";
	affected = NULL;
	destroyed = NULL;
	damaged = NULL;
	instigator = NULL;
	origin.x = -1;
	origin.y = -1;
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
		origin.x = x;
		origin.y = y;
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

	if(buffer.size() > 0)
        return buffer;

	output << "\"" << id << "\":{";

	//destroyed
	if( destroyed != NULL)
		output << "\"destroyed\": " << serialize_map(destroyed);

	//damaged
	if( damaged != NULL)
		output << "\"damaged\": " << serialize_map(damaged);

	//affected
	if( affected != NULL)
		output << "\"affected\": " << serialize_map(affected);

	//actor
	if( instigator != NULL)
		output << "\"instigator\": \"" << instigator->get_userid() << "\",";

	//origin
	output << "\"origin\": \"" << origin.x << " " << origin.y << "\",";

	//alerts
	output << "\"alerts\":\"" << outcome << "\"}";

	buffer = output.str();

	return buffer;
}

Vector2< int> Event::get_origin()
{
	return origin;
}
