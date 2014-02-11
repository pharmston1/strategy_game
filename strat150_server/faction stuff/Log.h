#ifndef Log_h
#define Log_h

#include <curses.h>
#include <fstream>

#define VECTOR_SIZE 50;

class Log
{
public:
	void output();
	void input();
private:
	vector< string> logStrings[VECTOR_SIZE];
};

void Log::output()
{
	initscr();
	echo();
	for( int i = 0; i < VECTOR_SIZE; i++)
	{
		printw( logStrings.at(i).c_str());
	}
	logStrings.erase( logStrings.begin(), logStrings.end());
}

#endif