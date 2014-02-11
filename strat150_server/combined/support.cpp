#include "support.h"


void mySleep(int sleepMs)
{
#ifdef __linux__
    usleep(sleepMs * 1000);   // usleep takes sleep time in us
#endif
#ifdef WINDOWS
    Sleep(sleepMs);
#endif
}


/*!
	\brief This function checks for file existence

 */
bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile.good();
}

/*!
    \fn bool strContains(std::string str, const char* seq)
    \brief Returns true if the given substring is present
    \param str String to search
    \param seq Character sequence to search for
    \return TRUE if found, FALSE if not found

*/
bool strContains(std::string str, const char* seq)
{
    return str.find(seq) != string::npos;
}



	/*!
		\fn struct Properties get_properties()
		\brief retrieves property values from file
		\return the struct container
	*/

struct Properties get_properties()
{
	struct Properties servprop;
	std::string line, prop, value;
	ifstream myproperties ( CONFIG_PATH);

	if(myproperties.bad())
    {
        cout << "Cannot find " << CONFIG_PATH;
        exit(-1);
    }

	while( myproperties.good())
	{
		getline( myproperties, line);
		size_t a = line.find('=');

        //Skip to next iteration if there's nothing of interest here
		if(a == string::npos)
            continue;

		prop = line.substr( 0, a);
		value = line.substr( ++a);

        std::stringstream trimmer;
        string trValue = value;
        trimmer << trValue;
        trValue.clear();
        trimmer >> trValue;


		//cout << line << " [" << prop << "]" << endl;

        if(strContains(prop, "server_name"))
		{
			servprop.servername = trValue;
			//cout << "Server name: [" << trValue << "]" << endl;
		}
		else if(strContains(prop, "tick_time"))
		{
			servprop.tick_size = atoi(trValue.c_str());
			//cout << "Tick time: [" << servprop.tick_size << "]" << endl;
		}
		else if(strContains(prop, "keep_orders"))
		{
			if(strContains(trValue, "true"))
                servprop.keep_orders = 1;
			else
				servprop.keep_orders = 0;

            //cout << "Order retention: [" << servprop.keep_orders << "]" << endl;

		}
		else if(strContains(prop, "player_count"))
		{
			servprop.player_count = atoi(trValue.c_str());
		}
		else if(strContains(prop, "rules_file"))
			servprop.rulesFile = "../" + trValue;
        else if(strContains(prop, "map_file"))
			servprop.rulesFile = "../" + trValue;
		else if(strContains(prop, "starting_balance"))
			servprop.balance = atoi( trValue.c_str());

	}
	myproperties.close();

	return servprop;
}

