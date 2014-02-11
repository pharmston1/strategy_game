#include <iostream>
#include <fstream>



#ifndef support_H
#define support_H
using namespace std;


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

#define _BITGRID_CELLTYPE unsigned int

/*!
    \brief This class houses a compact, memory-efficient two-dimensionary array of bits.
    \author Chris Livingston
 */
class Bitgrid
{
    private:
    _BITGRID_CELLTYPE *bits;

    unsigned int width, height, rWidth;

    //Accelerates checking
    unsigned long firstCell;
    unsigned long lastCell;
    unsigned long length;

    int cellsize;

    public:
    void clear()
    {
        for(unsigned long i = 0; i < length; i++)
            bits[i] = 0;

        //Not sane values, but the grid is empty
        firstCell = length - 1;
        lastCell = 0;
    }

    Bitgrid(unsigned int width, unsigned int height)
    {

        this->width = width;
        this->height = height;

        cellsize = sizeof(_BITGRID_CELLTYPE)*8;

        rWidth = width / cellsize + 1;

        length = (rWidth)*height + 1;

        bits = new _BITGRID_CELLTYPE[length];

        for(unsigned long i = 0; i < length; i++)
            bits[i] = 0;

        firstCell = length - 1;
        lastCell = 0;

        //cout << cellsize << " bits, " << length << " cells, " << rWidth << " wide" << endl;
        //cout << sizeof(_BITGRID_CELLTYPE) * length << endl;

    }

    ~Bitgrid()
    {
        //cout << "Freeing " << length*sizeof(_BITGRID_CELLTYPE) << " : " << length << endl;
        delete []bits;
    }


    ///Puts a boolean value in the grid.
    void put(bool v, unsigned int x, unsigned int y)
    {
        if(x < 0 || y < 0 || x >= width || y >= height) //Range checking
            return;

        int bpos = x % cellsize;
        _BITGRID_CELLTYPE mask = 1 << (bpos);
        unsigned long pos = (x/cellsize)+y*rWidth;

        //11111101111111 - set bit to 0
        if(!v)
        {
            mask = ~mask;
            bits[pos] &= mask;

        }
        else //00000010000000 - set bit to 1
        {
            bits[pos] |= mask;

        }

        if(pos > lastCell)
           lastCell = pos;

        if(pos < firstCell)
           firstCell = pos;

        //printf("\nbit %d (%d) at %d - F:%d L:%d\n", bpos,mask,pos,firstCell,lastCell);
    }

    ///Gets a boolean value from the grid
    bool get(unsigned int x, unsigned int y)
    {

        unsigned long pos = (x/cellsize)+y*rWidth;

        if(pos < firstCell || pos > lastCell)
        {
            return false;
        }

        int bpos = x % cellsize;
        _BITGRID_CELLTYPE mask = 1 << (bpos);

        return (bits[pos] & mask) > 0;
    }


    bool overlap(Bitgrid *grid)
    {
        if(width != grid->width && height != grid->height)
            return false;

        unsigned long i = 0;
        unsigned long j = 0;

        //Check general grid regions
        if(grid->firstCell < firstCell)
            i = grid->firstCell;
        else
            i = firstCell;

        if(grid->lastCell < lastCell)
            j = grid->lastCell;
        else
            j = lastCell;



        //Linear search
        for(i; i < length && i <= j; i++)
            if((grid->bits[i] & bits[i]) != 0)
        for(; i < length && i <= j; i++)
            if((grid->bits[i] & bits[i]) != 0)
                return true;

        return false;
    }

    ///Puts a pseudo-circle on the grid.
    void setCircle(int x0, int y0, int radius)
	{
		int r2 = radius * radius;
		for(int y=-radius; y<=radius; y++)
		{
			int y2 = y*y;

		    for(int x=-radius; x<=radius; x++)
		    {
		    	if(x*x+y2 < r2)
		            put(true, x0+x, y0+y);
		    }
		}
	}


};

#endif
