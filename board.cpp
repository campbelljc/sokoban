#include "board.h"
#include "entity.h"
#include "g_Game.h"

#include <vector>

C_Board::C_Board()
{
    //Assign 0 to the entire board
    for (int i = 0; i < this->MAX_WIDTH; i++)
    {
        for (int j = 0; j < this->MAX_HEIGHT; j++)
        {
            board[i][j] = 0;
			ground[i][j] = 0;
        }
    }
	timeToGenerate = 0;
}

C_Board::~C_Board()
{
    this->clear();
}

int C_Board::getEntityType(int x, int y)
{
    if (board[x][y] != NULL) return board[x][y]->getId();
    return 0;
}

bool C_Board::inGrid(int x, int y)
{
	return (x >= 0 && x < MAX_WIDTH && y >= 0 && y < MAX_HEIGHT);
}

void C_Board::clear()
{
    //deallocate memory
    for (int i = 0; i < this->entities.size(); i++)
    {
        delete this->entities[i];
    }

	timeToGenerate = 0;
	
    //empty vectors
    this->entities.clear();
    this->players.clear();
    this->balls.clear();
	this->blocks.clear();
	this->goals.clear();

    //clear board
    for (int i = 0; i < this->MAX_WIDTH; i++)
    {
        for (int j = 0; j < this->MAX_HEIGHT; j++)
        {
            board[i][j] = 0;
			ground[i][j] = 0;
        }
    }
}