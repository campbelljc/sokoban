#ifndef BOARD_H
#define BOARD_H

//Prevents out-of-bounds checking by boost::multi_array
#define BOOST_DISABLE_ASSERTS

#include <vector>
#include <iostream>
#include <string>

#include "coord.h"
#include "entity.h"
#include "solver.h"

#include <boost/multi_array.hpp>

class C_Entity;
class C_Player;
class C_Ball;
class C_Block;
class C_Goal;

/**
Description: Wrapper class for the board.
**/
class C_Board
{
    public:
        C_Board();
        ~C_Board();

        static const int MAX_WIDTH = 32;  //Board Width (tiles)
        static const int MAX_HEIGHT = 24; //Board Height (tiles)
		static const int TWIDTH = 32;  //Tile Width (pixels)
		static const int THEIGHT = 32; //Tile Height (pixels)

        C_Entity *board[MAX_WIDTH][MAX_HEIGHT];
		C_Entity *ground[MAX_WIDTH][MAX_HEIGHT];
		
		C_Entity *getEntityPtr(int x, int y) { return this->board[x][y]; }
		void setEntityPtr(C_Entity* entity, int x, int y) { board[x][y] = entity; }
		int getEntityType(int x, int y);

		C_Entity *getEntityPtrG(int x, int y) { return this->ground[x][y]; }
		void setEntityPtrG(C_Entity* entity, int x, int y) { ground[x][y] = entity; }
		int getEntityTypeG(int x, int y);

        C_Player* getPlayerPtr() { return players[0]; }
		bool inGrid(int x, int y);

        void clear();

		friend class C_Game;
		friend class C_Generator;
		friend class C_Graphics;
		friend class C_FileIO;
		
		C_Solver solution;
		
		int timeToGenerate;

    private:
        std::vector<C_Entity*> entities;
        std::vector<C_Player*> players;
        std::vector<C_Ball*> balls;
		std::vector<C_Block*> blocks;
		std::vector<C_Goal*> goals;
};

#endif
