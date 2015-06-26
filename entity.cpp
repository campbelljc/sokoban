#include <algorithm>
#include "entity.h"
#include "g_Game.h"
#include "Const.h"
#include "game.h"

C_Entity::C_Entity(C_Board* board_, int posX_, int posY_) : board(board_), pos (posX_, posY_), prevPos(posX_, posY_) { }

/**
Returns: true if the entity moved
         false if the entity didn't move
**/
bool C_Entity::move(int x_, int y_)
{
	if (board->getEntityPtr(pos.x+x_,pos.y+y_) == 0)
	{ // If the square is empty, we can move into it.
		prevPos = pos;
		board->setEntityPtr(this, pos.x+x_, pos.y+y_);
		board->setEntityPtr(0, pos.x, pos.y);
		pos.x += x_;
		pos.y += y_;
		return true;
	}
	return false;
}

C_Player::C_Player(C_Board* board_, int posX_, int posY_) : C_Entity(board_, posX_, posY_), sleeping(false) { }

bool C_Player::move(int x_, int y_)
{	
	if (!board->inGrid(pos.x+x_, pos.y+y_)) return false;
	
	if (board == g_Game.getBoardPtr() && g_Game.done) return false; // can't move player if level done.

	switch (board->getEntityType(pos.x+x_, pos.y+y_))
	{
		case 0:
		{
			if (this->C_Entity::move(x_,y_))
			{ // the player moved into the empty space successfully.
				if (board == g_Game.getBoardPtr())
				{
					std::cout<<"Inc this board move\n";
					g_Game.totalMoves ++;
					g_Game.broadcastMove(prevPos, x_, y_);
					g_Game.redraw();
				}
				else
				{
					std::cout<<"Inc opponent board move\n";
					g_Game.totalOpponentMoves ++;
				}
				return true;
			}
		}
	    case EID_BALL:
	    {
            // the player is trying to move into a blackball.
            // therefore the player is trying to move it.

            //If the ball can be pushed in this direction
            if (board->getEntityPtr(pos.x+x_, pos.y+y_)->push(x_,y_))
            { // the blackball was pushed successfully.
                // now we can move the player into the blackball's original square.

				if (board == g_Game.getBoardPtr())
				{
					g_Game.broadcastMove(board->getEntityPtr(pos.x+x_+x_, pos.y+y_+y_)->getPrevPos(), x_, y_); // move ball
				}
				
                return this->move(x_,y_); // call to move player
            }
            return false;
	    }
	}

	std::cout << "Unknown object (" << board->getEntityType(pos.x+x_, pos.y+y_) << ") at " << pos.x+x_ << ", " << pos.y+y_ << "\n";
    return false;
}

C_Ball::C_Ball(C_Board* board_, int posX_, int posY_) : C_Entity(board_, posX_, posY_) { }

C_Block::C_Block(C_Board* board_, int posX_, int posY_) : C_Entity(board_, posX_, posY_) { }

C_Goal::C_Goal(C_Board* board_, int posX_, int posY_) : C_Entity(board_, posX_, posY_) { }