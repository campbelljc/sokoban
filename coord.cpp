#include "coord.h"

C_Coord::C_Coord() : x(0), y(0) { }

C_Coord::C_Coord(int x_, int y_) : x(x_), y(y_) { }

bool C_Coord::operator == (C_Coord c)
{
    return (this->x == c.x && this->y == c.y);
}

C_Coord C_Coord::operator + (C_Coord c)
{
    return C_Coord(this->x+c.x, this->y+c.y);
}

C_Coord C_Coord::operator - (C_Coord c)
{
    return C_Coord(this->x-c.x, this->y-c.y);
}

std::ostream& operator << (std::ostream& os, const C_Coord& c)
{
	os << "(" << c.x << ", " << c.y << ")";
	return os;
}

C_Rect::C_Rect(int x_, int y_, int w_, int h_) : x(x_), y(y_), w(w_), h(h_) { }
