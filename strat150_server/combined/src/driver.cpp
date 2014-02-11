//#include "Update.h"
#include <ctime>
#include <string.h>
#include <cstdlib>
#include <string>
#include "support.h"
#include "orders.h"
#include "world.h"
#include "simulator.h"
//#include "Log.h"

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

int main()
{
	struct Properties prop = get_properties();
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

	//cout << world.dumpLog(0);

	//Player* testplayer = world.addPlayer( "test", "Agile");
	//if(world.createEquipment( "GeneralLight", testplayer))
	//world.createUnit( "AgileMediumTank", testplayer);

    //Making testplayer

/*	Player* testplayer = world.addPlayer( "test", "Agile");

	cout << "Player created." << endl;
	world.createEquipment( "GeneralLight", testplayer);

    //cout << "Equipment created.";
	Unit* testunit = world.createUnit( "AgileMediumTank", testplayer);
    cout << "Unit created.";
	Unit* testunit2 = world.createUnit( "AgileMediumTank", testplayer);
	world.moveUnit( testunit2, 10, 10);

	if( world.moveUnit( testunit, 10, 10))
		cout << "good to go partner"<<endl;
	else
		cout << "fail"<< endl;
*/
    //world.addOrders(testOrder);

    //cout << world.dumpLog(0);

    //cout << "testOrder freed." << endl;



	//OrdersLoader *loader = new OrdersLoader;
	//loader->loadOrders(0, false);
	//cout << loader->getLog() << endl << endl << loader->getOrders().size();
    //delete loader;

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
    string r = "Command not recognized.";

    if(strContains(cmd, "clear"))
    {
        r = "";
        clear();
    }

    bool info = false;

    if(strContains(cmd, "info"))
    {
        info = true;

    }

    if(info || strContains(cmd, "map"))
    {
      std::stringstream str;
        str.clear();
        str.str(std::string());


        r = "Map name: " + world.gameMap.getTitle() + "\n";
        r += "Map file: " + prop.mapFile + "\n";

        str << world.gameMap.getWidth() << "w x " << world.gameMap.getHeight() << "h \n";
        r += "Dimensions: " + str.str();
        str.str(std::string());


        str << world.gameMap.getTileSet().size();
        r += "Tiles Types: " + str.str() + "\n";
        str.str(std::string());

        str << world.gameMap.getTiles().size();
        r += "Non-default tiles: " + str.str() + "\n";
        str.str(std::string());

        r += "Description: " + world.gameMap.getDescription() + "\n";

    }

    return r;

}

void main_loop( Properties &settings)
{
	initscr();
	time_t lastTimeStamp, nextTimeStamp, currentTime;
	char input[INPUT_BUFLEN];

	bool quit = FALSE;
	unsigned int ticknumber = 0;


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
				//echo();

                //cbreak();
                //getstr( input);

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
				ol->loadOrders(lastCheck, false);
				//printw(ol->getLog().c_str());
				if(ol->getOrders().size() > 0)
                {
                    world.loadOrders(ol);
                    printw(world.dumpLog(0).c_str());
                    olQueue.push(ol);
                    printw("\nLoaded %d orders files (%d total)...\n  %s",
                           ol->getOrders().size(), olQueue.size(), input);
                }
				else
                    delete ol;
				lastCheck = currentTime;

			}
		}

        if(olQueue.size() > 0)
            printw("\nDeleting %d excess orders loaders\n  %s", olQueue.size(), input);

		while(!olQueue.empty())
		{
			delete olQueue.front();
			olQueue.pop();
		}

		update_tickfile( lastTimeStamp, ++ticknumber);

		printw("\nWorking...");	// where get orders and simulate function are
		refresh();
		//update_var.main_update( get_files( nextTimeStamp));

		//clear();
		printw("tick %d complete.\n  %s", ticknumber, input);

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
