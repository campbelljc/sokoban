#ifndef COORD_H
#define COORD_H

#include <iostream>

class C_Coord
{
    public:
        C_Coord();
        C_Coord(int x_, int y_);

        bool operator == (C_Coord);
        C_Coord operator + (C_Coord);
        C_Coord operator - (C_Coord);

        int x;
        int y;
};

std::ostream& operator << (std::ostream& os, const C_Coord& c);

class C_Rect
{
    public:
        C_Rect(int=0,int=0,int=0,int=0);
        int x;
        int y;
        int w;
        int h;
};

#endif
