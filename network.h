#ifndef NETWORK_H
#define NETWORK_H

#include "SDLheaders.h"
#include "coord.h"
#include <vector>
#include <string>

class C_NetGame
{
public:
	C_NetGame(std::string ip_, int diff_) : ipString(ip_), difficulty(diff_) { }
	std::string ipString;
	int difficulty;
};

class C_Network
{
    public:
        int init();
        void cleanUp();		

		C_NetGame* getNextAvailableGame();
		bool hostGame(int difficulty);
		bool joinGame(C_NetGame* game);

		void sendLevelToPeer();
		void receiveLevelFromPeer();
		void broadcastGameOver();

        void move(); //Asks permission to move
        void move(int, int); //Tells the server where it's moving

		bool receivedlevelFromPeer;
		
		static void staticRcvLevel(C_Network* instance) { instance->receiveLevelFromPeer(); }
		static void staticBcastMove(C_Network* instance) { instance->broadcastMoves(); }
		static void staticRcvMoves(C_Network* instance) { instance->receiveMoves(); }
    private:
		void getThisIP();
        bool startServer();
        void waitForClients(int);
		void removeGameFromLobby(); // remove game listing from lobby.

        bool startClient(const char*);
		
		void broadcastMoves();
		void receiveMoves();
		
		enum
		{
			MSG_LEVEL,
			MSG_MOVE,
			MSG_GAMEOVER
		};
		
		IPaddress thisIP;
		
        bool isServer;
        short port;
        TCPsocket sd;  //Socket descriptor
        std::vector<TCPsocket> csd; //Client socket descriptor
        IPaddress ip;
        IPaddress *remoteIP;		
		
		std::vector<C_NetGame> games;
};

#endif
