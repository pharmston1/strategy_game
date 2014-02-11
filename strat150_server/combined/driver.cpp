//#include "Update.h"
#include <ctime>
#include <string.h>
#include <cstdlib>
#include <string>
#include <signal.h>
#include <fstream>
#include "support.h"
#include "orders.h"
#include "world.h"
#include "simulator.h"
//#include "Log.h"
//#include "Base64.h"

using namespace std;

void main_loop( Properties &);
int kbhit();

//Input buffer length
#define INPUT_BUFLEN 256

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#endif


//For the occasional insane segfault
void posix_death_signal(int signum)
{
    try{
    endwin();
    }
    catch(int e)
    {;} //Kill ncurses/pdcurses

    signal(signum, SIG_DFL); // signal resending
    cerr << "An error (" << signum << ") has occurred." << endl;
    cerr << "Current world log:" << endl << World::currentLog.str();

    exit(3);
}

//Constructs the digest file for a given userID
void createDigest( string userid, time_t tick, string json)
{
	stringstream str;
	str << tick;
	string path = TEMP_DIGEST + userid + "-" + str.str() + ".json";
	ofstream file(path.c_str());
	file << json;
	file.close();
	ifstream temp(path.c_str());
	string truepath = CONFIG_DIGEST + userid + "-" + str.str() + ".json";
	ofstream cpy(truepath.c_str());
	cpy << temp.rdbuf();
	temp.close();
	cpy.close();
}

//Test functions and scratch code
void testbench(Properties &prop)
{
    World world(prop);

	Player* testplayer = world.addPlayer( "test", "Agile");
	Player* testplayer2 = world.addPlayer( "test2", "Agile");

    OrdersLoader loader;
    loader.loadOrders(0, false);

    world.loadOrders(&loader);

	//cout << world.dumpLog(0);


//	testplayer->setAlly(testplayer2, true);
//	testplayer2->setAlly(testplayer, true);

	set<Player*> play;
	play.insert(testplayer);
	play.insert(testplayer2);

    Unit* u3  = world.createUnit( "AgileAPC", testplayer2);
    Unit* u2  = world.createUnit( "AgileMediumTank", testplayer);
	Unit* u  = world.createUnit( "AgileMediumTank", testplayer);

	u3->attach_child(world.createEquipment( "GeneralLight", testplayer2));
	world.log();

	//cout << world.serializeEvents() << endl;

	cout << world.masterSerialize( testplayer) << endl;
//	createDigest( testplayer->get_userid(), 7, world.masterSerialize( testplayer));
	/*
    world.moveUnit(u2, 5, 5);
    //u->setPos(1, 1);
    world.moveUnit(u, 1, 1);

    u3->setTrait("Damage", 10);
    u2->setTrait("Damage", 10);
    u->setTrait("Damage", 10);

	u->pushWaypoint(Vector2<double>(30, 4));
	u2->pushWaypoint(Vector2<double>(30, 4));
	u3->pushWaypoint(Vector2<double>(30, 4));

	world.advanceTick(time_t(), time_t() + 10);
    world.advanceTick(time_t()+1, time_t() + 10);
	cout << world.dumpLog(0);
	cout << endl << world.dumpLog(1);

//	Vector2<int> pos = world.getNearestOpen(Vector2<double>(1, 0));

//	cout << pos.x << ", " << pos.y << endl;

    return;
//	Update update_var(prop.rulesFile, prop.mapFile);


//	Player one("Test1", prop.balance);
//	Player two("Test2", prop.balance);
//	Player three("Test3", prop.balance);
//	map< string, Player*>* players;
//	players = update_var.get_players();
//	players->insert( pair< string, Player*>( "Test2", &two));
	//players->push_back(two);
	//players->push_back(three);

    //cout << endl << endl;

//    World world(prop);


	/*set < Actor*>* testset = testequip3->getNetwork( );
	set< Actor*>::iterator iter = testset->begin();
	while( iter != testset->end())
	{
		cout << (*iter)->getType() << endl;
		iter++;
	}*/
	//cout << testset->size() << endl;
	//cout << testunit->getDoubleTrait("Cost") << endl;

	//world.createEquipment( "GeneralLight", testplayer);

    //cout << "Equipment created.";
	/*Unit* testunit = world.createUnit( "AgileMediumTank", testplayer);
    cout << "Unit created.";
	Unit* testunit2 = world.createUnit( "AgileMediumTank", testplayer);
	world.moveUnit( testunit2, 10, 10);

	if( world.moveUnit( testunit, 10, 10))
		cout << "good to go partner"<<endl;
	else
		cout << "fail"<< endl;*/
    //world.addOrders(testOrder);

    //cout << world.dumpLog(0);

    //cout << "testOrder freed." << endl;



	//OrdersLoader *loader = new OrdersLoader;
	//loader->loadOrders(0, false);
	//cout << loader->getLog() << endl << endl << loader->getOrders().size();
    //delete loader;
}

///////////////////////////////////////////////////////////////
int main()
{
    //Register error handlers
    signal(SIGSEGV, posix_death_signal);
    signal(SIGILL, posix_death_signal);

	struct Properties prop = get_properties();
    //testbench(prop);
    main_loop( prop);
	return 0;
}

void update_tickfile( time_t x, unsigned int tick)
{
	time_t utcTime;
	utcTime = mktime(gmtime( &x));
	ofstream tickfile;
	tickfile.open(TICK_FILE);

	if( tickfile.is_open())
		tickfile << tick << " " << utcTime;

	tickfile.close();
}

/*!
    \fn string process_command(string cmd)
    \brief Processes console commands and calls appropriate subroutines.
    \param cmd Command string
    \return Response to console.

*/
string process_command(string cmd, Properties &prop, World &world)
{
	if(strContains(cmd, "clear"))
    {
        clear();
		return "";
    }

	stringstream str;
	if(strContains(cmd, "help"))
	{
		str << "-----Recognized Commands-----\n";
		str << "clear:		Clears the console screen\n";
		str << "players:	Returns a list of connected players.\n";
		str << "			NOTE: Adding '-c' returns the number\n";
		str << "			of players in addition to the list.\n";
		str << "count:		Returns number of players connected.\n";
		str << "info|map:	Returns current map information.\n";
		str << "quit:		Exits the console\n";
		return str.str();
	}

	if(strContains(cmd, "players"))
	{
		Base64 b64;
		str << "Current players connected: " << endl;
		map<string, Player*> players = world.getPlayers();
		map<string, Player*>::iterator pIter = players.begin();
		for(pIter; pIter != players.end(); pIter++)
		{
			str << "UserID:\t\t" << pIter->first << endl;
			str << "Nickname:\t" << b64.base64_decode(pIter->second->nickname) << endl;
			str << "Faction:\t"	 << pIter->second->getFaction() << endl;
		}
		if(strContains(cmd, "-c"))
			str << "Player Count: " << players.size();
		return str.str();
	}

	if(strContains(cmd, "count"))
	{
		str << "Player Count: " << world.getPlayers().size() << endl;
		return  str.str();
	}

    if(strContains(cmd, "info") || strContains(cmd, "map"))
    {
        str << "Map name: " << world.gameMap.getTitle() << endl;
        str << "Map file: " << prop.mapFile << endl;
        str << "Dimensions: " << world.gameMap.getWidth() << "w x " << world.gameMap.getHeight() << "h \n";
        str << "Tiles Types: " << world.gameMap.getTileSet().size() << endl;
        str << "Non-default tiles: " << world.gameMap.getTiles().size() << endl;
        str << "Description: " << world.gameMap.getDescription() << endl;
		return str.str();
    }

    return "Command not recognized.";
}

void main_loop( Properties &settings)
{
	initscr();
	time_t lastTimeStamp, nextTimeStamp, currentTime;
	char input[INPUT_BUFLEN];

	bool quit = FALSE;
	int ticknumber = 0;


	printw("strat150 Server\n  ");
	refresh();

	input[0] = '\0';
    int inputIndex = 0;

    World world(settings);
    //printw(world.dumpLog(0).c_str());
    refresh();

	queue<OrdersLoader*> olQueue;
	time_t lastCheck = 0;

    //Input loop
	while(!quit)
	{

        //printw("Tick %d", ticknumber);

		cout.flush();

        keypad(stdscr, TRUE);

        int maxx = getmaxx(stdscr);
        int maxy = getmaxy(stdscr);

        nextTimeStamp = time( &lastTimeStamp) + settings.tick_size;
		while (  time( &currentTime) <  nextTimeStamp && !quit)	// loop for time of tick
		{
			nodelay(stdscr, TRUE);
			scrollok(stdscr, TRUE);

			int x, y;
            getyx(stdscr, y, x);

            move(0, 0);
            long systime = static_cast<long>(time(0));
            long next = static_cast<long>(nextTimeStamp);
            long remaining = next - systime;

            printw("Tick %d : System time: %d %d %d [%d]       ", ticknumber, next, systime, remaining, settings.tick_size);

            move(y, 0);
            printw("> ");

            move(y, x);

            //Check for window resize

            int mx = getmaxx(stdscr);
            int my = getmaxy(stdscr);

            //Redraw window if X size has changed
            if(mx != maxx)
            {
                clear();
                printw("Tick %d\n%s", ticknumber, input);
            }

            maxx = mx;
            maxy = my;

			if ( kbhit())	// check if keyboard has been hit
			{

                int in = getch();

                if(in >= 32 && in <= 126)
                {

                   input[inputIndex++] = in;

                   if(inputIndex < INPUT_BUFLEN)
                        printw("%c", in, in);
                   else
                        inputIndex--;

                   input[inputIndex+1] = '\0';

                }

                if(in == KEY_BACKSPACE || in == 7 || in == 126) //Backspace
                {
                    inputIndex--;
                    if(x > 2)
                    {
                        mvaddch(y, x-1, ' ');
                        move(y, x-1);
                    }
                    else if(inputIndex > 0)
                    {

                        mvaddch(y-1, maxx-1, ' ');
                        move(y-1, maxx-1);
                    }

                    if(inputIndex < 0)
                        inputIndex = 0;

                    input[inputIndex] = '\0';
                }


                if(in == KEY_ENTER || in == 10) //Check if enter/return has been pressed
                {
                    printw("\n%s [%d]\n",input, inputIndex);

                    if( strcmp( input, "quit") == 0)	// compares the string
                    {
                        printw("\nQuitting...\n");
                        quit = TRUE;	// sets to quit to true

                    }
                    else
                    {
                        //quit = false;
                        string r = process_command(input, settings, world);
                        printw("\n%s\n  ", r.c_str());
                    }

                    //Reset input buffer upon entry
                    inputIndex = 0;
                    memset( input, 0, sizeof(input));
                    input[0] = 0;
				}
			}
			else
			{
				mySleep(1);
			}

			refresh();
			mySleep(10);

            //Check for new orders files every three seconds or so
			if(currentTime - lastCheck > ORDERS_INTERVAL)
			{
                //printw("Checking...\n");
				OrdersLoader* ol = new OrdersLoader;
				ol->loadOrders(lastCheck, true);

				if(ol->getOrders().size() > 0)
                {
                    printw(ol->getLog().c_str());
                    world.loadOrders(ol);
                    //printw(world.dumpLog(0).c_str());
                    olQueue.push(ol);
                    printw("\nLoaded %d orders files (%d total)...\n  %s",
                           ol->getOrders().size(), olQueue.size(), input);
                }
				else
                    delete ol;
				lastCheck = currentTime;

			}
		}

        //if(olQueue.size() > 0)
        //    printw("\nDeleting %d excess orders loaders\n  %s", olQueue.size(), input);

		while(!olQueue.empty())
		{
			delete olQueue.front();
			olQueue.pop();
		}


		printw("\nWorking...");	// where get orders and simulate function are

		ticknumber = world.advanceTick(currentTime, nextTimeStamp);
		update_tickfile(lastTimeStamp, ticknumber);
		printw("Log for tick %d\n%s\n", ticknumber,  (world.dumpLog((ticknumber)).c_str()));


		map<string, Player*> players = world.getPlayers();
		map<string, Player*>::iterator pIter = players.begin();
		while(pIter != players.end())
		{
            string userID = pIter->first;
            createDigest(userID, ticknumber, world.masterSerialize(pIter->second));
            pIter++;
		}

		refresh();
		//update_var.main_update( get_files( nextTimeStamp));

		//clear();
		printw("Tick %d complete.\n  %s", ticknumber, input);

	}

	endwin();
	mySleep( 10);
}

int kbhit()
{
	noecho();
    char ch = (char) getch();

    if (ch != ERR)
	{
        ungetch(ch);
        return 1;
    }
	else
        return 0;
}
