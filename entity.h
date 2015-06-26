#ifndef ENTITY_H
#define ENTITY_H

#include <iostream>
#include <vector>

#include "Const.h"
#include "coord.h"

class C_Board;

class C_Entity
{
    public:
		C_Entity(C_Board* board_, int posX_=0, int posY_=0);
		
		virtual bool move(int, int);
		virtual bool push(int x_, int y_) { return move(x_, y_); }

		int getPosX() const { return pos.x; }
		int getPosY() const { return pos.y; }
		C_Coord getPos() const { return this->pos; }
		C_Coord getPrevPos() const { return this->prevPos; }

		virtual int getId() = 0;
		virtual char* getImage() = 0;
		virtual char getChar() = 0;
    protected:
        C_Coord pos;
        C_Coord prevPos;
		C_Board* board;
};

class C_Player : public C_Entity
{
    public:
		C_Player(C_Board* board_, int posX_=0, int posY_=0);
		bool move(int, int);
		bool canGoOver(int id);
		
		void sleep() { sleeping = true; }
		void wakeUp() { sleeping = false; }
		bool isSleeping() { return sleeping; }
		
		int getId() { return EID_PLAYER; }
		char* getImage() { return "tiles/player.png"; };
		char getChar() { return '@'; }
	private:
		bool sleeping;
};

class C_Block : public C_Entity
{
    public:
		C_Block(C_Board*, int, int);
		
		int getId() { return EID_BLOCK; }
		char* getImage() { return "tiles/block.png"; };
		char getChar() { return 'x'; }
};

class C_Goal : public C_Entity
{
	public:
		C_Goal(C_Board*, int, int);
		
		int getId() { return EID_GOAL; }
   		char* getImage() { return "tiles/goal.png"; };
		char getChar() { return 'G'; }
};

class C_Ball : public C_Entity
{
    public:
        C_Ball(C_Board*, int, int);

        int getId() { return EID_BALL; }
		char* getImage() { return "tiles/ball.png"; };
		char getChar() { return '0'; }
};


#endif
