#ifndef GENERATOR_H
#define GENERATOR_H

#include <vector>
#include <queue>
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"

#include "board.h"
#include "coord.h"

class C_BallCoord
{
public:
    C_BallCoord(int x_, int y_, int gX_, int gY_) : x(x_), y(y_), gX(gX_), gY(gY_), dist(-1) { }

    int x;
    int y;
	int gX;
	int gY;
	
	int dist;
};

struct CompareBallCoord
{
	bool operator()( C_BallCoord &b1,  C_BallCoord &b2) const
	{ // return true if b1 less than b2
		int b1DistToGoal = abs(b1.x - b1.gX) + abs(b1.y - b1.gY);
		int b2DistToGoal = abs(b2.x - b2.gX) + abs(b2.y - b2.gY);
		
		b1.dist = b1DistToGoal;
		b2.dist = b2DistToGoal;
		
		return (b1DistToGoal < b2DistToGoal); // want larger dist to have higher priority
	}
};

typedef boost::uniform_int<> NumberDistribution; // choose a distribution
typedef boost::mt19937 RandomNumberGenerator;    // pick the random number generator method,
typedef boost::variate_generator< RandomNumberGenerator&, NumberDistribution > Generator;  // link the generator to the distribution

class C_Generator
{
public:
	C_Generator();
	
	static const int MINDISTFROMGOAL = 2;
	static const int MINSOLUTIONLENGTH = 30;
	static const int MINSOLUTIONBOXLINES = 8;
	static const int NUMGOALS = 1;
	
	void generateLevel(bool continuous, int minBoxLines = MINSOLUTIONBOXLINES, int maxBoxLines = 10000, int numGoals = NUMGOALS, int length = 0);
	
	int getWidth() { return width; }
	int getHeight() { return height; }
	void addSize(int s) { width += s; height += s; }
	
private:
	bool tryToGenerate();
	bool checkForEnoughSpace(int width, int height);
	bool checkForLargeSpace(int width, int height);
	bool checkForConnectivity(int width, int height);
	bool checkForThreeWalls(int width, int height);
	void dfs(int i, int j, int width, int height);
	
	void placePlayer();
	void removePlayer();
	
	bool placeGoal();
	void removeGoal();
	
	void getAvailableSpotsForBall(int ball);
	bool inCorner(int i, int j);
	
	bool placeBall(int ball, bool continuous);
	
	C_Board* board;
	
	bool visited[32][24]; // 32,24 = max width+height.
	
	int** patterns; //[DEFAULTWIDTH][DEFAULTHEIGHT];
	
	int numGoals;
	int minBoxLines;
	int maxBoxLines;
	int length;
	
	int width, height;
	
	std::vector<C_Coord> goalPositions;
	std::vector<C_Coord> previousPlayerPositions;
	
	std::priority_queue<C_BallCoord, std::vector<C_BallCoord>, CompareBallCoord> ballSpots[50];
	
	static RandomNumberGenerator generator;
};

#endif