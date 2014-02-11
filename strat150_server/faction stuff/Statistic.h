#ifndef Statistic_h
#define Statistic_h

#include "rules.h"

class Statistic
{
public:
	Statistic( RulesContainer*);
	void increment( string);
private:
	map< string, unsigned int> unitTypesNumbers;	// keeps track of the type of the unit and the amount
	map< string, unsigned int> playerVictories;	// track the number of victories a player has

	RulesContainer *rules;
};

Statistic::Statistic( RulesContainer *myrules)
{
	rules = myrules;
	map< string, unsigned int>::iterator it = unitTypesNumbers.begin();
	for(; it != unitTypesNumbers.end(); it++)
	{
		set< string>::iterator it2 = rules->getGroupEntryKeys( "units").begin();
		for(; it2 != rules->getGroupEntryKeys( "units").end(); it2++)
		{
			(*it).first = (*it2).assign;
		}
		(*it).second = 0;
	}
}

void Statistic::increment( string typeofUnit)
{
	map< string, unsigned int>::iterator it = unitTypesNumbers.begin();
	for(; it != unitTypesNumbers.end(); it++)
	{
		if( typeofUnit == (*it).first)
			(*it).second++;
	}
}

#endif