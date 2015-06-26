#include "board.h"
#include "fileio.h"
#include "entity.h"
#include "g_Game.h"

#include <string>
#include <fstream>

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

int C_FileIO::init()
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        return false;
    }
    CFRelease(resourcesURL);
    chdir(path); // change working directory to app resources folder.
			
	return true;
}

bool C_FileIO::loadLevel(int numBoxLines, int numGoals)
{
	C_Board* board = g_Game.getBoardPtr();

	std::string dirStr = "./levels/" + toString(g_Game.width) + "x" + toString(g_Game.height) + "-" + toString(numGoals);
	
	std::string path;
	boost::filesystem::directory_iterator end;
	for ( boost::filesystem::directory_iterator dir(dirStr); dir != end; ++dir )
	{
		path = dir->path().string();
		if (path.find(".txt") != std::string::npos)
		{
			std::string boxlinestr = path;
			boxlinestr.erase(0, boxlinestr.find("-")+1);
			boxlinestr.erase(0, boxlinestr.find("/")+1);
			boxlinestr.erase(boxlinestr.find("-"));
			int boxline = atoi(boxlinestr.c_str());
			if (boxline == numBoxLines) break;
		}
	}
	
	std::ifstream file(path.c_str(), std::ifstream::in|std::ifstream::binary);
	if (!file.is_open())
	{ // the level could not be loaded
		std::cout << "Error loading level at path " << path << ".\n";
		return false;
	}
	
    board->clear();
	
	std::string gentime;
	getline(file, gentime);
	board->timeToGenerate = atoi(gentime.c_str());

    C_Coord pos; //stores coordinates on array

#ifdef WIN32
	char mem[(g_Game.width+1)*(g_Game.height)];	
	size_t size = sizeof(mem);
#else
	char mem[(g_Game.width+1)*g_Game.height];
	size_t size = sizeof(char[(g_Game.width+1)*g_Game.height]);
#endif
	
	file.read(mem, size);
	
	int num = 0;
	
	for (int i = 0; i < size; i++)
	{
		pos.x = i%(g_Game.width+1);
		pos.y = i/(g_Game.width+1);
        switch (mem[i])
        {
            case '\n': { num --; break; } //New line. Nothing happens
            case ' ': break; // empty square
            case '/':
            case '@': //player
            {
                board->board[pos.x][pos.y] = new C_Player(board, pos.x, pos.y);
				board->entities.push_back(board->board[pos.x][pos.y]);
				board->players.push_back((C_Player*)board->board[pos.x][pos.y]);
                break;
            }
            case '0': //ball
            {
                board->board[pos.x][pos.y] = new C_Ball(board, pos.x, pos.y);
				board->entities.push_back(board->board[pos.x][pos.y]);
				board->balls.push_back((C_Ball*)board->board[pos.x][pos.y]);
                break;
            }
			case 'x': // static block
			{
				board->board[pos.x][pos.y] = new C_Block(board, pos.x, pos.y);
				board->entities.push_back(board->board[pos.x][pos.y]);
				board->blocks.push_back((C_Block*)board->board[pos.x][pos.y]);
                break;
			}
			case 'G':
			{
				board->ground[pos.x][pos.y] = new C_Goal(board, pos.x, pos.y);
				board->entities.push_back(board->ground[pos.x][pos.y]);
				board->goals.push_back((C_Goal*)board->ground[pos.x][pos.y]);
                break;
			}
			default:
			{
				std::cout << "Unknown character loaded: " << mem[i] << "\n";
			    break;
			}
        }
		
		num ++;
	}

	if (board->players.size() == 0)
	{
		std::cout << "No player in " << path << ".\n";
		board->clear();
		return false;
	}
	
	//board->solution.solve();

    file.close();
	return true;
}

bool C_FileIO::loadFromArray(char msg[1024], C_Board* board)
{
    C_Coord pos; //stores coordinates on array
	int num = 0;
	
//	std::cout<<"----rcvd\n";
//	for (int i = 0; i < 200; i ++) std::cout<<msg[i];
//	std::cout<<"----\n";

	for (int i = 0; i < (g_Game.width*(g_Game.height+1)); i++)
	{
		pos.x = i%(g_Game.width+1);
		pos.y = i/(g_Game.height+1);
        switch (msg[i])
        {
            case '\n': { num --; break; } //New line. Nothing happens
            case ' ': break; // empty square
            case '/':
            case '@': //player
            {
                board->board[pos.x][pos.y] = new C_Player(board, pos.x, pos.y);
				board->entities.push_back(board->board[pos.x][pos.y]);
				board->players.push_back((C_Player*)board->board[pos.x][pos.y]);
                break;
            }
            case '0': //ball
            {
                board->board[pos.x][pos.y] = new C_Ball(board, pos.x, pos.y);
				board->entities.push_back(board->board[pos.x][pos.y]);
				board->balls.push_back((C_Ball*)board->board[pos.x][pos.y]);
                break;
            }
			case 'x': // static block
			{
				board->board[pos.x][pos.y] = new C_Block(board, pos.x, pos.y);
				board->entities.push_back(board->board[pos.x][pos.y]);
				board->blocks.push_back((C_Block*)board->board[pos.x][pos.y]);
                break;
			}
			case 'G':
			{
				board->ground[pos.x][pos.y] = new C_Goal(board, pos.x, pos.y);
				board->entities.push_back(board->ground[pos.x][pos.y]);
				board->goals.push_back((C_Goal*)board->ground[pos.x][pos.y]);
                break;
			}
			default:
			{
				std::cout << "Unknown character loaded: " << msg[i] << "\n";
			    break;
			}
        }
		
		num ++;
	}

	if (board->players.size() == 0)
	{
		std::cout << "No player in received level.\n";
		board->clear();
		return false;
	}
	
	return true;
}

void C_FileIO::saveLevel()
{
	C_Board* board = g_Game.getBoardPtr();

	std::string boardStr;
	for (int i = 0; i < g_Game.width-1; i ++)
	{
		for (int j = 0; j < g_Game.height-1; j ++)
		{
			if (board->getEntityPtr(i, j) == 0) boardStr += " ";
			else boardStr += board->getEntityPtr(i, j)->getChar();
		}
	}
	
#ifdef WIN32
	std::string path = "levels\\"
		+ toString(g_Game.width) + "x" + toString(g_Game.height) + "-" + toString(board->goals.size())
			+ "\\" + toString(board->solution.numBoxLines)
				+ "-" + toString(board->solution.length)
					+ "-" + boardStr
					+ "-" + board->solution.solString + ".txt";
#else
	std::string path = "levels/"
		+ toString(g_Game.width) + "x" + toString(g_Game.height) + "-" + toString(board->goals.size())
			+ "/" + toString(board->solution.numBoxLines)
				+ "-" + toString(board->solution.length)
					+ "-" + boardStr
					+ "-" + board->solution.solString + ".txt";	
#endif

	path = "../../../" + path;
	std::cout<<"Saving level at path " << path << ". \n";	
	
	std::ofstream file(path.c_str());
	if (!file.is_open())
	{ // the level could not be loaded
		std::cout << "Error saving level.\n";
		return;
	}
	
	file << board->timeToGenerate<<"\n";
	
	for (int i = 0; i < g_Game.width; i ++)
	{
		for (int j = 0; j < g_Game.height; j ++)
		{
			if (board->getEntityPtr(i, j) == 0) file << " ";
			else file << board->getEntityPtr(i, j)->getChar();
		}
		file << "\n";
	}
		
	file.close();
}

void C_FileIO::genResults()
{
	int lengthPerBoxLine[20];
	int numFilesPerBoxLine[20];
	for (int count = 0; count < 20; count ++)
	{
		lengthPerBoxLine[count] = numFilesPerBoxLine[count] = 0;
	}
	
	for (int size = 8; size <= 11; size += 3)
	{
		for (int goals = 1; goals < 6; goals ++)
		{
#ifdef WIN32
			std::string dirStr = ".\\levels\\" + toString(size) + "x" + toString(size) + "-" + toString(goals);
#else
			std::string dirStr = "./levels/" + toString(size) + "x" + toString(size) + "-" + toString(goals);
#endif
			
			std::cout<<"dirstr:"<<dirStr<<"\n";
	
			std::string path;
			boost::filesystem::directory_iterator end;
			
			int numFiles, totalBoxLines, totalTime;
			numFiles = totalBoxLines = totalTime = 0;
			for ( boost::filesystem::directory_iterator dir(dirStr); dir != end; ++dir )
			{ // for all files in folder
				path = dir->path().string();
				if (path.find(".txt") != std::string::npos)
				{
					numFiles ++;
					
					std::string boxlinestr = path;
					boxlinestr.erase(0, boxlinestr.find("-")+1);
					boxlinestr.erase(0, boxlinestr.find("/")+1);
					boxlinestr.erase(boxlinestr.find("-"));
					int boxline = atoi(boxlinestr.c_str());
					totalBoxLines += boxline;
					
					std::string lengthstr = path;
					lengthstr.erase(0, lengthstr.find("-")+1);
					lengthstr.erase(0, lengthstr.find("-")+1);
					lengthstr.erase(lengthstr.find("-"));
					int length = atoi(lengthstr.c_str());
					
					numFilesPerBoxLine[boxline] ++;
					lengthPerBoxLine[boxline] += length;
					
					std::ifstream file(path.c_str(), std::ifstream::in|std::ifstream::binary);
					if (!file.is_open())
					{ // the level could not be loaded
						std::cout << "Error loading level at path " << path << ".\n";
						continue;
					}
						
					std::string gentime;
					getline(file, gentime);
					totalTime += atoi(gentime.c_str());
					
					file.close();
				}
			}
			
			std::cout<<"Num files: "<<numFiles <<", totalboxlines:"<<totalBoxLines <<", totaltime: " << totalTime<<"\n";
			
			if (numFiles == 0) continue;
			
			float avgBL = (float)totalBoxLines/(float)numFiles;
			float avgTime = (float)totalTime/(float)numFiles;
			std::cout<<"Avg bl: " << avgBL <<", avg time: " << avgTime<<"\n";
		}	
	}
	
	for (int count = 0; count < 20; count ++)
	{
		if (numFilesPerBoxLine[count] == 0) continue;
		float avgLength = (float)lengthPerBoxLine[count]/(float)numFilesPerBoxLine[count];
		
		std::cout<<"Avg length for " << count << " boxlines is " << avgLength <<".\n";
	}
}