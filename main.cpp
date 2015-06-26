#include <fstream>
#include <iostream>
#include "g_Game.h"

int main ( int argc, char** argv )
{
	std::cout << "g_Game.init() starting...\n";
    if (!g_Game.init())
    {
		std::cout<<"Error initializing game."<<std::flush;
		atexit(SDL_Quit);
        return 0;
    }
	atexit(SDL_Quit);
	std::cout << "...done.\n";

	std::cout << "g_Game.start() starting...\n";
    g_Game.start();
	std::cout << "...done.\n";

    return 0;
}
