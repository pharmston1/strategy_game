#include "Units.h"

/*!
    \fn void Unit::setPos(int x, int y)
    \brief Sets unit coordinates.
    \param x X-coordinate
    \param y Y-coordinate
*/
void Unit::setPos(int x, int y)
{
    coord[0] = x;
    coord[1] = y;
}

///Returns true if the unit is on the map
bool Unit::onMap()
{
    return (coord[0] >= 0 && coord[1] >= 0);
}

///Gets the unit's current coordinates.
vector<int> Unit::getPos()
{
    return coord;
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

	coord.resize(2);
	coord[0] = -1;
	coord[1] = -1;
}

Unit::~Unit()
{
    delete modTraits;
}