#ifndef Files_H
#define Files_H
#include "Jzon.h"

#ifdef _WIN32
#include "dirent.h"
#endif

#ifdef __linux__
#include <dirent.h>
#endif

#include <set>
#include <queue>
#include <curses.h> //Uses PDCurses Win32a SDL layer in Windows
#include <queue>
#include <map>
#include <iostream>
#include <stdio.h>
#include <ctime>


#include "support.h"
#include "vmath.h"
#include "rules.h"
//#include "Paths.h"


/*! \file */

using namespace Jzon;
using namespace std;

/// Prototypes

bool delete_orders( string);

/*!
    \class Orders
    \brief Container class for all player-submitted orders.
    \author Chris Livingston
*/
class Orders
{
public:
    friend class World;
    struct UnitOrder
    {
        vector <string> target_ids;
        queue< Vector2<double> > coords;
        string stance;

        UnitOrder()
        {
            stance = "Default";
        }

    };

    struct Assembly;

    ///Individual unit/equipment key node.
    struct AttachNode
    {
        AttachNode* parent;
        int quantity;
        set<AttachNode*> attached;
        string tag;  //Can be either an actorID or unit/equip type
        string label; //Loaded from rules

        AttachNode(int qty, string tagName, string labelName)
        {
            parent = NULL;
            quantity = qty;
            tag = tagName;
            label = labelName;
        }
    };

    ///Internal class to represent part assemblies - may or may not be contiguous
    struct Assembly
    {
        int quantity;

        map<string, AttachNode*> nodes; //Keyed by label

        ///Adds a new node - this struct handles cleanup of created nodes
        void addNode(int qty, string label, string tag)
        {
            AttachNode* node = new AttachNode(qty, tag, label);
            nodes[label] = node;
        }

        ///Links existing nodes - if either key is not found, dummy entries are created!
        void addLink(string parentKey, string childKey)
        {
            AttachNode *parent, *child;
            map<string, AttachNode*>::iterator iter;

            iter = nodes.find(parentKey);
            if(iter == nodes.end()) //Parent not found, create a dummy entry
            {
                parent = new AttachNode(1, parentKey, parentKey);
                nodes[parentKey] = parent;
            }
            else
                parent = iter->second;

            iter = nodes.find(childKey);
            if(iter == nodes.end()) //Child not found, create a dummy entry
            {
                child = new AttachNode(1, childKey, childKey);
                nodes[childKey] = child;
            }
            else
                child = iter->second;

            //Now link the two
            child->parent = parent;
            parent->attached.insert(child);

        }

        ~Assembly()
        {
            map<string, AttachNode*>::iterator iter = nodes.begin();

            while(iter != nodes.end())
            {
                delete iter->second;
                iter++;
            }
        }
    };


    struct Drop
    {
        int quantity;
        string unitType;
        int xpos, ypos;

        Drop() //Init with invalid values
        {
            quantity = 0;
            xpos = -1;
            ypos = -1;
        }

    };

    struct BuyInfo
    {
        int quantity;
        string unitType;
        set< string> equip;

        Assembly assembly;
        Drop drop;

        BuyInfo()
        {

            unitType = "none";
            quantity = 1;
        }
    };



private:
    stringstream parseLog;
    int tick;

    map<string, UnitOrder*> unitOrders;    //Keyed by unitID
    set<Assembly*> assemblies;                   //Sets of attachments not found in buy orders
    set<BuyInfo*> buys;                    //No need for a key
    set<Drop*> drops;                      //No need for a key
    map<string, string> messages;          //To, playerID
    map<string, vector<double> > specials; //Keyed by specialID

    time_t timeStamp;
    string userID;
    string faction;
    string nickname;

    bool invalid; //Flag raised if orders fail to parse
    string orders_path; //For when we're done

public:
    Orders(string path);
    ~Orders();
    double color[3]; //RGB

    std::string getUserID();
    void dispose();
    bool isValid();
    std::string getLog();
};


class OrdersLoader
{
private:
	vector<Orders*> ov; //Orders vector
	//time_t lastStamp;
	//time_t currStamp;
	//time_t nextStamp;
	stringstream notlog;
	bool shouldAdd(string filename, time_t stamp);
	void addOrder(string filename, bool purge);
	void deleteFile(string filename);

public:
	OrdersLoader();
	~OrdersLoader();
	vector<Orders*> getOrders();
	bool loadOrders(time_t stamp, bool purge);
	string getLog();
};

#endif
