#include <time.h>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include <boost/random/random_device.hpp>

#include "generator.h"
#include "solver.h"
#include "g_Game.h"

typedef boost::uniform_int<> NumberDistribution; // choose a distribution
typedef boost::mt19937 RandomNumberGenerator;    // pick the random number generator method,
typedef boost::variate_generator< RandomNumberGenerator&, NumberDistribution > Generator;  // link the generator to the distribution

boost::random_device dev;

RandomNumberGenerator C_Generator::generator(dev);

int timeInMilli()
{ // src http://stackoverflow.com/questions/19423628/get-time-difference-in-milliseconds-in-c
#ifdef WIN32
	return 0; // didn't have time to search for win compatible version
#else
  timeval t;
  gettimeofday(&t, NULL);

  std::string buf(20, '\0');
  strftime(&buf[0], buf.size(), "%H:%M:%S:", localtime(&t.tv_sec));
  std::string str_hr  = buf.substr(0, 2);
  std::string str_min = buf.substr(3, 2);
  std::string str_sec = buf.substr(6, 2);

  int hr    = atoi(str_hr.c_str());
  int min   = atoi(str_min.c_str());
  int sec   = atoi(str_sec.c_str());
  int milli = t.tv_usec/1000;

  int timeInMilli = (((hr * 60) + min) * 60 + sec) * 1000 + milli;
  return timeInMilli;
#endif
}

C_Generator::C_Generator() : numGoals(0), minBoxLines(0), maxBoxLines(0), length(0), width(6), height(6), patterns(NULL)
{
	this->board = g_Game.getBoardPtr();
}

int startRoomTime, endRoomTime;
int startBallTime, endBallTime;

void C_Generator::generateLevel(bool continuous, int minBoxLines_, int maxBoxLines_, int numGoals_, int length_)
{
	this->numGoals = numGoals_;
	this->minBoxLines = minBoxLines_;
	this->maxBoxLines = maxBoxLines_;
	this->length = length_;
	
	for (int count = 0; count < numGoals; count ++)
	{
		while (!ballSpots[count].empty())
		{ // clear the vector
			ballSpots[count].pop();
		}
	}
	
	if (patterns == NULL)
	{
		patterns = new int*[width];
	    for (int i = 0; i < width; i++)
	    {
			patterns[i] = new int[height];
	    }
	}
	
	while(true)
	{
		startRoomTime = timeInMilli();
		do
		{
			board->clear();
			goalPositions.clear();
			previousPlayerPositions.clear();
		} while (!tryToGenerate());		

		endRoomTime = timeInMilli();
		int numTrialsWithPlayer;
		bool gen = false;
		for (numTrialsWithPlayer = 0; numTrialsWithPlayer < 2; numTrialsWithPlayer ++)
		{
			placePlayer();
			
			int trial;
			bool succeeded = false;
			for (trial = 0; trial < 20; trial ++)
			{
				startBallTime = timeInMilli();
				
				for (int count = 0; count < numGoals; count ++)
				{ // remove all goals, if placed
					removeGoal();
				}
			
				for (int count = 0; count < numGoals; count ++)
				{ // place all goals
					placeGoal();
				}
				
				for (int count = 0; count < numGoals; count ++)
				{ // get all available spots that each ball can be placed in.
					getAvailableSpotsForBall(count);
				}
	
				bool allBallsPlaced = placeBall(0, continuous); // recursive function to place all balls.
				if (allBallsPlaced)
				{ // done!
					endBallTime = timeInMilli();
					board->timeToGenerate = (endRoomTime - startRoomTime) + (endBallTime - startBallTime);
					gen = true;
					succeeded = true;
					break;
				}		
			}
			
			if (trial >= 20 && !succeeded)
			{ // we couldnt generate a goal-ball placement for numGoals amount of goals, so re-generate the room shape.
				removePlayer();
				for (int count = 0; count < numGoals; count ++)
				{ // remove all goals
					removeGoal();
				}
				continue;
			}
			
			break; // done
		}
				
		if (gen) break;
	}
	
	g_Game.saveLevel();
	
	// transfer all goals to the ground, so that players/balls can stand on them.
	for (int i = 0; i < g_Game.width; i ++)
	{
		for (int j = 0; j < g_Game.height; j ++)
		{
			if (board->board[i][j] != 0 && board->board[i][j]->getId() == EID_GOAL)
			{
				board->ground[i][j] = board->board[i][j];
				board->board[i][j] = 0;
			}
		}
	}
}

bool C_Generator::tryToGenerate()
{
#ifdef WIN32
	  SDL_Event event; // for performance reasons
	  SDL_PollEvent(&event);
#endif
	
	NumberDistribution distribution( 0, 16 );	

	Generator randPattern(generator, distribution);

	NumberDistribution flipChance( 0, 1 );
	Generator flipGen(generator, flipChance);

	NumberDistribution rotationChance( 0, 4 );
	Generator rotationGen(generator, rotationChance);

	int tempBoard[width][height];
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            tempBoard[i][j] = -1;
            patterns[i][j] = -1;
        }
    }
	
	int roomTemplates[17][5][5] = { { {-1, -1, -1, -1, -1}, {-1, 0, 0, 0, -1}, {-1, 0, 0, 0, -1}, {-1, 0, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, 0, 0, -1}, {-1, 0, 0, 0, -1}, {-1, 0, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, 0, 0}, {-1, EID_BLOCK, EID_BLOCK, 0, 0}, {-1, 0, 0, 0, -1}, {-1, 0, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, 0, 0, 0, -1}, {-1, 0, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, EID_BLOCK, 0, 0, -1}, {-1, EID_BLOCK, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, 0, -1, -1}, {-1, EID_BLOCK, 0, 0, -1}, {0, 0, 0, 0, -1}, {-1, 0, 0, EID_BLOCK, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, 0, 0, -1}, {0, 0, 0, 0, -1}, {-1, EID_BLOCK, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, 0, -1, -1}, {-1, EID_BLOCK, 0, 0, -1}, {0, 0, 0, 0, -1}, {-1, EID_BLOCK, 0, EID_BLOCK, -1}, {-1, -1, 0, -1, -1} },
						  { {-1, -1, 0, -1, -1}, {-1, EID_BLOCK, 0, EID_BLOCK, -1}, {0, 0, 0, 0, -1}, {-1, EID_BLOCK, 0, EID_BLOCK, -1}, {-1, -1, 0, -1, -1} },
	/*connectivity excep*/{ {-1, -1, 0, -1, -1}, {-1, EID_BLOCK, 0, EID_BLOCK, -1}, {-1, EID_BLOCK, 0, 0, 0}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {0, 0, 0, 0, 0}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, 0, 0, 0, 0}, {-1, 0, EID_BLOCK, 0, 0}, {-1, 0, 0, 0, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, -1, -1, -1, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, EID_BLOCK, 0, 0, -1}, {0, 0, 0, 0, -1}, {0, 0, -1, -1, -1} },
						  { {-1, 0, -1, 0, -1}, {-1, 0, 0, 0, -1}, {-1, EID_BLOCK, 0, EID_BLOCK, -1}, {-1, 0, 0, 0, -1}, {-1, 0, -1, 0, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {-1, 0, 0, 0, -1}, {-1, 0, 0, 0, -1} },
						  { {-1, -1, -1, -1, -1}, {-1, EID_BLOCK, EID_BLOCK, EID_BLOCK, -1}, {0, 0, EID_BLOCK, 0, 0}, {-1, 0, 0, 0, -1}, {-1, 0, 0, -1, -1} } };

	// generate empty room.
	for (int rowCount = 0; rowCount < width; rowCount += 3)
	{
		for (int colCount = 0; colCount < height; colCount += 3)
		{
			int trials;
			for (trials = 0; trials < 100000; trials ++)
			{
				int chosenPattern[5][5];
				int num = randPattern();
				for (int row = 0; row < 5; row ++)
				{
					for (int col = 0; col < 5; col ++)
					{
						chosenPattern[row][col] = roomTemplates[num][row][col];
					}
				}
			
				// random flip chance
				if (flipGen() == 1)
				{ // src : http://www.smotricz.com/lore/array_flipping.htm
			        int temp, r, c;
					temp = r = c = 0;
			        for (r=0; r<5; r++) {
			          for (c=0; c<5-r; c++) {
						temp = chosenPattern[r][c];
			            chosenPattern[r][c] = chosenPattern[c][r];
			            chosenPattern[c][r] = temp;						
			          }
			        }
				}
			
				int numRotations = rotationGen();
				for (int i = 0; i < numRotations; i ++)
				{ // src : http://stackoverflow.com/questions/16684856/rotating-a-2d-pixel-array-by-90-degrees
					int oldArray[5][5];
					for (int row = 0; row < 5; row ++) { for (int col = 0; col < 5; col ++) { oldArray[row][col] = chosenPattern[row][col]; }}
	
					for(int i=0; i<5; i++) {
					    for(int j=0; j<5; j++) {
					        chosenPattern[i][j] = oldArray[5-1-j][i];
					    }
					}
				}
			
				// verify that the pattern would work!
				bool badOverlap = false;
				int x, y; x = y = 0;
				for (int row = rowCount - 1; row < rowCount + 4; row ++)
				{
					y = 0;
					if (row < 0 || row >= width) continue;
					for (int col = colCount - 1; col < colCount + 4; col ++)
					{
						if (col < 0 || col >= height) continue;
						if (tempBoard[row][col] == -1 || chosenPattern[x][y] == -1) { y++; continue; } // -1 always matches
						if (tempBoard[row][col] != chosenPattern[x][y])
						{ // if there is already something at board[i][j], and it does not match with the pattern,
							badOverlap = true; // then abandon this attempt
							break;
						}
						y++;
					}
					if (badOverlap) break;
					x++;
				}
				if (badOverlap) continue; // try another pattern if this one didn't fit.
			
				// no overlap, so let's use it!
				x = y = 0;
				for (int row = rowCount - 1; row < rowCount + 4; row ++)
				{
					y = 0;
					if (row < 0 || row >= width) continue;
					for (int col = colCount - 1; col < colCount + 4; col ++)
					{
						if (col < 0 || col >= height) continue;
						if (chosenPattern[x][y] != -1)
						{
							tempBoard[row][col] = chosenPattern[x][y];
							patterns[row][col] = num;
							if (num == 9 && x == 2 && y == 2)
							{
								patterns[row][col] = 1000;
							}
						}
						y ++;
					}
					x ++;
				}
				
				break; // done this grid, so get out of the trial loop and proceed to the next 3x3 board area.
			}
			if (trials == 1000) { std::cout << "too many trials\n"; return false; }
		}
	}
	
	// translate board into C_Board class.
	board->clear();
    for (int i = 0; i < width + 2; i++)
    {
        for (int j = 0; j < height + 2; j++)
        {
			if (i == width+1 || j == height+1 || i == 0 || j == 0)
			{
				board->board[i][j] = new C_Block(board, i, j);
				board->entities.push_back(board->board[i][j]);
				board->blocks.push_back((C_Block*)board->board[i][j]);
				continue;
			}
			switch(tempBoard[i][j])
			{
				case 0:
				{
					g_Game.setEntityPtr(0, i+1, j+1);
					break;
				}
				case -1:
				case EID_BLOCK:
				{
					board->board[i+1][j+1] = new C_Block(board, i+1, j+1);
					board->entities.push_back(board->board[i+1][j+1]);
					board->blocks.push_back((C_Block*)board->board[i+1][j+1]);
					break;
				}
			}
		}
	}
	
	g_Game.redraw();
						
	if (!checkForConnectivity(width, height))
	{
	//	std::cout<<"NO connection\n";
		return false;
	}
	
	if (!checkForEnoughSpace(width, height))
	{
	//	std::cout<<"not enough space\n";
		return false;
	}
	
	if (checkForThreeWalls(width, height))
	{
	//	std::cout<<"has an empty square surrounded by 3 walls.\n";
		return false;
	}
		
	if (checkForLargeSpace(width+1, height+1))
	{
	//	std::cout<<"Large space detected.\n";
		return false;
	}
	
	g_Game.redraw();
	
	return true;
}

bool C_Generator::checkForEnoughSpace(int width, int height)
{ // check if there is an appropriate amount of empty space (at least 20%)
	int count = 0;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
			if (board->board[i][j] == 0)
			{
				count ++;
			}
		}
	}
	if (count < 0.2*width*height)
		return false;
	else return true;
}

bool C_Generator::checkForLargeSpace(int width, int height)
{
	for (int rowCount = 0; rowCount < width; rowCount ++)
	{
		for (int colCount = 0; colCount < height; colCount ++)
		{
			if (board->board[rowCount][colCount] != 0) continue;
			
			int i = rowCount;
			int j = colCount;
			if (i+2 < width && j+3 < height)
			{ // possible for a 3(row) x 4(col) space to exist starting here.
				bool empty = true;
				for (int row = i; row < i+2+1; row ++)
				{ // for each row...
					for (int col = j; col < j+3+1; col ++)
					{ // for each col...
						if (board->board[row][col] != 0)
						{
							empty = false;
							break;
						}
					}
				}
				if (empty)
				{						
					return true; // if the whole 3x4 space is empty, return true
				}
			}
		
			if (i+3 < width && j+2 < height)
			{ // possible for a 4(row) x 3(col) space to exist starting here.
				bool empty = true;
				for (int row = i; row < i+3+1; row ++)
				{ // for each row...
					for (int col = j; col < j+2+1; col ++)
					{ // for each col...
						if (board->board[row][col] != 0)
						{
							empty = false;
							break;
						}
					}
				}

				if (empty)
				{
					return true; // if the whole 4x3 space is empty, return true
				}
			}
		}
	}
	
	return false;
}

bool C_Generator::checkForConnectivity(int width, int height)
{
	// check for contiguity.
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
			this->visited[i][j] = false;
		}
	}
	
	int sI, sJ; sI = sJ = 0;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
			if (board->board[i][j] == 0 && patterns[i][j] != 1000)
			{ // starting point.
				sI = i; sJ = j;
			}
		}
	}
	
	dfs(sI, sJ, width, height);
	
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
			if (board->board[i][j] == 0 && this->visited[i][j] == false) return false;
		}
	}
	return true;
}

bool C_Generator::checkForThreeWalls(int width, int height)
{
	for (int i = 0; i < width; i ++)
	{
		for (int j = 0; j < height; j ++)
		{
			if (board->board[i][j] == 0)
			{
				int numWalls = 0;
				if ((i > 0 && board->board[i-1][j] != 0 && board->board[i-1][j]->getId() == EID_BLOCK) || (i == 0)) numWalls++;
				if ((i < width-1 && board->board[i+1][j] != 0 && board->board[i+1][j]->getId() == EID_BLOCK) || (i == width-1)) numWalls++;
				if ((j > 0 && board->board[i][j-1] != 0 && board->board[i][j-1]->getId() == EID_BLOCK) || (j == 0)) numWalls++;
				if ((j < height-1 && board->board[i][j+1] != 0 && board->board[i][j+1]->getId() == EID_BLOCK) || (j == height-1)) numWalls++;
				
				if (numWalls >= 3) return true;
			}
		}
	}
	
	return false;
}

void C_Generator::dfs(int i, int j, int width, int height)
{
	if (this->visited[i][j]) return;
	
	this->visited[i][j] = true;
	if (i > 0 && board->board[i-1][j] == 0 && patterns[i-1][j] != 1000) dfs(i-1, j, width, height);
	if (i < width-1 && board->board[i+1][j] == 0 && patterns[i+1][j] != 1000) dfs(i+1, j, width, height);
	if (j > 0 && board->board[i][j-1] == 0 && patterns[i][j-1] != 1000) dfs(i, j-1, width, height);
	if (j < height-1 && board->board[i][j+1] == 0 && patterns[i][j+1] != 1000) dfs(i, j+1, width, height);
}

void C_Generator::placePlayer()
{
	NumberDistribution widthDist( 0, width );
	NumberDistribution heightDist( 0, height );
	Generator widthGen(generator, widthDist);
	Generator heightGen(generator, heightDist);
	
	int x, y;
	bool alreadyVisitedThisSquare;
	do
	{
		alreadyVisitedThisSquare = false;
		
		x = widthGen();
		y = heightGen();
		
		for (int count = 0; count < previousPlayerPositions.size(); count ++)
		{
			if (x == previousPlayerPositions.at(count).x && y == previousPlayerPositions.at(count).y)
			{
				alreadyVisitedThisSquare = true;
			}
		}
	} while(board->board[x][y] != 0 || alreadyVisitedThisSquare);
	
	int i = x;
	int j = y;
			
	if (board->board[i][j] == 0)
	{ // try player in this location
		board->board[i][j] = new C_Player(board, i, j);
		board->entities.push_back(board->board[i][j]);
		board->players.push_back((C_Player*)board->board[i][j]);
		
		previousPlayerPositions.push_back(C_Coord(i, j));
		
		g_Game.redraw();
		
		return;
	}
}

void C_Generator::removePlayer()
{
	int playerPosX = g_Game.getPlayerPtr()->getPosX();
	int playerPosY = g_Game.getPlayerPtr()->getPosY();
	
	board->entities.erase(std::remove(board->entities.begin(), board->entities.end(), board->board[playerPosX][playerPosY]), board->entities.end());
	board->players.clear();
//	delete board->board[playerPosX][playerPosY];
	board->board[playerPosX][playerPosY] = 0;
}

bool C_Generator::placeGoal()
{	
	NumberDistribution widthDist( 0, width );
	NumberDistribution heightDist( 0, height );
	Generator widthGen(generator, widthDist);
	Generator heightGen(generator, heightDist);
	int x, y;
	do
	{ // choose a random empty spot on the board for a goal.
		x = widthGen();
		y = heightGen();
	} while(board->board[x][y] != 0);
	
	board->board[x][y] = new C_Goal(board, x, y);
	board->goals.push_back((C_Goal*)board->board[x][y]);
	
	g_Game.redraw();
	
	goalPositions.push_back(C_Coord(x, y));
	
	return true;
}

void C_Generator::removeGoal()
{ // erase the most recently placed goal.
	if (goalPositions.size() == 0) return;
	
	C_Coord lastGoal = goalPositions.back();
	goalPositions.pop_back();
	
	// no sol was possible with that goal!
	board->goals.erase(std::remove(board->goals.begin(), board->goals.end(), board->board[lastGoal.x][lastGoal.y]), board->goals.end());
	delete board->board[lastGoal.x][lastGoal.y];
	board->board[lastGoal.x][lastGoal.y] = 0;
}

void C_Generator::getAvailableSpotsForBall(int ball)
{ // want to find a position for the ball that is farthest away from the last goal placed.
	while (!ballSpots[ball].empty())
	{ // clear the vector
		ballSpots[ball].pop();
	}
	
	for (int i = 0; i < g_Game.width; i ++)
	{
		for (int j = 0; j < g_Game.height; j ++)
		{
			if (board->board[i][j] == 0)
			{
				// we want to compare the distance between this ball and the nearest goal.
				
				if (inCorner(i, j)) continue; // if this ball is in a corner, then reject it, since it can never be pushed.
				
				int index = -1;
				int minDist = 1000;
				for (int count = 0; count < goalPositions.size(); count ++)
				{
					int distToGoal = abs(i-goalPositions.at(count).x) + abs(j-goalPositions.at(count).y);
					if (distToGoal < minDist)
					{
						index = count;
						minDist = distToGoal;
					}
				}
				
				ballSpots[ball].push(C_BallCoord(i, j, goalPositions.at(index).x, goalPositions.at(index).y));
			}
		}
	}
}

bool C_Generator::inCorner(int i, int j)
{
	bool sides[4]; // 0 = top, 1 = right, 2 = bottom, 3 = left.
	for (int count = 0; count < 4; count ++) sides[count] = false;
		
	if ((j > 0 && board->board[i][j-1] != 0 && board->board[i][j-1]->getId() == EID_BLOCK) || (j == 0)) sides[0] = true;
	if ((i < g_Game.width-1 && board->board[i+1][j] != 0 && board->board[i+1][j]->getId() == EID_BLOCK) || (i == g_Game.width-1)) sides[1] = true;
	if ((j < g_Game.height-1 && board->board[i][j+1] != 0 && board->board[i][j+1]->getId() == EID_BLOCK) || (j == g_Game.height-1)) sides[2] = true;
	if ((i > 0 && board->board[i-1][j] != 0 && board->board[i-1][j]->getId() == EID_BLOCK) || (i == 0)) sides[3] = true;
	
	for (int count = 0; count < 4; count ++)
	{
		if (sides[count] == true && sides[(count+1)%4] == true)
		{ // e.g. if top(0) and right(1) are true, or e.g. if left(3) and top(0) are true,
			return true; // then the spot is in a "corner"
		}
	}
	
	// don't want the ball to be touching a border wall either, since then it can only ever be
	// pushed in two directions, so the goal usually has to be in that direction, and so on.
	if (i == 1 || i == g_Game.width-2 || j == 1 || j == g_Game.height-2) return true;
	
	// TODO: check other walls...
	
	return false;
}

bool C_Generator::placeBall(int ball, bool continuous)
{
#ifdef WIN32
	  SDL_Event event; // for performance reasons
	  SDL_PollEvent(&event);
#endif
	  
	if (ballSpots[ball].empty())
	{ // no spots left to place this ball! we have to return false, in order to change the position for a prior ball.
		return false;
	}
	C_BallCoord ballSpot = ballSpots[ball].top();
	ballSpots[ball].pop();
	
	if (ballSpot.dist < MINDISTFROMGOAL || board->board[ballSpot.x][ballSpot.y] != 0)
	{ // the furthest ball from the goal state was still too close!
		return placeBall(ball, continuous);
	}
	
	board->board[ballSpot.x][ballSpot.y] = new C_Ball(board, ballSpot.x, ballSpot.y); // place ball in level
	
	g_Game.redraw();
	
	// ball #(ball) was placed, now try to place ball+1.
	
	if (ball+1 == this->numGoals) // (ball starts at 0)
	{ // we just placed the last ball successfully.
		// now see if it is a solvable level.
		
		board->solution.reset();
		board->solution.solve();
		
		if (continuous)
		{
			if (board->solution.possible)
			{
				endBallTime = timeInMilli();
				board->timeToGenerate = (endRoomTime - startRoomTime) + (endBallTime - startBallTime);
				g_Game.saveLevel();
				startBallTime = timeInMilli();
			}
			
			delete board->board[ballSpot.x][ballSpot.y]; // first remove it from its current position.
			board->board[ballSpot.x][ballSpot.y] = 0;
			
			return placeBall(ball, continuous);
		}
		else
		{
			if (board->solution.possible
				&& board->solution.numBoxLines >= this->minBoxLines
				&& board->solution.numBoxLines <= this->maxBoxLines
				&& (!g_Game.isNetworkGame || board->solution.length == this->length)) // not network game OR sol length matches
			{ // a solution was found!
				g_Game.saveLevel();
				return true;
			}
			else
			{ // level not possible
				// try placing this ball in a different spot.
			
				delete board->board[ballSpot.x][ballSpot.y]; // first remove it from its current position.
				board->board[ballSpot.x][ballSpot.y] = 0;
			
				return placeBall(ball, continuous);
			}
		}
	}
	else
	{ // still have a ball to place, so try to place it.
		bool nextBallPlaced = placeBall(ball + 1, continuous);
		
		if (nextBallPlaced)
		{ // placed the ball, so we can return true for this one.
			return true;
		}
		else
		{ // we could not place the next ball (no spots!). so we have to re-place this ball
			delete board->board[ballSpot.x][ballSpot.y]; // first remove it from its current position.
			board->board[ballSpot.x][ballSpot.y] = 0;
			
			return placeBall(ball, continuous); // then try placing this ball again.
		}
	}
}