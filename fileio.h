#ifndef FILEIO_H
#define FILEIO_H

#include <fstream>

class C_Board;

class C_FileIO
{
    public:
		int init();
		bool loadLevel(int numBoxLines, int numGoals);
		bool loadFromArray(char msg[1024], C_Board* board);
		void saveLevel();		
		void genResults();
};

#endif
