#ifndef Update_H
#define Update_H

//#include "tree.h"
#include "Players.h"
#include "Units.h"
#include "Files.h"
#include "GameMap.h"
#include "Equipment.h"
//#include "Statistic.h"

/*! \file */

using namespace std;

/*!
	\class OrdersLoader
	\brief Parses all orders files, generating requisite orders objects.
	\author Chris Livingston/Phil Harmston
*/

class OrdersLoader
{
    private:


};

/*!
	\fn Update::Update( string fname)
	\brief constructor: calls rules serializer
*/
Update::Update(string rulesFile, string mapFile)
{
	units = new map< string, Unit*>;
	players = new map< string, Player*>;
	rc = new RulesContainer;
    rc->loadJsonFile(rulesFile);

	if(!fexists(mapFile.c_str()))
		cout << "Map file " << mapFile << " not found!" << endl;
	else
		if(gameMap.constructMap(mapFile))
			gameMap.drawMap();
		else
			cout << "Failed to load map " << mapFile << endl;
}

/*!
	\fn vector< Player>* Update::get_players()
	\return vector< Player>* is list of players
*/

map< string, Player*>* Update::get_players()
{
	return players;
}

/*!
	\fn void Update::main_update()
	\brief goes the orders update the units and players for next simulation
*/

void Update::main_update( queue< Orders*> ordersQueue)
{
	while( !ordersQueue.empty())
	{
		string userID = ordersQueue.front()->getUserID();

		if( !ordersQueue.front()->unit_info.empty())	// update the units values
			apply_orders( ordersQueue.front()->unit_info);
		if( !ordersQueue.front()->buys.empty())	// update buying or not
			update_buys(userID, ordersQueue.front()->buys);

		ordersQueue.pop();
	}
}

/*!
	\fn void Update::apply_orders( Orders &unitord)
	\brief updates units with the order information
	\param unitord Orders that are disseminate to the units
*/
void Update::apply_orders( map< string, Orders::Info> unitinfo)
{

	map<string, Orders::Info>::iterator iter = unitinfo.begin();
	while(iter != unitinfo.end())
	{
		map<string, Unit*>::iterator iter2 = units->begin();
		while( iter2 != units->end())
		{
			if( (*iter).first.compare( (*iter2).first) == 0)
			{
				if( !(*iter).second.coord->empty())	// unit's updated move coordinates
					(*iter2).second->nextcoord = *(*iter).second.coord;
				else if( !(*iter).second.stance.empty())	// unit's updated stance
					(*iter2).second->stance = (*iter).second.stance;
				else if( !(*iter).second.target_ids.empty())	// unit's updated targets
					(*iter2).second->targets = (*iter).second.target_ids;
			}
		}
		iter++;
	}
}

/*!
	\fn void Update::update_buys( map<string, unsigned int> buys)
	\brief updates the buys queue for add_unit function
	\param buys     Map of the unit type and the number of that type purchased
*/
void Update::update_buys(string userID, map<string, Orders::BuyInfo*> buys)
{
    //Safeguard
    if(players->count(userID) == 0)
    {
        return; //Player not found
    }

    Player* currplayer = players->at(userID);

	map< string, Orders::BuyInfo*>::iterator it = buys.begin();
	//if(
	while( it != buys.end())
	{
        //Create units and assign to players
		for( unsigned int i = 0; i < (*it).second->quantity; i++)
		{
			
			int cost = get_fullunit_cost( (*it).second->unitType, &(*it).second->equip, currplayer);
			if( currplayer->has_enough_money( cost))
			{
				currplayer->purchase( cost);
				if( rc->getEntry( "units", (*it).second->unitType) != NULL)
				{
					Unit* newunit = new Unit;
					newunit->setPlayer( lookUpPlayer( userID));
					newunit->setType( rc->getEntry( "units", (*it).second->unitType));
					units->insert( pair< string, Unit*>( newunit->getID(), newunit));
				}
				else
				{
					printw("unit not foud\n");
					refresh();
				}

				size_t x, y;
				map< string, Equipment*> labels;
				for( int i = 0; i < (*it).second->equip.size(); i++)
				{
					if( (*it).second->equip.empty())
					{
						x = (*it).second->equip.at(i).find(' ');
						string equiptype = (*it).second->equip.at(i).substr( 0, x);
						x = (*it).second->equip.at(i).find_last_of(' ');
						//string labels = (*it).second->equip.at(i).substr( x);

						Equipment *newequip = new Equipment;
						newequip->setPlayer( lookUpPlayer( userID));
						newequip->setType( rc->getEntry( "equipment", equiptype));

						labels.insert( pair< string, Equipment*>( (*it).second->equip.at(i).substr( x), newequip));

						newequip->setType( rc->getEntry( "equipment", equiptype));
						equipment->insert( pair< string, Equipment*> ( newequip->getID(), newequip));
					}
				}
				/*for( int i = 0; i < (*it).second->attached.size(); i++)
				{
					string bstr, astr;
					size_t a = (*it).second->attached.at(i).find(' ');	// finding space separating x and y
					astr = (*it).second->attached.at(i).substr( 0, a);
					bstr = (*it).second->attached.at(i).substr(a--);
					if( labels.count( astr) == 1 && labels.count(bstr))
						//units->at( i)labels.at( astr)
				}*/
			}
        }

		it++;
	}
	//buys.clear();
	//add_unit( playerbuys);
}

void Update::attach_buys( vector< string> attachinfo, vector< string> equip)
{
	for( int i = 0; i < attachinfo.size(); i++)
	{
		size_t x = attachinfo.at(i).find_last_of(' ');
		string temp = attachinfo.at(i).substr( x);

	}
}

int Update::get_fullunit_cost( string unittype, vector< string>* equipinfo, Player* player)
{
	int unitSum = 0;
	if( !unittype.empty())
		unitSum = (int)rc->getDouble( "units", unittype, "Cost");


	for( int i = 0; i < equipinfo->size(); i++)
	{
		size_t x = equipinfo->at(i).find(' ');
		string equiptype = equipinfo->at(i).substr( 0, x);
		size_t y = equipinfo->at(i).find_last_of(' ');
		size_t z = y - x;
		string quant = equipinfo->at(i).substr( ++x, --z);

		if( player->has_enough_money( unitSum + ((int) rc->getDouble( "equipment", equiptype, "Cost") * atoi( quant.c_str()))))
			continue;
		unitSum = unitSum + ((int) rc->getDouble( "equipment", equiptype, "Cost") * atoi( quant.c_str()));
	}
	printw("cost: %d\n", unitSum);
	refresh();
	return unitSum;
}

/*void Update::update_messages( map< string, string> messages)
{

}*/

/*!
	\fn queue<Orders> Update::get_orders()
	\return queue of orders
*/

/*queue<Orders> Update::get_orders()
{
	return ordersQueue;
}*/

/*!
	\fn Player* Update::lookUpPlayer( string user)
	\param user userid of player
	\return Player* finds player
*/

Player* Update::lookUpPlayer( string user)
{
	return players->at( user);
}

Unit* Update::lookUpUnit( string unitid)
{
	return units->at( unitid);
}

//void Update::equip_parse( string* equip, string* label, string
#endif
