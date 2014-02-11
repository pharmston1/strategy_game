#include "world.h"


stringstream World::currentLog;

///Constructor
World::World(Properties &properties)
{
    string rulesPath = properties.rulesFile;
    string mapPath = properties.mapFile;

    settings = &properties;

    worldTime = 0;
    currentTick = 0;
    nextTick = 1;
    currentLog << "World created." << endl;


    if(rules.loadJsonFile(rulesPath))
    {
        currentLog << "Rules file " << rulesPath << " loaded" << endl;
        currentLog << "  Sections: " << (rules.getSingleEntryKeys().size() + rules.getGroupKeys().size()) << endl;
        factions = rules.getGroupEntryKeys("factions");
        currentLog << factions.size() << " faction(s)" << endl;
        currentLog << rules.getGroupEntryKeys("units").size() << " unit type(s)" << endl;
        currentLog << rules.getGroupEntryKeys("equipment").size() << " equipment type(s)" << endl;
    }
    else
        currentLog << "WARNING: Failed to load rules file " << rulesPath <<  endl;
    if(gameMap.constructMap(mapPath))
    {
        currentLog << "Map file " << mapPath << " loaded" << endl;

        std::stringstream str;
        str.clear();
        str.str(std::string());
        string r;
        r = "  Map name: " + gameMap.getTitle() + "\n";
        str << gameMap.getWidth() << "w x " << gameMap.getHeight() << "h \n";
        r += "  Dimensions: " + str.str();
        str.str(std::string());
        str << gameMap.getTileSet().size();
        r += "  Tiles Types: " + str.str() + "\n";
        str.str(std::string());
        str << gameMap.getTiles().size();
        r += "  Non-default tiles: " + str.str() + "\n";
        str.str(std::string());
        r += "  Description: " + gameMap.getDescription() + "\n";

        currentLog << r;

    }
    else
        currentLog << "WARNING: Failed to load map file " << mapPath <<  endl;


}

//Destructor
World::~World()
{
    while(actors.size() > 0)
        this->removeActor(actors.begin()->second);

    while(players.size() > 0)
    {
        delete players.begin()->second;
        players.erase(players.begin());
    }

}

///Uses the magic of STL Algorithms to retrieve a set of needed actors
set<Actor*> World::findActors(Player *owner, set<Actor*> exclude, string type)
{
    set<Actor*> result;

    std::set_difference(typeActors[type].begin(), typeActors[type].end(),
                        exclude.begin(), exclude.end(),
    std::inserter(result, result.end()));

    set<Actor*>::iterator iter = result.begin();

    if(owner != NULL)
    while(iter != result.end())
    {
        if((*iter)->getPlayer() != owner)
            result.erase(iter++);
        else
            iter++;
    }

    return result;
}

///Gets the world log for the specified tick
string World::dumpLog(int tick)
{
    if(currentTick == tick)
        return currentLog.str();

    return tickLogs[tick];
}

///Refreshes player visibility
void World::refreshPlayerVisibility()
{
    map<string, Player*>::iterator playerIter = players.begin();
    for(playerIter; playerIter != players.end(); playerIter++)
        playerIter->second->update_visibility(gameMap.getWidth(), gameMap.getHeight());

    //currentLog << "Refreshed player visibility..." << endl;
}

///Gets tile type at a given coordinate
string World::getTileTypeAt(double x, double y)
{
    if(x < 0 || y < 0 || x >= gameMap.getWidth() || y >= gameMap.getHeight())
        return gameMap.getDefaultTileType();

    Tile* tile = gameMap.getTiles()[(int)x][(int)y];
    return ((tile != NULL) ? tile->getTerrainType() : gameMap.getDefaultTileType());
}

///info buffer for serialization///////////////////////////////////////////////////

void World::updateInfoBuffer(time_t complete, time_t next)
{
    stringstream str;
    str << "\"info\":{";
    str << "\"tick\":" << currentTick << ",";
    str << "\"time\":" << complete << ",";
    str << "\"next\":" << next << ",";
    str << "\"motd\":" << "\"Message of the day\"" << "}";
    infoBuffer = str.str();
}

string World::serializeInfo()
{
    return infoBuffer;
}

string World::serializeEvents()
{
	stringstream str;

	str << "\"events\":{ ";

	map<string, Event*>::iterator eventIter = events.begin();
	for(eventIter; eventIter != events.end(); eventIter++)
		str << eventIter->second->serialize() << ",";

	str.seekp((long)str.tellp() - 1);
	str << "},";

	return str.str();
}

bool World::isVisible(Event* ev, Player* pl)
{
	return ev->isInvolved(pl);
}

void World::addEvent(Event* eve)
{
	if(eve != NULL)
		events.insert(pair<string, Event*>(eve->getID(), eve));
}

string World::serializePlayers(bool privacy)//Player* player)
{
	stringstream str;

	str << "\"players\":{ ";

	map<string, Player*>::iterator playerIter = players.begin();
	for(playerIter; playerIter != players.end(); playerIter++)
		str << playerIter->second->serialize(privacy);

	str.seekp((long)str.tellp() - 1);
	str << "},";

	return str.str();
}

string World::serializeUnits()
{
	stringstream str;

	str << "\"units\":{ ";

	map<string, Unit*>::iterator unitIter = units.begin();
	for(unitIter; unitIter != units.end(); unitIter++)
		str << unitIter->second->serialize() << ",";

	str.seekp((long)str.tellp() - 1);
	str << "},";

	return str.str();
}

string World::serializeEquipment()
{
	stringstream str;

	str << "\"equipment\":{ ";

	map<string, Equipment*>::iterator eqIter = equipments.begin();
	for(eqIter; eqIter != equipments.end(); eqIter++)
		str << eqIter->second->serialize() << ",";

	str.seekp((long)str.tellp() - 1);
	str << "},";

	return str.str();
}

string World::serializeDiplomacy(set<Player*> pl)
{
	stringstream str;

	str << "\"diplomacy\":{ ";

	str << "\"allies\": [ ";

	set<Player*>::iterator plIter = pl.begin();
	for(plIter; plIter != pl.end(); plIter++)
	{
		for(set<Player*>::iterator alIter = pl.begin(); alIter != pl.end(); alIter++)
		{
			if((*plIter)->isAlly(*alIter) && (*plIter) != (*alIter))
				str << "\"" << (*plIter)->get_userid() << " " << (*alIter)->get_userid() << "\",";
		}
	}

	str.seekp((long)str.tellp() - 1);
	str << "]";
	str << "},";

	return str.str();
}

///Adds a whole slew of orders in an OrdersLoader
void World::loadOrders(OrdersLoader *loader)
{
    vector<Orders*> ordersArray = loader->getOrders();

    if(ordersArray.size() > 0)
    {
        currentLog << endl << "Parsing " << ordersArray.size() << " order(s)" << endl;

        for(int i = 0; i < ordersArray.size(); i++)
        {
            //cout << ordersArray[i]->getLog();
            addOrders(ordersArray[i]);
            //cout << currentLog.str() << endl;
        }

    }

}

///Adds a single orders to the world, returns success
bool World::addOrders(Orders* orders)
{
    string userID = orders->getUserID();

    Player *player;

    //Player not found, allocate it
    if(players.count(userID) == 0)
    {
        currentLog << "Found new player [" << userID << "]" << endl;
        player = new Player(userID, DEFAULT_STARTING_BALANCE);
        players[userID] = player;

        string faction = orders->faction;

        if(factions.count(faction) == 0)
        {
            currentLog << " WARNING: Player-specified faction " << faction << " not found.  Using default faction " << DEFAULT_FACTION << endl;
        }

        player->color[0] = orders->color[0];
        player->color[1] = orders->color[1];
        player->color[2] = orders->color[2];

        player->nickname = orders->nickname;
        player->setFaction(faction);
    }

    player = players[userID];

    //Parse unit orders
    currentLog << "Parsing orders from [" << userID << "]" << endl;
    if(orders->unitOrders.size() > 0)
    {
        currentLog << "Checking unit orders..." << endl;

        map<string, Orders::UnitOrder*>::iterator orderIter = orders->unitOrders.begin();
        while(orderIter != orders->unitOrders.end())
        {
            Orders::UnitOrder *unitOrder = orderIter->second;
            string unitID = orderIter->first;
            orderIter++;

            currentLog << "Parsing orders for " << unitID << "...";

            //Make sure our unit exists!
            if(units.count(unitID) > 0)
            {
                Unit* unit = units[unitID];

                //Make sure we're not ordering other units around
                if(unit->getPlayer() == player)
                {
                    currentLog << "found." << endl;

                    unit->clearTargets();
                    unit->clearWaypoints();

                    //Waypoint queue
                    while(unitOrder->coords.size() > 0)
                    {
                        Vector2<int> coord = unitOrder->coords.front();

                        if(gameMap.inBounds(coord.x, coord.y))
                            unit->pushWaypoint(coord);
                        else
                            currentLog << "Coordinates "
                                       << coord << " are out of bounds!" << endl;

                        unitOrder->coords.pop();
                    }

                    //Target queue
                    for(int i = 0; i < unitOrder->target_ids.size(); i++)
                    if(units.count(unitOrder->target_ids[i]) > 0)
                    {
                        unit->pushTarget(units[unitOrder->target_ids[i]]);
                    }

                    unit->setStance(unitOrder->stance);

                }
                else currentLog << "does not belong to player." << endl;

            }
            else currentLog << "no match." << endl;


        }


    }



    //First check for unit purchases
    if(orders->buys.size() > 0)
    {
        currentLog << "Checking buy order(s)..." << endl;
        //currentLog << orders->getLog() << endl;

        set<Orders::BuyInfo*>::iterator buyIter = orders->buys.begin();
        while(buyIter != orders->buys.end())
        {

            Orders::BuyInfo *buy = *buyIter;
            buyIter++;

            int itemCost = rules.getDouble("units", buy->unitType, "Cost");

            if(buy->unitType.size() > 0)
            {
                currentLog << "  Buying unit " << buy->unitType << endl;
            }

            //Buying equipment
            set<string>::iterator eqIter = buy->equip.begin();
            while(eqIter != buy->equip.end())
            {
                currentLog << "  Buying equipment " << *eqIter << endl;
                itemCost += rules.getDouble("equipment", *eqIter, "Cost");
                eqIter++;
            }

            int totalCost = itemCost*buy->quantity;
            currentLog << " Ind. cost = " << itemCost << "  Total: " << itemCost*buy->quantity << endl;

            if(player->has_enough_money(totalCost))
            {
                player->purchase(totalCost);

                Unit *base;
                queue<Unit*> bases;
                queue<Equipment*> equips;

                //Attempt to create the unit
                for(int i = 0; i < buy->quantity; i++)
                {
                    base = createUnit(buy->unitType, player);
                    if(base)
                    {
                        bases.push(base);

                        //TODO:  More drop sanity checking
                        if(buy->drop.xpos >= 0 && buy->drop.ypos >= 0)
                        {
                            Vector2<int> dropCoords = this->getNearestOpen(Vector2<double>(buy->drop.xpos, buy->drop.ypos));
                            currentLog << "  Dropping " << buy->unitType << " " << base->getID() << " to " << dropCoords.x << ", " << dropCoords.y << endl;
                            if(!moveUnit(base, dropCoords))
                                cout << "Move unit failed." << endl;
                        }
                    }
                    else
                        break;
                }

                set<string>::iterator eqIter = buy->equip.begin();
                while(eqIter != buy->equip.end())
                {
                    bool attachBase = false;
                    if(base != NULL && buy->assembly.nodes.count(*eqIter) == 0)
                        attachBase = true;

                    for(int i = 0; i < buy->quantity; i++)
                    {
                        Equipment *eq = createEquipment(*eqIter, player);
                        equips.push(eq);

                        //Do implied attachments
                        if(attachBase && bases.size() > 0)
                        {
                            Unit *b = bases.front();

                            if(b != NULL && b->attach_child(eq))
                                currentLog << "  Attaching " << eq->getID()
                                           << " to base " <<     b->getID() << endl;

                            bases.pop();
                        }

                    }
                    eqIter++;
                }

                //Now for attachments
                if(buy->assembly.quantity)
                //for(int i = 0; i < buy->quantity; i++)
                {
                    Orders::AttachNode *node;

                    string key;

                    //Cycle through the whole attachment thing
                    map<string, Orders::AttachNode*>::iterator nodeIter = buy->assembly.nodes.begin();
                    while(nodeIter != buy->assembly.nodes.end())
                    {
                        key = nodeIter->first;
                        node = nodeIter->second;
                        currentLog << " " << key;

                        if(node->parent != NULL)
                        {
                           currentLog << "->" << node->parent->label;
                           currentLog << " [" << node->tag << "+" << node->parent->tag << "]" << endl;

                           set<Actor*> parents = this->findActors(player, set<Actor*>(), node->parent->tag);
                           set<Actor*> children = this->findActors(player, set<Actor*>(), node->tag);
                           for(int i = 0; i < buy->quantity; i++)
                           {
                                //Destructively read it out
                                Actor *pActor = *parents.begin();
                                Actor *cActor = *children.begin();
                                if(pActor && cActor)
                                {
                                    currentLog << "  Attaching " << cActor->getID() << " to " << pActor->getID();
                                    parents.erase(parents.begin());
                                    children.erase(children.begin());

                                    if(pActor->attach_child(cActor))
                                        currentLog << "...success." << endl;
                                    else
                                        currentLog << "...failed!  Are there any circular references?" << endl;


                                }
                                else break;
                           }

                        }

                        nodeIter++;
                    }

                    currentLog << endl;
                }
            }
            else
            {
                currentLog << "Player cannot afford this!  Aborting purchase." << endl;
            }

        }

        currentLog << "Buy order done." << endl;

    }////////////////////////////////////////////////////////////
    //else currentLog << orders->getLog() << endl;


    return true;
}

///Culls an actor from the world
void World::removeActor(Actor *actor)
{
    if(!actor->isDestroyed()) //Can't remove non-destroyed actors
        return;

    Unit *u = dynamic_cast<Unit*>(actor); //Unit specific stuff
    Equipment *eq = dynamic_cast<Equipment*>(actor); //Equipment specific stuff
    if(u != NULL)
    {
        //If it's a unit, remove it
        unitLocations.erase(getCoordIndex(u->getPos().x, u->getPos().y));
        units.erase(actor->getID());
    }

    if(eq != NULL)
    {
        equipments.erase(actor->getID());
    }

    Player *p = actor->getPlayer();
    if(p != NULL)
    {
        p->disown(actor);
    }

    currentLog << "Culling destroyed actor [" << actor->getID() << "] " << endl;
    allocatedActors.erase(actor);
    typeActors[actor->getType()].erase(actor);
    delete actor;
}

///Creates and assigns a unit to a player
Unit* World::createUnit( string unitType, Player* owner)
{
	if( players.count( owner->get_userid()) == 1 && rules.getGroupEntryKeys( "units").count( unitType) == 1)
	{
		Unit* newunit = new Unit;
		newunit->setPlayer( owner);
		owner->add_actor(newunit);
		newunit->setType( rules.getEntry( "units", unitType));
		units.insert( pair< string, Unit*>( newunit->getID(), newunit));
		actors.insert( pair<string, Actor*>( newunit->getID(), newunit));
		unitLocations.insert( pair< int, Unit*>( getCoordIndex( newunit->getPos().x, newunit->getPos().y), newunit));
		currentLog << "unit created: " << newunit->getID() << endl;
		allocatedActors.insert(newunit);
		typeActors[unitType].insert(newunit);
		return newunit;
	}
	currentLog << "Unit creation failed: player or unit type not found" << endl;
	return NULL;	// if player or unit not found
}

///Creates and assigns an equipment to a player
Equipment* World::createEquipment( string equipType, Player* owner)
{
	if( players.count( owner->get_userid()) == 1 && rules.getGroupEntryKeys( "equipment").count( equipType) == 1)
	{
		Equipment* newequip = new Equipment;
		newequip->setPlayer( owner);
		newequip->setType( rules.getEntry( "equipment", equipType));
		equipments.insert( pair< string, Equipment*>( newequip->getID(), newequip));
		actors.insert( pair<string, Actor*>( newequip->getID(), newequip));
		currentLog << "Equipment created: " << newequip->getID() << endl;
		allocatedActors.insert(newequip);
		typeActors[equipType].insert(newequip);
		return newequip;
	}
	currentLog << "Equipment creation failed: player or unit type not found" << endl;
	return NULL;	// if player or equipment not found
}

///Updates and moves tick forward, returns the world's tick number
int World::advanceTick(time_t complete, time_t next)
{
	refreshPlayerVisibility();
	double deltaTime = 1.0 / STEP_COUNT;
	for(int i = 0; i < STEP_COUNT; i++)
		update(deltaTime);
	refreshPlayerVisibility();
	assignEvents();

    //Archive and clear the log buffer
    tickLogs[currentTick] = currentLog.str();
	currentLog.str("");

	updateInfoBuffer(complete, next);

	nextTick = ++currentTick + 1;
	return currentTick - 1;
}

///Runs all of the necessary updates
void World::update(double deltaTime)
{
	updateUnits(deltaTime);
	updateEquipment(deltaTime);
	updatePlayers(deltaTime);
}

///Gets the designated unit's current target - accounts for deleted units
Unit* World::getUnitTarget(Unit *unit)
{
    Unit* target = unit->getCurrentTarget();
    if(target == NULL)
        return target;

    //Clean up after deleted actors
    if(allocatedActors.count(target) == 0)
    {
        //currentLog << "Orphan target for " << unit->getID() << " found." << endl;
        unit->popTarget();
        return getUnitTarget(unit); //Go until the queue runs dry
    }

    return target;

}

///Iterates through all of the units, updating them
void World::updateUnits(double deltaTime)
{
	//project location
	map<string, Unit*>::iterator unitIter = units.begin();
	for(unitIter; unitIter != units.end(); unitIter++)
	{
        string unitID = unitIter->first;
		Unit *unit = unitIter->second;

	//update movement
	if(!unit->isDestroyed() && unit->hasWaypoints())
	{
		//Gets unit speed
		double unit_speed = unit->getTotalDoubleTrait("MoveSpeed");

		//Gets modifier from the terrain where the player is at
		Vector2<double> curr_coord = unit->getPos();

		string terrainType = getTileTypeAt(curr_coord.x, curr_coord.y);

		string modType = unit->getStringTrait("MoveType");

		double modifier = rules.getEntry("terrainModifier", modType)->getDouble(terrainType);

        Vector2<double> ab;
		//Gets vector from current position to next point
		if(this->getUnitTarget(unit) != NULL)
            ab = this->getUnitTarget(unit)->getPos() - curr_coord; //Chase targets
        else
            ab = unit->getCurrentWaypoint() - curr_coord;

        //currentLog << ab.x << " " << ab.y << endl;

		//Gets vector to move unit
		ab.normalize();
		ab *= unit_speed * modifier * deltaTime;

        curr_coord += ab;
		unit->setPos(curr_coord);

        //Collision!
		if(!moveUnit(unit, curr_coord))
		{
            Unit *oU = getUnitAt(curr_coord.x, curr_coord.y);
            Player* o = oU->getPlayer();

            if(areAllies(o, unit->getPlayer()))
            {//currentLog << "FRIENDLY BUMP has been detected at "
			 //           << curr_coord.x << " " << curr_coord.y << endl;
            }
            else
			{
                doCombat(unit, oU);
			}
        }

        if(unit->updateWaypoints())
            currentLog << "Unit " << unit->getID() << " at waypoint ("
                       << floor(curr_coord.x) << ", " << floor(curr_coord.y) << ")" << endl;
	}

	if(unit->isDestroyed())
        this->removeActor(unit);
	}
}

///Initiates combat between two units
void World::doCombat(Unit *attacker, Unit *defender)
{
    currentLog << " --- " << endl;
    string tileType = getTileTypeAt(defender->getPos().x, defender->getPos().y);

    Player *attPlayer = attacker->getPlayer();
    Player *defPlayer = defender->getPlayer();

    string attType = attacker->getStringTrait("DisplayName");
    string defType = defender->getStringTrait("DisplayName");

    if(areAllies(defPlayer, attPlayer))
        currentLog << " FRIENDLY FIRE IN PROGRESS!" << endl;

    double attPower = attacker->getTotalDoubleTrait("Damage");
    double defPower = defender->getTotalDoubleTrait("Damage");

    string attArmorKey = attacker->getStringTrait("Armor");
    double attArmor = rules.getDouble("armorTypes", attArmorKey);
    string defArmorKey = defender->getStringTrait("Armor");
    double defArmor = rules.getDouble("armorTypes", defArmorKey);

    string attMoveType = attacker->getStringTrait("MoveType");
    double attDefBonus = rules.getDouble("defenseBonus", attMoveType, tileType);
    double attAttBonus =  rules.getDouble("attackBonus", attMoveType, tileType);

    string defMoveType = defender->getStringTrait("MoveType");
    double defDefBonus = rules.getDouble("defenseBonus", defMoveType, tileType);
    double defAttBonus =  rules.getDouble("attackBonus", defMoveType, tileType);


    double attHP = attacker->getTotalDoubleTrait("Health");
    double defHP = defender->getTotalDoubleTrait("Health");

    currentLog << "Combat initiated between "
               << attPlayer->get_userid() << " (" << attType << " " << attacker->getID() << ") and "
               << defPlayer->get_userid() << " (" << defType << " " << defender->getID() << ") - Terrain: "
               << tileType << endl;

    currentLog << " Attacker: " << attPower << "dmg " << attHP << "hp "
               << attArmorKey << " Armor (" << attArmor << ") "
               << " - " << attMoveType << " A" << attAttBonus << "/D" << attDefBonus << endl;

    currentLog << " Defender: " << defPower << "dmg " << defHP << "hp "
               << defArmorKey << " Armor (" << defArmor << ") "
               << " - " << defMoveType << " A" << defAttBonus << "/D" << defDefBonus << endl;

    double attEffective =   attPower
                          * attAttBonus
                          * (1 - defArmor)
                          * (1 - defDefBonus);

    double defEffective =   defPower
                          * defAttBonus
                          * (1 - attArmor)
                          * (1 - attDefBonus);

    currentLog << "  Attacker effective power: " << attEffective << endl
               << "  Defender effective power: " << defEffective << endl << endl;

    if(attEffective <= 0 && defEffective <= 0)
    {
        currentLog << "  \nDraw - neither unit has attack capability.\n ---";
        return;
    }

    int shots = 0;
    while(attHP > 0 && defHP > 0)
    {
        attHP -= defEffective;
        defHP -= attEffective;
        shots++;
    }

    Unit* victor;
    double vRemHP;
    Unit* loser;
    double lRemHP;

    stringstream summary;

    if(attHP > defHP)
    {
        currentLog << "  Attacker wins." << endl;
        victor = attacker;
        vRemHP = attHP;
        loser  = defender;
        lRemHP = defHP;
    }
    else
    {
        currentLog << "  Defender wins." << endl;
        victor = defender;
        vRemHP = defHP;
        loser  = attacker;
        lRemHP = attHP;
    }

    Event* event = new Event();

    if(vRemHP <= 0)
    {
        currentLog << "  Victor destroyed." << endl;
        event->addDestroyed(victor);
        victor->destroy();
        loser->setTrait("Health", lRemHP);
        event->setoutcome(victor->getStringTrait("DisplayName")+" and "+loser->getStringTrait("DisplayName")+" destroyed each other!  THERE WERE NO SURVIVORS.");
    }

    if(lRemHP <= 0)
    {
        currentLog << "  Loser destroyed." << endl;
        event->addDestroyed(loser);
        loser->destroy();
        victor->setTrait("Health", vRemHP);
        event->setoutcome(victor->getStringTrait("DisplayName")+" destroyed "+loser->getStringTrait("DisplayName")+" in combat!");
    }



    currentLog << "  Shots fired: " << shots << endl;


    this->addEvent(event);



    event->addAffected(victor);
    event->setInstigator(victor->getPlayer());
    event->setorigin(victor->getPos().x, victor->getPos().y);

    currentLog << " --- " << endl << event->serialize() << endl;
    //TODO:  ARMOR PIERCING EFFECTS



}

void World::updateEquipment(double deltaTime)
{
    //Equipment update stuff happens here
}

void World::updatePlayers(double deltaTime)
{

}

void World::assignEvents()
{
}

///adds a player to the world
Player* World::addPlayer( string userid, string faction)
{
	if( rules.getGroupEntryKeys( "factions").count( faction) == 1)
	{
		Player* newplayer = new Player( userid, settings->balance);
		newplayer->setFaction( faction);
		players.insert( pair< string, Player*>( userid, newplayer));
		currentLog << "Player " << userid << " added to world" << endl;
		return newplayer;
	}
	currentLog << "Player creation failed" << endl;
	return NULL;
}

/// gets the key for the unitLocations map
int World::getCoordIndex( double x, double y)
{
	return  ((int)floor(y)) * gameMap.getWidth() + ((int)floor(x));
}

///Retrieves the unit at the specified coordinates, or NULL if none found
Unit* World::getUnitAt( double x, double y)
{
	if( x >= 0 && y >= 0)
	{
		map< int, Unit*>::iterator iter = unitLocations.find( getCoordIndex( x, y));
		if( iter != unitLocations.end())
			return (*iter).second;
	}
	return NULL;
}

///Retrieves the unit at the specified coordinates, or NULL if none found
Unit* World::getUnitAt(Vector2<double> pos)
{
    return getUnitAt(pos.x, pos.y);
}

///Finds a nearby unocuppied tile for the specified point
Vector2<int> World::getNearestOpen(Vector2<double> point)
{

    //int index = getCoordIndex(point.x, point.y);
    if(!this->getUnitAt(point))
    {
//        cout << "Found immediately: " << point << endl;
        return point;
    }


    Vector2<int> n(1, 1);
    Vector2<int> nearestPoint(-gameMap.getWidth(), -gameMap.getHeight());

    //While the nearest point has not been found
    while(nearestPoint.x < 0 || nearestPoint.y < 0)
    {
//    cout << "n = " << n << ": ";
        Vector2<int> llCorner = point - n;
        Vector2<int> urCorner = point + n;
        Vector2<int> test;

        //Using square length for performance
        double bestDist = nearestPoint.lengthSq();

        //Scan through the search space
        for(test.x = llCorner.x; test.x < urCorner.x; test.x++)
            for(test.y = llCorner.y; test.y < urCorner.y; test.y++)
            if(!getUnitAt(test) && gameMap.inBounds(test.x, test.y)) //Found an empty square
            {
                Vector2<int> line = test - point;
                if(line.lengthSq() < bestDist)
                {
                    bestDist = line.lengthSq();
                    nearestPoint = test;
                }
            }
        n = n * 2;
    }

//    cout << nearestPoint << " " << endl;

    return nearestPoint;
}

///Updates the specified unit's position - returns true if no collision
bool World::moveUnit(Unit *mUnit, Vector2<double> newPos)
{
    return moveUnit(mUnit, newPos.x, newPos.y);
}

///Updates the specified unit's position - returns true if no collision
bool World::moveUnit( Unit* mUnit, double x, double y)
{

    Unit* u = getUnitAt( x, y);
    if(u == mUnit)
        return true; //We don't care about colliding with ourselves

	if(u == NULL || u->isDestroyed())
	{

        int index = getCoordIndex(x, y);
        int oldIndex = getCoordIndex( mUnit->getPos().x, mUnit->getPos().y);
		unitLocations.erase(oldIndex);
		unitLocations.insert( pair< int, Unit*>(index, mUnit));
		mUnit->setPos(x, y);
		//currentLog << mUnit->getID() << " " << oldIndex << " -> " << index
		//           << " " << x << ", " << y << endl;
		return true;
	}

    //currentLog << "Collision: " << mUnit->getID() << " vs " << u->getID() << endl;
	return false;
}

string World::masterSerialize( Player* currplayer)
{
	stringstream master;
	if(infoBuffer.size() == 0)
        updateInfoBuffer(0, time_t());

	master<< "{";
	master << serializePlayers( true);
	master << serializeEvents();
	master << serializeUnits();
	master << serializeEquipment();

	set< Player*> sp;

	map< string, Player*>::iterator pit = players.begin();
	for( pit; pit != players.end(); pit++)
		sp.insert( pit->second);

	master << serializeDiplomacy(sp);
	master << serializeInfo();

//	master.seekp( (long) master.tellp() -1);
	master<< "}";
	return master.str();
}

void World::log()
{
	string path = "log.txt";
	path.insert( 0, DEFAULT_LOG);
	ofstream mylog( path.c_str());
	mylog << currentLog.str() << endl;
	mylog << "/////////////////////////////" << endl << "////////////////////////////"<< endl;
	mylog.close();
}