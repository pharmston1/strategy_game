#include "orders.h"



///Returns a text log of order parsing
string Orders::getLog()
{
    return parseLog.str();
}

///Returns TRUE if orders were successfully parsed.
bool Orders::isValid()
{
    return !invalid;
}

///Constructor - Loads and parses the given orders file
Orders::Orders(string path)
{
    orders_path = path;
    parseLog << "Loading orders: " << orders_path << endl;

    timeStamp = time_t();

    //RulesContainer uses the same topology, so we're sticking with it
    Rules::RulesContainer oc;

    //On failure to parse, mark orders as invalid and bail out
    if(!oc.loadJsonFile(path))
    {
        parseLog << "Parsing FAILED: Unable to parse file " << path << endl;
        invalid = true;
        return;
    }

    Rules::RulesEntry *infoBlock = oc.getEntry("", "Info");
    Rules::RulesEntry *messageBlock = oc.getEntry("", "Messages");
    Rules::RulesEntry *specialsBlock = oc.getEntry("", "Specials");
    Rules::RulesEntry *dropsBlock = oc.getEntry("", "Drop");
    Rules::RulesEntry *attachBlock = oc.getEntry("", "Attach");

    set<string> unitKeys = oc.getSingleEntryKeys();
    set<string> buyKeys = oc.getGroupEntryKeys("Buy");

    //Info block must be present
    if(infoBlock != NULL)
    {
        userID = infoBlock->getString("userID");

        //Validate userID against orders path
        if(userID.size() == 0 || strContains(userID, path.c_str()))
        {
            //userID mismatch - spoofing attempt?
            parseLog << "Parsing FAILED: Info block/filename userID mismatch [" << userID << "] in " << path << endl;
            invalid = true;
            return;
        }

        nickname = infoBlock->getString("Nickname");

        parseLog << "userID: " << userID << "  Base64 nick: [" << nickname << "]" << endl;

        //Color directive
        vector<double> cv = infoBlock->getDoubleArray("Color");
        if(cv.size() >= 3)
        {
            color[0] = min(cv[0], 1.0);
            color[1] = min(cv[1], 1.0);
            color[2] = min(cv[2], 1.0);

            parseLog << "Color: R" << color[0] << " G" << color[1] << " B" << color[2] << endl;
        }
        else //Default to white if invalid
        {
            color[0] = 1;
            color[1] = 1;
            color[2] = 1;
            parseLog << "No color/invalid color" << endl;
        }

        faction = infoBlock->getString("Faction");

        parseLog << "Declared faction: " << faction << endl;
    }
    else
    {
        parseLog << "Parsing FAILED: Missing Info block" << endl;
        invalid = true;
        return;
    }

    if(unitKeys.size() > 1)
    {
        set<string>::iterator unitIter = unitKeys.begin();

        while(unitIter != unitKeys.end())
        {
            Rules::RulesEntry *unitEntry = oc.getEntry("", *unitIter);
            string unitID = *unitIter;
            unitIter++;

            //Attempt to retrieve values
            vector<string> moves = unitEntry->getStringArray("Move");
            vector<string> targets = unitEntry->getStringArray("Target");
            string stance = unitEntry->getString("Stance");

            //Determine if this is NOT a unit order format
            if(moves.size() == 0 && targets.size() == 0 && stance.length() == 0)
                continue;

            UnitOrder *order = new UnitOrder;

            order->stance = stance;

            parseLog << " unitID[" << unitID << "]: ";

            //Traverse the move list
            //if(moves.size() > 0)
            for(unsigned int i = 0; i < moves.size(); i++)
            {
                istringstream movstr(moves[i]);

                Vector2<double> pos;
                pos.x = -1;
                pos.y = -1;

                if(movstr) movstr >> pos.x;
                if(movstr) movstr >> pos.y;

                if(pos.x < 0 || pos.y < 0)
                    continue;

                order->coords.push(pos);

                parseLog << "->" << pos.x << "," << pos.y << " ";

            }

            for(unsigned int i = 0; i < targets.size(); i++)
            {
                parseLog << " TRG[" << targets[i] << "]";
                order->target_ids.push_back(targets[i]);
            }

            if(stance.size() > 0)
            {
                order->stance = stance;
                parseLog << " - " << stance;
            }
            else order->stance = "Default";

            unitOrders[unitID] = order;

            parseLog << endl;
        }
    }

    //message block is optional///////////////////////////////////////////
    if(messageBlock != NULL)
    {
        set<string> recipients = messageBlock->getStringKeys();
        set<string>::iterator recipient = recipients.begin();

        while(recipient != recipients.end())
        {
            messages[*recipient] = messageBlock->getString(*recipient);
            parseLog << "From: " << userID << "  To: " << *recipient << "  Msg: " << messages[*recipient] << endl;
            recipient++;
        }
    }
    else parseLog << "No messages." << endl;

    //Drop block is optional//////////////////////////////////////////////
    if(dropsBlock != NULL)
    {
        set<string> dropKeys = dropsBlock->getDoubleArrayKeys();
        set<string>::iterator dropIter = dropKeys.begin();

        //Dereferenced dropIter is unitID/Type to drop
        while(dropIter != dropKeys.end())
        {
            Drop *drop = new Drop();
            drop->unitType = *dropIter;
            vector<double> params = dropsBlock->getDoubleArray(*dropIter);

            drop->quantity = params[0];
            drop->xpos = params[1];
            drop->ypos = params[2];

            dropIter++;

            parseLog << "Drop: " << drop->unitType << " at " << drop->xpos << ", " << drop->ypos << endl;

            drops.insert(drop);
        }
    }
    else parseLog << "No unit drops." << endl;

    //Specials block is optional////////////////////////////////////////////
    if(specialsBlock != NULL)
    {
        set<string> specialKeys = specialsBlock->getDoubleArrayKeys();
        set<string>::iterator specIter = specialKeys.begin();

        while(specIter != specialKeys.end())
        {
            specials[*specIter] = specialsBlock->getDoubleArray(*specIter);
            parseLog << "Special: " << *specIter << endl;
            specIter++;
        }
    }
    else parseLog << "No specials." << endl;

    //Non-buy order attachments//////////////////////////////////////////////
    if(attachBlock != NULL)
    {
        set<string> attachKeys = attachBlock->getStringKeys();
        set<string>::iterator attachIter = attachKeys.begin();

        while(attachIter != attachKeys.end())
        {
            Assembly *assembly = new Assembly;
            bool good = true;

            string parentKey = *attachIter;

            //This class does whitespace delineations
            istringstream iss(attachBlock->getString(*attachIter));

            string childKey;
            int childQty = 1;

            if(iss)
                iss >> childKey;
            else
                good = false;

            if(iss)
                iss >> childQty;


            //Spit out a warning if the attachment syntax is mauled
            if(good)
            {
                assembly->addNode(childQty, childKey, childKey);
                assembly->addLink(parentKey, childKey);
                parseLog << "Attach (" << childQty << ") " << childKey
                         << " to " << parentKey << endl;
                assemblies.insert(assembly);    //For later cleanup
            }
            else
            {
                parseLog << "WARNING:  Could not parse assembly [" << parentKey
                         << " : " << attachBlock->getString(*attachIter) << "]" << endl;
                delete assembly;
            }
            attachIter++;
        }
    }
    else parseLog << "No non-buy attachments." << endl;

    //Parse the buys group////////////////////////////////////////////
    if(buyKeys.size() > 0)
    {
        set<string>::iterator bkIter = buyKeys.begin();

        while(bkIter != buyKeys.end())
        {
            BuyInfo* buyInfo = new BuyInfo;

            string unitKey = oc.getString("Buy", *bkIter, "Unit");
            buyInfo->quantity = oc.getDouble("Buy", *bkIter, "Qty");

            //If some asinine parameter is passed, reset to 1
            if(buyInfo->quantity <= 0)
                buyInfo->quantity = 1;

            bool hasBase = false;

            //Add the unit type and the base node
            if(unitKey.length() > 1)
            {
                buyInfo->unitType = unitKey;
                buyInfo->assembly.addNode(1, "BASE", unitKey);
                hasBase = true;
            }

            parseLog << "Buy order [" << *bkIter << " : "
                     << buyInfo->unitType << "]" << endl;

            //Add equipment info
            vector<string> equipStrs = oc.getStringArray("Buy", *bkIter, "Equipment");
            vector<string>::iterator eqIter = equipStrs.begin();
            while(eqIter != equipStrs.end())
            {
                istringstream iss(*eqIter);
                bool good = true;
                string eqKey = "";
                string eqLabel = "NONE";
                int eqQty = 0;

                //If any of the components are missing, skip
                if(iss) iss >> eqKey;
                else good = false;

                if(iss) iss >> eqQty;
                else good = false;

                if(iss) iss >> eqLabel;
                //else good = false;

                if(!good)
                    parseLog << "WARNING:  Equipment [" << *eqIter << "] skipped" << endl;
                else
                {
                    parseLog << " " << eqKey << " (" << eqQty << ") " << eqLabel << endl;
                    buyInfo->assembly.addNode(eqQty, eqLabel, eqKey);
                    buyInfo->equip.insert(eqKey);
                }


                eqIter++;
            }

            //Add attachment params
            vector<string> equipAttachments = oc.getStringArray("Buy", *bkIter, "Attach");
            eqIter = equipAttachments.begin();
            while(eqIter != equipAttachments.end())
            {
                istringstream iss(*eqIter);
                bool good = true;
                string childLabel = "";
                string parentLabel = "";

                //If any of the components are missing, skip
                if(iss) iss >> childLabel;
                else good = false;

                if(iss) iss >> parentLabel;
                else good = false;

                if(!good)
                    parseLog << "WARNING:  Attachment parameter [" << *eqIter << "] skipped" << endl;
                else
                {
                    parseLog << " " << childLabel << " linked to " << parentLabel << endl;
                    buyInfo->assembly.addLink(parentLabel, childLabel);
                }


                eqIter++;
            }

            //Add drop params
            string dropParams = oc.getString("Buy", *bkIter, "Drop");
            istringstream iss(dropParams);

            int x = -1;
            int y = -1;

            if(iss) iss >> x;
            if(iss) iss >> y;

            if(x >= 0 && y >= 0 && hasBase)
            {
                //Drop *drop = new Drop;
                buyInfo->drop.quantity = buyInfo->quantity;

                buyInfo->drop.xpos = x;
                buyInfo->drop.ypos = y;
                buyInfo->drop.unitType = buyInfo->unitType;

                parseLog << " Drop purchased units to " << x << ", " << y << endl;

                //drops.insert(drop);
            }
            else parseLog << " Unit will be sent to inventory." << endl;

            buys.insert(buyInfo);

            bkIter++;
        }
    }
    else parseLog << "No buy orders." << endl;

    parseLog << "Parsing complete." << endl;
    invalid = false;
}

///Erases all internal sub-orders and buy requests
Orders::~Orders()
{
    map<string, UnitOrder*>::iterator unitIter = unitOrders.begin();
    set<BuyInfo*>::iterator buyIter = buys.begin();
    set<Drop*>::iterator dropIter = drops.begin();
    set<Assembly*>::iterator attachIter = assemblies.begin();

    while(unitIter != unitOrders.end()) //Erase unit orders
    {
        delete unitIter->second;
        unitIter++;
    }

    while(buyIter != buys.end())    //Erase buy orders
    {
        delete *buyIter;
        buyIter++;
    }

    while(dropIter != drops.end())  //Erase drops
    {
        delete *dropIter;
        dropIter++;
    }

    while(attachIter != assemblies.end())   //Erase non-buy assemblies
    {
        delete *attachIter;
        attachIter++;
    }

    //done
}

std::string Orders::getUserID()
{
    return userID;
}


///Constructor
OrdersLoader::OrdersLoader()
{
}

///Destructor
OrdersLoader::~OrdersLoader()
{
	vector<Orders*>::iterator ordersIter = ov.begin();
	while(ordersIter != ov.end()){
		delete *ordersIter;
		ordersIter++;
	}
}

vector<Orders*> OrdersLoader::getOrders()
{
	return ov;
}

///Loads orders from the target directory
bool OrdersLoader::loadOrders(time_t stamp, bool purge)
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	dir = opendir(CONFIG_ORDERS);
	notlog << "Trying to open " << CONFIG_ORDERS << endl;
	if(dir != NULL)
	{
		while(true)
		{
			entry = readdir(dir);
			if(entry != NULL)
			{
				try
				{
					if(shouldAdd(entry->d_name, stamp))
					{
						addOrder(entry->d_name, purge);
					}
				}
				catch(int e)
				{
					notlog << "ERROR:  Exception occurred while parsing " << entry->d_name << endl;
				}
			}
			else break;
		}
		closedir(dir);
		notlog << "Successfully loaded orders" << endl;
		return true;
	}
	notlog << "Could not open directory..." << endl;
	return false;
}

bool OrdersLoader::shouldAdd(string filename, time_t stamp)
{
	//Makes sure that it is a JSON file
	if(filename.find(".json") == string::npos)	return false;
	//Removes ".json" and "uniqID-", leaving timestamp
	size_t pos = filename.find("-");
	filename = filename.substr(pos + 1, filename.length() - pos - 6);
	//Compares timestamp to time wanted
	stringstream str;
	str << stamp;
	return (filename >= str.str());
}

void OrdersLoader::addOrder(string filename, bool purge)
{
	notlog << "Orders file found: " << filename << endl;
	Orders* newOrder = new Orders(CONFIG_ORDERS + filename);
	notlog << "Trying to create Orders pointer for " << filename << endl;
	if(newOrder->isValid())
	{
		ov.push_back(newOrder);
		notlog << "Created pointer for " << filename << endl;
		if(purge) deleteFile(filename);
		notlog << endl;
	}
	else
	{
		notlog << newOrder->getLog() << endl;
		delete newOrder;
	}
}

void OrdersLoader::deleteFile(string filename)
{
	notlog << "Attempting to delete " << filename << endl;
	if(remove((CONFIG_ORDERS + filename).c_str()) == 0)
		notlog << "Successfully deleted " << filename << endl;
	else
		notlog << "Could not delete " << filename << endl;
}

string OrdersLoader::getLog()
{
	return notlog.str();
}

