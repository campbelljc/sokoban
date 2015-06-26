#include "game.h"
#include "Const.h"
#include "g_Game.h"
#include "board.h"
#include <thread>

C_Game::C_Game() : state(STATE_MENU), isNetworkGame(false), totalOpponentMoves(0), totalMoves(0), oppGameFinished(false), done(false), chosenBoxLines(7), chosenNumGoals(2) { }

int C_Game::init()
{
    if (!fio.init()) return 0;
    if (!gfx.init()) return 0; //Initiates graphics
    if (!network.init()) return 0; //Initiates input

	setState(STATE_MENU); //Starts in the menu
	
	width = generator.getWidth() + 2;
	height = generator.getHeight() + 2;

	refreshWindowCaption();
	
    return 1;
}

void C_Game::addSize(int s)
{
	generator.addSize(s);
	width = generator.getWidth() + 2;
	height = generator.getHeight() + 2;
}

void C_Game::checkForCompletion()
{
	bool goalsDone = true;
	if (!done)
	{
		for (int count = 0; count < board.goals.size(); count ++)
		{
			if (board.getEntityType(board.goals.at(count)->getPosX(), board.goals.at(count)->getPosY()) != EID_BALL)
			{
				goalsDone = false;
			}
		}
		if (goalsDone)
		{
	//		std::cout<<"THIS GAME FINISHED\n";
			done = true;
		}		
	}

	bool oppGoalsDone = true;
	if (isNetworkGame && !oppGameFinished)
	{
		for (int count = 0; count < opponentBoard.goals.size(); count ++)
		{
			if (opponentBoard.getEntityType(opponentBoard.goals.at(count)->getPosX(), opponentBoard.goals.at(count)->getPosY()) != EID_BALL)
			{
				oppGoalsDone = false;
			}
		}
		if (oppGoalsDone)
		{
	//		std::cout<<"OPP GAME FINISHED!\n";
			oppGameFinished = true;
		}
	}
	
	if ((!isNetworkGame && done) || (isNetworkGame && done && oppGameFinished))
	{
		if (isNetworkGame)
			std::this_thread::sleep_for( std::chrono::milliseconds( 2500 ) );
		std::cout<<"Both games done: game over!\n";
		std::cout<<"Your total moves:"<<g_Game.totalMoves<<" and opp's:"<<g_Game.totalOpponentMoves<<"\n";
		setState(STATE_ENDSCREEN);
	}
}

void C_Game::hostGame()
{
	int difficulty = DIFF_EASY;
	network.hostGame(difficulty);
	
	//std::cout<<"client connected to our game\n";
	
	setupNetworkGame(difficulty);
}

void C_Game::joinGame()
{
	C_NetGame* game = network.getNextAvailableGame();
	if (game == NULL)
	{
		std::cout<<"No available game.\n";
		return;
	}
	network.joinGame(game);
//	std::cout<<"we joined a game, diff(2): "<< game->difficulty<<"\n";
	
	setupNetworkGame(game->difficulty);
}

void C_Game::setupNetworkGame(int difficulty)
{
	int numGoals, minBoxLines, maxBoxLines, solLength;
	isNetworkGame = true;
	switch(difficulty)
	{
		case DIFF_VERYEASY:
		{
			numGoals = 1;
			minBoxLines = 1;
			maxBoxLines = 1;
			solLength = 4;
			break;
		}
		case DIFF_EASY:
		{
			numGoals = 1;
			minBoxLines = 3;
			maxBoxLines = 5;
			solLength = 10;
			break;
		}
		case DIFF_AVERAGE:
		{
			numGoals = 2;
			minBoxLines = 6;
			maxBoxLines = 8;
			solLength = 34;
			break;
		}
		case DIFF_HARD:
		{
			numGoals = 3;
			minBoxLines = 8;
			maxBoxLines = 10;
			solLength = 30;
			break;
		}
		case DIFF_VERYHARD:
		{
			numGoals = 4;
			minBoxLines = 10;
			maxBoxLines = 13;
			solLength = 35;
			break;
		}
	}
	
	std::thread waitForPeerGeneration(C_Network::staticRcvLevel, &network);
	
	std::cout<<"Gen'ing with numgoals:"<<numGoals<<"\n";
	generator.generateLevel(false, minBoxLines, maxBoxLines, numGoals, solLength);
	
	// done generating, so tell peer.
	network.sendLevelToPeer();
	
//	std::cout<<"waiting for peer to generate\n";
	
	waitForPeerGeneration.join();

//	std::cout<<"peer done gen\n";
	
	oppGameFinished = false;
	done = false;
	
	setState(STATE_PLAY);
	rcvMoveThread = std::thread(C_Network::staticRcvMoves, &network);
	bcastMoveThread = std::thread(C_Network::staticBcastMove, &network);
}

void C_Game::broadcastMove(C_Coord pos, int x_, int y_)
{
	movesToBroadcast.push(C_Move(pos, x_, y_));
}

void C_Game::continuallyGenerateLevels()
{
	time_t now = time(NULL);
	while(difftime(time(NULL), now) <= 60*33)
	{ // TODO: loop doesn't work
		generator.generateLevel(true, 0, 10000, g_Game.chosenNumGoals);
	}
}

void C_Game::setState(int state_)
{
    state = state_;
}

bool C_Game::inRect(C_Coord mousePos, std::string path, int midXPos, int topYPos)
{
	path = "images32/" + path;
	C_Size imageSize = C_Graphics::getImageSize(path);
	std::cout<<"path2:"<<path<<"\n";
	
	C_Rect dstRect(midXPos-(imageSize.w/2), topYPos, imageSize.w, imageSize.h);
	if (midXPos < (imageSize.w/2)) dstRect.x = midXPos;
	else if (midXPos == SWIDTH) dstRect.x = midXPos - imageSize.w;
	if (topYPos > SHEIGHT-imageSize.h) dstRect.y = topYPos - imageSize.h;
	
	return this->inRect(mousePos, dstRect.x, dstRect.y, dstRect.x+dstRect.w, dstRect.y+dstRect.h);
}

void C_Game::refreshWindowCaption()
{
    switch (state)
    {
        case STATE_PLAY:
		{
			std::string title = "PSG - Boxlines: " + toString(board.solution.numBoxLines) + ", length: " + toString(board.solution.length) + ", gentime: " + toString(board.timeToGenerate);

			gfx.setWindowTitle(title);

			break;
		}
		case STATE_ENDSCREEN:
        case STATE_MENU:
		{
			std::string title = "PSG. Boxlines: " + toString(g_Game.chosenBoxLines) +"-" + toString(g_Game.chosenBoxLines+2) + ", num goals: " + toString(g_Game.chosenNumGoals) + ", size: " + toString(g_Game.width);

			gfx.setWindowTitle(title);

			break;
		}
    }
}

void C_Game::redraw(C_Entity* ent)
{
	if (ent == 0)
	{
	    gfx.drawGame();
	    return;
	}

	gfx.animate(ent);
}

void C_Game::delay(int ms)
{
	SDL_Delay(ms);
}
