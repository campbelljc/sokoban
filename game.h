#ifndef GAME_H
#define GAME_H

#include <vector>
#include <sstream>
#include <thread>
#include <queue>

#include "board.h"
#include "graphics.h"
#include "generator.h"
#include "fileio.h"
#include "network.h"
#include "ui.h"

template <class type>
std::string toString(type toBeConverted) {
	std::stringstream out;
	out << toBeConverted;
	return out.str();
}

class C_Move
{
public:
	C_Move(C_Coord pos_, int x__, int y__) : pos(pos_), x_(x__), y_(y__) { }
	C_Coord pos;
	int x_;
	int y_;
};

class C_Game
{
    public:
        C_Game();

        int init();
        void start() { ui.start(); }
		
		void setState(int state_);
		int getState() { return state; }

		void redraw(C_Entity* = 0);
		void toggleFullscreen() { gfx.toggleFullscreen(); }
		void animateTile(C_Entity* ent, const char* filename, int steps, int stepDelay) { gfx.animateTileOverSpace(ent, filename, steps, stepDelay, C_Rect(0, 0, C_Board::TWIDTH, C_Board::THEIGHT), C_Rect(0, 0, C_Board::TWIDTH, C_Board::THEIGHT)); }
		
		void delay(int ms);
		void refreshWindowCaption();

        C_Player* getPlayerPtr() { return board.players[0]; }
        C_Entity* getEntityPtr(int a, int b) { return board.getEntityPtr(a,b); }
		void setEntityPtr(C_Entity* entity, int x, int y) { board.setEntityPtr(entity, x, y); }
		int getEntityType(int x, int y) { return board.getEntityType(x, y); }
        C_Board* getBoardPtr() { return &board; }
		C_Board* getOpponentBoardPtr() { return &opponentBoard; }

		void generateLevel(bool continuous, int minBoxLines, int maxBoxLines, int numGoals) { generator.generateLevel(continuous, minBoxLines, maxBoxLines, numGoals); }
		void continuallyGenerateLevels();
						
		bool loadLevel() { return fio.loadLevel(chosenBoxLines, chosenNumGoals); }
		void saveLevel() { fio.saveLevel(); }
		
		void hostGame();
		void joinGame();
		bool loadOpponentBoardFromMessage(char msg[1024]) { return fio.loadFromArray(msg, getOpponentBoardPtr()); }
		
		int width;
		int height;
		void addSize(int s);
		
		bool inRect(C_Coord mousePos, std::string path, int midXPos, int topYPos);
		bool inRect(C_Coord pos, int x1, int y1, int x2, int y2) { return (pos.x >= x1 && pos.x <= x2 && pos.y >= y1 && pos.y <= y2); }
		
		bool isNetworkGame;
		
		int totalMoves;
		int totalOpponentMoves;
		
		bool oppGameFinished;
		bool done;
		
		int chosenBoxLines;
		int chosenNumGoals;
		
		bool diffSelected;
		
		void broadcastMove(C_Coord pos, int x_, int y_);
		
		std::queue<C_Move> movesToBroadcast;
		std::thread rcvMoveThread;
		std::thread bcastMoveThread;		
		void checkForCompletion();
	protected:
		C_Board board;
		C_Board opponentBoard;
    private:
		void setupNetworkGame(int difficulty);
		
		int state;

		C_UI ui;
        C_Graphics gfx;
		C_FileIO fio;
		C_Network network;
		C_Generator generator;
};

#endif
